//
// Copyright 2010-2012 Ettus Research LLC
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

#include "fx2_ctrl.hpp"
#include <shd/utils/msg.hpp>
#include <shd/exception.hpp>
#include <shd/transport/usb_control.hpp>
#include <boost/functional/hash.hpp>
#include <boost/thread/thread.hpp>
#include <stdint.h>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cstring>

using namespace shd;
using namespace shd::smini;

#define FX2_FIRMWARE_LOAD 0xa0

static const bool load_img_msg = true;

typedef uint32_t hash_type;

/***********************************************************************
 * Helper Functions
 **********************************************************************/
/*!
 * Create a file hash
 * The hash will be used to identify the loaded firmware and fpga image
 * \param filename file used to generate hash value
 * \return hash value in a size_t type
 */
static hash_type generate_hash(const char *filename)
{
    std::ifstream file(filename);
    if (not file){
        throw shd::io_error(std::string("cannot open input file ") + filename);
    }

    size_t hash = 0;

    char ch;
    while (file.get(ch)) {
        boost::hash_combine(hash, ch);
    }

    if (not file.eof()){
        throw shd::io_error(std::string("file error ") + filename);
    }

    file.close();
    return hash_type(hash);
}


/*!
 * Verify checksum of a Intel HEX record
 * \param record a line from an Intel HEX file
 * \return true if record is valid, false otherwise
 */
static bool checksum(std::string *record)
{

    size_t len = record->length();
    unsigned int i;
    unsigned char sum = 0;
    unsigned int val;

    for (i = 1; i < len; i += 2) {
        std::istringstream(record->substr(i, 2)) >> std::hex >> val;
        sum += val;
    }

    if (sum == 0)
       return true;
    else
       return false;
}


/*!
 * Parse Intel HEX record
 *
 * \param record a line from an Intel HEX file
 * \param len output length of record
 * \param addr output address
 * \param type output type
 * \param data output data
 * \return true if record is sucessfully read, false on error
 */
bool parse_record(std::string *record, unsigned int &len,
                  unsigned int &addr, unsigned int &type,
                  unsigned char* data)
{
    unsigned int i;
    std::string _data;
    unsigned int val;

    if (record->substr(0, 1) != ":")
        return false;

    std::istringstream(record->substr(1, 2)) >> std::hex >> len;
    std::istringstream(record->substr(3, 4)) >> std::hex >> addr;
    std::istringstream(record->substr(7, 2)) >> std::hex >> type;

    if (len > (2 * (record->length() - 9)))  // sanity check to prevent buffer overrun
        return false;

    for (i = 0; i < len; i++) {
        std::istringstream(record->substr(9 + 2 * i, 2)) >> std::hex >> val;
        data[i] = (unsigned char) val;
    }

    return true;
}


/*!
 * SMINI control implementation for device discovery and configuration
 */
class fx2_ctrl_impl : public fx2_ctrl {
public:
    fx2_ctrl_impl(shd::transport::usb_control::sptr ctrl_transport)
    {
        _ctrl_transport = ctrl_transport;
    }

    void smini_fx2_reset(void){
        unsigned char reset_y = 1;
        unsigned char reset_n = 0;
        smini_control_write(FX2_FIRMWARE_LOAD, 0xe600, 0, &reset_y, 1);
        smini_control_write(FX2_FIRMWARE_LOAD, 0xe600, 0, &reset_n, 1);
        //wait for things to settle
        boost::this_thread::sleep(boost::posix_time::milliseconds(2000));
    }

    void smini_load_firmware(std::string filestring, bool force)
    {
        const char *filename = filestring.c_str();

        hash_type hash = generate_hash(filename);

        hash_type loaded_hash; smini_get_firmware_hash(loaded_hash);

        if (not force and (hash == loaded_hash)) return;

        //FIXME: verify types
        unsigned int len;
        unsigned int addr;
        unsigned int type;
        unsigned char data[512];

        std::ifstream file;
        file.open(filename, std::ifstream::in);

        if (!file.good()) {
            throw shd::io_error("smini_load_firmware: cannot open firmware input file");
        }

        unsigned char reset_y = 1;
        unsigned char reset_n = 0;

        //hit the reset line
        if (load_img_msg) SHD_MSG(status) << "Loading firmware image: " << filestring << "..." << std::flush;
        smini_control_write(FX2_FIRMWARE_LOAD, 0xe600, 0, &reset_y, 1);

        while (!file.eof()) {
           std::string record;
           file >> record;

            if (!(record.length() > 0))
                continue;

            //check for valid record
            if (not checksum(&record) or not parse_record(&record, len, addr, type, data)) {
                throw shd::io_error("smini_load_firmware: bad record checksum");
            }

            //type 0x00 is data
            if (type == 0x00) {
                int ret = smini_control_write(FX2_FIRMWARE_LOAD, addr, 0, data, len);
                if (ret < 0) throw shd::io_error("smini_load_firmware: smini_control_write failed");
            }
            //type 0x01 is end
            else if (type == 0x01) {
                smini_set_firmware_hash(hash); //set hash before reset
                smini_control_write(FX2_FIRMWARE_LOAD, 0xe600, 0, &reset_n, 1);
                file.close();

                //wait for things to settle
                boost::this_thread::sleep(boost::posix_time::milliseconds(1000));
                if (load_img_msg) SHD_MSG(status) << " done" << std::endl;
                return;
            }
            //type anything else is unhandled
            else {
                throw shd::io_error("smini_load_firmware: unsupported record");
            }
        }

        //file did not end
        throw shd::io_error("smini_load_firmware: bad record");
    }

