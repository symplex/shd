//
// Copyright 2010-2012,2014-2015 Ettus Research LLC
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//

#include "smini2_regs.hpp"
#include "smini2_impl.hpp"
#include "fw_common.h"
#include "smini2_iface.hpp"
#include <shd/exception.hpp>
#include <shd/utils/msg.hpp>
#include <shd/utils/paths.hpp>
#include <shd/utils/tasks.hpp>
#include <shd/utils/paths.hpp>
#include <shd/utils/safe_call.hpp>
#include <shd/types/dict.hpp>
#include <boost/thread.hpp>
#include <boost/foreach.hpp>
#include <boost/asio.hpp> //used for htonl and ntohl
#include <boost/assign/list_of.hpp>
#include <boost/format.hpp>
#include <boost/bind.hpp>
#include <boost/tokenizer.hpp>
#include <boost/functional/hash.hpp>
#include <boost/filesystem.hpp>
#include <algorithm>
#include <iostream>
#include <shd/utils/platform.hpp>

using namespace shd;
using namespace shd::smini;
using namespace shd::transport;
namespace fs = boost::filesystem;

static const double CTRL_RECV_TIMEOUT = 1.0;
static const size_t CTRL_RECV_RETRIES = 3;

//custom timeout error for retry logic to catch/retry
struct timeout_error : shd::runtime_error
{
    timeout_error(const std::string &what):
        shd::runtime_error(what)
    {
        //NOP
    }
};

static const uint32_t MIN_PROTO_COMPAT_SPI = 7;
static const uint32_t MIN_PROTO_COMPAT_I2C = 7;
// The register compat number must reflect the protocol compatibility
// and the compatibility of the register mapping (more likely to change).
static const uint32_t MIN_PROTO_COMPAT_REG = 10;
//static const uint32_t MIN_PROTO_COMPAT_UART = 7;

class smini2_iface_impl : public smini2_iface{
public:
/***********************************************************************
 * Structors
 **********************************************************************/
    smini2_iface_impl(udp_simple::sptr ctrl_transport):
        _ctrl_transport(ctrl_transport),
        _ctrl_seq_num(0),
        _protocol_compat(0) //initialized below...
    {
        //Obtain the firmware's compat number.
        //Save the response compat number for communication.
        //TODO can choose to reject certain older compat numbers
        smini2_ctrl_data_t ctrl_data = smini2_ctrl_data_t();
        ctrl_data.id = htonl(SMINI2_CTRL_ID_WAZZUP_BRO);
        ctrl_data = ctrl_send_and_recv(ctrl_data, 0, ~0);
        if (ntohl(ctrl_data.id) != SMINI2_CTRL_ID_WAZZUP_DUDE)
            throw shd::runtime_error("firmware not responding");
        _protocol_compat = ntohl(ctrl_data.proto_ver);

        mb_eeprom = mboard_eeprom_t(*this, SMINI2_EEPROM_MAP_KEY);
    }

    ~smini2_iface_impl(void){SHD_SAFE_CALL(
        this->lock_device(false);
    )}

/***********************************************************************
 * Device locking
 **********************************************************************/

    void lock_device(bool lock){
        if (lock){
            this->pokefw(U2_FW_REG_LOCK_GPID, get_process_hash());
            _lock_task = task::make(boost::bind(&smini2_iface_impl::lock_task, this));
        }
        else{
            _lock_task.reset(); //shutdown the task
            this->pokefw(U2_FW_REG_LOCK_TIME, 0); //unlock
        }
    }

    bool is_device_locked(void){
        //never assume lock with fpga image mismatch
        if ((this->peek32(U2_REG_COMPAT_NUM_RB) >> 16) != SMINI2_FPGA_COMPAT_NUM) return false;

        uint32_t lock_time = this->peekfw(U2_FW_REG_LOCK_TIME);
        uint32_t lock_gpid = this->peekfw(U2_FW_REG_LOCK_GPID);

        //may not be the right tick rate, but this is ok for locking purposes
        const uint32_t lock_timeout_time = uint32_t(3*100e6);

        //if the difference is larger, assume not locked anymore
        if ((lock_time & 1) == 0) return false; //bit0 says unlocked
        const uint32_t time_diff = this->get_curr_time() - lock_time;
        if (time_diff >= lock_timeout_time) return false;

        //otherwise only lock if the device hash is different that ours
        return lock_gpid != get_process_hash();
    }

    void lock_task(void){
        //re-lock in task
        this->pokefw(U2_FW_REG_LOCK_TIME, this->get_curr_time());
        //sleep for a bit
        boost::this_thread::sleep(boost::posix_time::milliseconds(1500));
    }