    void smini_init(void){
        //disable
        smini_rx_enable(false);
        smini_tx_enable(false);

        //toggle resets
        smini_rx_reset(true);
        smini_tx_reset(true);
        smini_rx_reset(false);
        smini_tx_reset(false);
    }

    void smini_load_fpga(std::string filestring)
    {
        const char *filename = filestring.c_str();

        hash_type hash = generate_hash(filename);

        hash_type loaded_hash; smini_get_fpga_hash(loaded_hash);

        if (hash == loaded_hash) return;
        const int ep0_size = 64;
        unsigned char buf[ep0_size];

        if (load_img_msg) SHD_MSG(status) << "Loading FPGA image: " << filestring << "..." << std::flush;
        std::ifstream file;
        file.open(filename, std::ios::in | std::ios::binary);
        if (not file.good()) {
            throw shd::io_error("smini_load_fpga: cannot open fpga input file");
        }

        smini_fpga_reset(true); //holding the fpga in reset while loading

        if (smini_control_write_cmd(VRQ_FPGA_LOAD, 0, FL_BEGIN) < 0) {
            throw shd::io_error("smini_load_fpga: fpga load error");
        }

        while (not file.eof()) {
            file.read((char *)buf, sizeof(buf));
            const std::streamsize n = file.gcount();
            if(n == 0) continue;
            int ret = smini_control_write(VRQ_FPGA_LOAD, 0, FL_XFER, buf, uint16_t(n));
            if (ret < 0 or std::streamsize(ret) != n) {
                throw shd::io_error("smini_load_fpga: fpga load error");
            }
        }

        if (smini_control_write_cmd(VRQ_FPGA_LOAD, 0, FL_END) < 0) {
            throw shd::io_error("smini_load_fpga: fpga load error");
        }

        smini_set_fpga_hash(hash);

        smini_fpga_reset(false); //done loading, take fpga out of reset

        file.close();
        if (load_img_msg) SHD_MSG(status) << " done" << std::endl;
    }

    void smini_load_eeprom(std::string filestring)
    {
        if (load_img_msg) SHD_MSG(status) << "Loading EEPROM image: " << filestring << "..." << std::flush;
        const char *filename = filestring.c_str();
        const uint16_t i2c_addr = 0x50;

        unsigned int addr;
        unsigned char data[256];
        unsigned char sendbuf[17];

        std::ifstream file;
        file.open(filename, std::ifstream::in);

        if (not file.good()) {
            throw shd::io_error("smini_load_eeprom: cannot open EEPROM input file");
        }

        file.read((char *)data, 256);
        std::streamsize len = file.gcount();

        if(len == 256) {
            throw shd::io_error("smini_load_eeprom: image size too large");
        }

        const int pagesize = 16;
        addr = 0;
        while(len > 0) {
            sendbuf[0] = addr;
            memcpy(sendbuf+1, &data[addr], len > pagesize ? pagesize : size_t(len));
            int ret = smini_i2c_write(i2c_addr, sendbuf, (len > pagesize ? pagesize : size_t(len))+1);
            if (ret < 0) {
                throw shd::io_error("smini_load_eeprom: smini_i2c_write failed");
            }
            addr += pagesize;
            len -= pagesize;
            boost::this_thread::sleep(boost::posix_time::milliseconds(100));
        }
        file.close();
        if (load_img_msg) SHD_MSG(status) << " done" << std::endl;
    }


    void smini_set_led(int led_num, bool on)
    {
        SHD_ASSERT_THROW(smini_control_write_cmd(VRQ_SET_LED, on, led_num) >= 0);
    }


    void smini_get_firmware_hash(hash_type &hash)
    {
        SHD_ASSERT_THROW(smini_control_read(0xa0, SMINI_HASH_SLOT_0_ADDR, 0,
                                 (unsigned char*) &hash, sizeof(hash)) >= 0);
    }


    void smini_set_firmware_hash(hash_type hash)
    {
        SHD_ASSERT_THROW(smini_control_write(0xa0, SMINI_HASH_SLOT_0_ADDR, 0,
                                  (unsigned char*) &hash, sizeof(hash)) >= 0);

    }


    void smini_get_fpga_hash(hash_type &hash)
    {
        SHD_ASSERT_THROW(smini_control_read(0xa0, SMINI_HASH_SLOT_1_ADDR, 0,
                                 (unsigned char*) &hash, sizeof(hash)) >= 0);
    }


    void smini_set_fpga_hash(hash_type hash)
    {
        SHD_ASSERT_THROW(smini_control_write(0xa0, SMINI_HASH_SLOT_1_ADDR, 0,
                                  (unsigned char*) &hash, sizeof(hash)) >= 0);
    }

    void smini_tx_enable(bool on)
    {
        SHD_ASSERT_THROW(smini_control_write_cmd(VRQ_FPGA_SET_TX_ENABLE, on, 0) >= 0);
    }


    void smini_rx_enable(bool on)
    {
        SHD_ASSERT_THROW(smini_control_write_cmd(VRQ_FPGA_SET_RX_ENABLE, on, 0) >= 0);
    }


    void smini_tx_reset(bool on)
    {
        SHD_ASSERT_THROW(smini_control_write_cmd(VRQ_FPGA_SET_TX_RESET, on, 0) >= 0);
    }


    void smini_rx_reset(bool on)
    {
        SHD_ASSERT_THROW(smini_control_write_cmd(VRQ_FPGA_SET_RX_RESET, on, 0) >= 0);
    }

    void smini_fpga_reset(bool on)
    {
        SHD_ASSERT_THROW(smini_control_write_cmd(VRQ_FPGA_SET_RESET, on, 0) >= 0);
    }

    int smini_control_write(uint8_t request,
                           uint16_t value,
                           uint16_t index,
                           unsigned char *buff,
                           uint16_t length)
    {
        return _ctrl_transport->submit(VRT_VENDOR_OUT,     // bmReqeustType
                                       request,            // bRequest
                                       value,              // wValue
                                       index,              // wIndex
                                       buff,               // data
                                       length);            // wLength
    }


    int smini_control_read(uint8_t request,
                          uint16_t value,
                          uint16_t index,
                          unsigned char *buff,
                          uint16_t length)
    {
        return _ctrl_transport->submit(VRT_VENDOR_IN,      // bmReqeustType
                                       request,            // bRequest
                                       value,              // wValue
                                       index,              // wIndex
                                       buff,               // data
                                       length);            // wLength
    }


    int smini_control_write_cmd(uint8_t request, uint16_t value, uint16_t index)
    {
        return smini_control_write(request, value, index, 0, 0);
    }

    byte_vector_t read_eeprom(
        uint16_t addr,
        uint16_t offset,
        size_t num_bytes
    ){
        this->write_i2c(addr, byte_vector_t(1, uint8_t(offset)));
        return this->read_i2c(addr, num_bytes);
    }

    int smini_i2c_write(uint16_t i2c_addr, unsigned char *buf, uint16_t len)
    {
        return smini_control_write(VRQ_I2C_WRITE, i2c_addr, 0, buf, len);
    }

    int smini_i2c_read(uint16_t i2c_addr, unsigned char *buf, uint16_t len)
    {
        return smini_control_read(VRQ_I2C_READ, i2c_addr, 0, buf, len);
    }

    static const bool iface_debug = false;
    static const size_t max_i2c_data_bytes = 64;

    void write_i2c(uint16_t addr, const byte_vector_t &bytes)
    {
        SHD_ASSERT_THROW(bytes.size() < max_i2c_data_bytes);

        int ret = this->smini_i2c_write(addr, (unsigned char *)&bytes.front(), bytes.size());

        if (iface_debug && (ret < 0))
            shd::runtime_error("SMINI: failed i2c write");
    }

    byte_vector_t read_i2c(uint16_t addr, size_t num_bytes)
    {
      SHD_ASSERT_THROW(num_bytes < max_i2c_data_bytes);

      byte_vector_t bytes(num_bytes);
      int ret = this->smini_i2c_read(addr, (unsigned char *)&bytes.front(), num_bytes);

      if (iface_debug && ((ret < 0) || (unsigned)ret < (num_bytes)))
          shd::runtime_error("SMINI: failed i2c read");

      return bytes;
    }


private:
    shd::transport::usb_control::sptr _ctrl_transport;
};

/***********************************************************************
 * Public make function for fx2_ctrl interface
 **********************************************************************/
fx2_ctrl::sptr fx2_ctrl::make(shd::transport::usb_control::sptr ctrl_transport){
    return sptr(new fx2_ctrl_impl(ctrl_transport));
}