    uint32_t get_curr_time(void){
        return this->peek32(U2_REG_TIME64_LO_RB_IMM) | 1; //bit 1 says locked
    }

/***********************************************************************
 * Peek and Poke
 **********************************************************************/
    void poke32(const wb_addr_type addr, const uint32_t data){
        this->get_reg<uint32_t, SMINI2_REG_ACTION_FPGA_POKE32>(addr, data);
    }

    uint32_t peek32(const wb_addr_type addr){
        return this->get_reg<uint32_t, SMINI2_REG_ACTION_FPGA_PEEK32>(addr);
    }

    void poke16(const wb_addr_type addr, const uint16_t data){
        this->get_reg<uint16_t, SMINI2_REG_ACTION_FPGA_POKE16>(addr, data);
    }

    uint16_t peek16(const wb_addr_type addr){
        return this->get_reg<uint16_t, SMINI2_REG_ACTION_FPGA_PEEK16>(addr);
    }

    void pokefw(wb_addr_type addr, uint32_t data)
    {
        this->get_reg<uint32_t, SMINI2_REG_ACTION_FW_POKE32>(addr, data);
    }

    uint32_t peekfw(wb_addr_type addr)
    {
        return this->get_reg<uint32_t, SMINI2_REG_ACTION_FW_PEEK32>(addr);
    }

    template <class T, smini2_reg_action_t action>
    T get_reg(wb_addr_type addr, T data = 0){
        //setup the out data
        smini2_ctrl_data_t out_data = smini2_ctrl_data_t();
        out_data.id = htonl(SMINI2_CTRL_ID_GET_THIS_REGISTER_FOR_ME_BRO);
        out_data.data.reg_args.addr = htonl(addr);
        out_data.data.reg_args.data = htonl(uint32_t(data));
        out_data.data.reg_args.action = action;

        //send and recv
        smini2_ctrl_data_t in_data = this->ctrl_send_and_recv(out_data, MIN_PROTO_COMPAT_REG);
        SHD_ASSERT_THROW(ntohl(in_data.id) == SMINI2_CTRL_ID_OMG_GOT_REGISTER_SO_BAD_DUDE);
        return T(ntohl(in_data.data.reg_args.data));
    }

/***********************************************************************
 * SPI
 **********************************************************************/
    uint32_t transact_spi(
        int which_slave,
        const spi_config_t &config,
        uint32_t data,
        size_t num_bits,
        bool readback
    ){
        static const shd::dict<spi_config_t::edge_t, int> spi_edge_to_otw = boost::assign::map_list_of
            (spi_config_t::EDGE_RISE, SMINI2_CLK_EDGE_RISE)
            (spi_config_t::EDGE_FALL, SMINI2_CLK_EDGE_FALL)
        ;

        //setup the out data
        smini2_ctrl_data_t out_data = smini2_ctrl_data_t();
        out_data.id = htonl(SMINI2_CTRL_ID_TRANSACT_ME_SOME_SPI_BRO);
        out_data.data.spi_args.dev = htonl(which_slave);
        out_data.data.spi_args.miso_edge = spi_edge_to_otw[config.miso_edge];
        out_data.data.spi_args.mosi_edge = spi_edge_to_otw[config.mosi_edge];
        out_data.data.spi_args.readback = (readback)? 1 : 0;
        out_data.data.spi_args.num_bits = num_bits;
        out_data.data.spi_args.data = htonl(data);

        //send and recv
        smini2_ctrl_data_t in_data = this->ctrl_send_and_recv(out_data, MIN_PROTO_COMPAT_SPI);
        SHD_ASSERT_THROW(ntohl(in_data.id) == SMINI2_CTRL_ID_OMG_TRANSACTED_SPI_DUDE);

        return ntohl(in_data.data.spi_args.data);
    }

/***********************************************************************
 * I2C
 **********************************************************************/
    void write_i2c(uint16_t addr, const byte_vector_t &buf){
        //setup the out data
        smini2_ctrl_data_t out_data = smini2_ctrl_data_t();
        out_data.id = htonl(SMINI2_CTRL_ID_WRITE_THESE_I2C_VALUES_BRO);
        out_data.data.i2c_args.addr = uint8_t(addr);
        out_data.data.i2c_args.bytes = buf.size();

        //limitation of i2c transaction size
        SHD_ASSERT_THROW(buf.size() <= sizeof(out_data.data.i2c_args.data));

        //copy in the data
        std::copy(buf.begin(), buf.end(), out_data.data.i2c_args.data);

        //send and recv
        smini2_ctrl_data_t in_data = this->ctrl_send_and_recv(out_data, MIN_PROTO_COMPAT_I2C);
        SHD_ASSERT_THROW(ntohl(in_data.id) == SMINI2_CTRL_ID_COOL_IM_DONE_I2C_WRITE_DUDE);
    }

    byte_vector_t read_i2c(uint16_t addr, size_t num_bytes){
        //setup the out data
        smini2_ctrl_data_t out_data = smini2_ctrl_data_t();
        out_data.id = htonl(SMINI2_CTRL_ID_DO_AN_I2C_READ_FOR_ME_BRO);
        out_data.data.i2c_args.addr = uint8_t(addr);
        out_data.data.i2c_args.bytes = num_bytes;

        //limitation of i2c transaction size
        SHD_ASSERT_THROW(num_bytes <= sizeof(out_data.data.i2c_args.data));

        //send and recv
        smini2_ctrl_data_t in_data = this->ctrl_send_and_recv(out_data, MIN_PROTO_COMPAT_I2C);
        SHD_ASSERT_THROW(ntohl(in_data.id) == SMINI2_CTRL_ID_HERES_THE_I2C_DATA_DUDE);
        SHD_ASSERT_THROW(in_data.data.i2c_args.bytes == num_bytes);

        //copy out the data
        byte_vector_t result(num_bytes);
        std::copy(in_data.data.i2c_args.data, in_data.data.i2c_args.data + num_bytes, result.begin());
        return result;
    }

/***********************************************************************
 * Send/Recv over control
 **********************************************************************/
    smini2_ctrl_data_t ctrl_send_and_recv(
        const smini2_ctrl_data_t &out_data,
        uint32_t lo = SMINI2_FW_COMPAT_NUM,
        uint32_t hi = SMINI2_FW_COMPAT_NUM
    ){
        boost::mutex::scoped_lock lock(_ctrl_mutex);

        for (size_t i = 0; i < CTRL_RECV_RETRIES; i++){
            try{
                return ctrl_send_and_recv_internal(out_data, lo, hi, CTRL_RECV_TIMEOUT/CTRL_RECV_RETRIES);
            }
            catch(const timeout_error &e){
                SHD_MSG(error)
                    << "Control packet attempt " << i
                    << ", sequence number " << _ctrl_seq_num
                    << ":\n" << e.what() << std::endl;
            }
        }
        throw shd::runtime_error("link dead: timeout waiting for control packet ACK");
    }

    smini2_ctrl_data_t ctrl_send_and_recv_internal(
        const smini2_ctrl_data_t &out_data,
        uint32_t lo, uint32_t hi,
        const double timeout
    ){
        //fill in the seq number and send
        smini2_ctrl_data_t out_copy = out_data;
        out_copy.proto_ver = htonl(_protocol_compat);
        out_copy.seq = htonl(++_ctrl_seq_num);
        _ctrl_transport->send(boost::asio::buffer(&out_copy, sizeof(smini2_ctrl_data_t)));

        //loop until we get the packet or timeout
        uint8_t smini2_ctrl_data_in_mem[udp_simple::mtu]; //allocate max bytes for recv
        const smini2_ctrl_data_t *ctrl_data_in = reinterpret_cast<const smini2_ctrl_data_t *>(smini2_ctrl_data_in_mem);
        while(true){
            size_t len = _ctrl_transport->recv(boost::asio::buffer(smini2_ctrl_data_in_mem), timeout);
            uint32_t compat = ntohl(ctrl_data_in->proto_ver);
            if(len >= sizeof(uint32_t) and (hi < compat or lo > compat)){
                throw shd::runtime_error(str(boost::format(
                    "\nPlease update the firmware and FPGA images for your device.\n"
                    "See the application notes for SMINI2/N-Series for instructions.\n"
                    "Expected protocol compatibility number %s, but got %d:\n"
                    "The firmware build is not compatible with the host code build.\n"
                    "%s\n"
                ) % ((lo == hi)? (boost::format("%d") % hi) : (boost::format("[%d to %d]") % lo % hi))
                  % compat % this->images_warn_help_message()));
            }
            if (len >= sizeof(smini2_ctrl_data_t) and ntohl(ctrl_data_in->seq) == _ctrl_seq_num){
                return *ctrl_data_in;
            }
            if (len == 0) break; //timeout
            //didnt get seq or bad packet, continue looking...
        }
        throw timeout_error("no control response, possible packet loss");
    }

    rev_type get_rev(void){
        std::string hw = mb_eeprom["hardware"];
        if (hw.empty()) return SMINI_NXXX;
        switch (boost::lexical_cast<uint16_t>(hw)){
        case 0x0300:
        case 0x0301: return SMINI2_REV3;
        case 0x0400: return SMINI2_REV4;
        case 0x0A00: return SMINI_N200;
        case 0x0A01: return SMINI_N210;
        case 0x0A10: return SMINI_N200_R4;
        case 0x0A11: return SMINI_N210_R4;
        }
        return SMINI_NXXX; //unknown type
    }

    const std::string get_cname(void){
        switch(this->get_rev()){
        case SMINI2_REV3: return "SMINI2 r3";
        case SMINI2_REV4: return "SMINI2 r4";
        case SMINI_N200: return "N200";
        case SMINI_N210: return "N210";
        case SMINI_N200_R4: return "N200r4";
        case SMINI_N210_R4: return "N210r4";
        case SMINI_NXXX: return "N???";
        }
        SHD_THROW_INVALID_CODE_PATH();
    }

    const std::string get_fw_version_string(void){
        uint32_t minor = this->get_reg<uint32_t, SMINI2_REG_ACTION_FW_PEEK32>(U2_FW_REG_VER_MINOR);
        return str(boost::format("%u.%u") % _protocol_compat % minor);
    }

    std::string images_warn_help_message(void){
        //determine the images names
        std::string fw_image, fpga_image;
        switch(this->get_rev()){
        case SMINI2_REV3:   fpga_image = "smini2_fpga.bin";        fw_image = "smini2_fw.bin";     break;
        case SMINI2_REV4:   fpga_image = "smini2_fpga.bin";        fw_image = "smini2_fw.bin";     break;
        case SMINI_N200:    fpga_image = "smini_n200_r2_fpga.bin"; fw_image = "smini_n200_fw.bin"; break;
        case SMINI_N210:    fpga_image = "smini_n210_r2_fpga.bin"; fw_image = "smini_n210_fw.bin"; break;
        case SMINI_N200_R4: fpga_image = "smini_n200_r4_fpga.bin"; fw_image = "smini_n200_fw.bin"; break;
        case SMINI_N210_R4: fpga_image = "smini_n210_r4_fpga.bin"; fw_image = "smini_n210_fw.bin"; break;
        default: break;
        }
        if (fw_image.empty() or fpga_image.empty()) return "";

        //does your platform use sudo?
        std::string sudo;
        #if defined(SHD_PLATFORM_LINUX) || defined(SHD_PLATFORM_MACOS)
            sudo = "sudo ";
        #endif


        //look up the real FS path to the images
        std::string fw_image_path, fpga_image_path;
        try{
            fw_image_path = shd::find_image_path(fw_image);
            fpga_image_path = shd::find_image_path(fpga_image);
        }
        catch(const std::exception &){
            return str(boost::format("Could not find %s and %s in your images path!\n%s") % fw_image % fpga_image % print_utility_error("shd_images_downloader.py"));
        }

        //escape char for multi-line cmd + newline + indent?
        #ifdef SHD_PLATFORM_WIN32
            const std::string ml = "^\n    ";
        #else
            const std::string ml = "\\\n    ";
        #endif

        //create the burner commands
        if (this->get_rev() == SMINI2_REV3 or this->get_rev() == SMINI2_REV4){
            const std::string card_burner = shd::find_utility("smini2_card_burner_gui.py");
            const std::string card_burner_cmd = str(boost::format(" %s\"%s\" %s--fpga=\"%s\" %s--fw=\"%s\"") % sudo % card_burner % ml % fpga_image_path % ml % fw_image_path);
            return str(boost::format("%s\n%s") % print_utility_error("shd_images_downloader.py") % card_burner_cmd);
        }
        else{
            const std::string addr = _ctrl_transport->get_recv_addr();
            const std::string image_loader_path = (fs::path(shd::get_pkg_path()) / "bin" / "shd_image_loader").string();
            const std::string image_loader_cmd = str(boost::format(" \"%s\" %s--args=\"type=smini2,addr=%s\"") % image_loader_path % ml % addr);
            return str(boost::format("%s\n%s") % print_utility_error("shd_images_downloader.py") % image_loader_cmd);
        }
    }

    void set_time(const time_spec_t&)
    {
        throw shd::not_implemented_error("Timed commands not supported");
    }

    time_spec_t get_time(void)
    {
        return (0.0);
    }

private:
    //this lovely lady makes it all possible
    udp_simple::sptr _ctrl_transport;

    //used in send/recv
    boost::mutex _ctrl_mutex;
    uint32_t _ctrl_seq_num;
    uint32_t _protocol_compat;

    //lock thread stuff
    task::sptr _lock_task;
};

/***********************************************************************
 * Public make function for smini2 interface
 **********************************************************************/
smini2_iface::sptr smini2_iface::make(udp_simple::sptr ctrl_transport){
    return smini2_iface::sptr(new smini2_iface_impl(ctrl_transport));
}

