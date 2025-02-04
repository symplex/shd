//
// Copyright 2014 Ettus Research LLC
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

#include <shd/exception.hpp>
#include <shd/utils/byteswap.hpp>
#include <shd/transport/udp_simple.hpp>


#include "e300_i2c.hpp"
#include <cstring>

namespace shd { namespace smini { namespace e300 {

class zc_impl : public i2c
{
public:
    zc_impl(shd::transport::zero_copy_if::sptr xport) : _xport(xport)
    {
    }

    virtual ~zc_impl(void)
    {
    }

    void set_i2c_reg8(
        const uint8_t addr,
        const uint8_t reg,
        const uint8_t value)
    {
        i2c_transaction_t transaction;
        transaction.type = WRITE | ONEBYTE;
        transaction.addr = addr;
        transaction.reg  = shd::htonx<uint16_t>(reg);
        transaction.data = value;
        {
            shd::transport::managed_send_buffer::sptr buff = _xport->get_send_buff(10.0);
            if (not buff or buff->size() < sizeof(transaction))
                throw shd::runtime_error("i2c_zc_impl send timeout");
            std::memcpy(buff->cast<void *>(), &transaction, sizeof(transaction));
            buff->commit(sizeof(transaction));
        }
    }

    uint8_t get_i2c_reg8(
        const uint8_t addr,
        const uint8_t reg)
    {
        i2c_transaction_t transaction;
        transaction.type = READ | ONEBYTE;
        transaction.addr = addr;
        transaction.reg = shd::htonx<uint16_t>(reg);
        {
            shd::transport::managed_send_buffer::sptr buff = _xport->get_send_buff(10.0);
            if (not buff or buff->size() < sizeof(transaction))
                throw std::runtime_error("i2c_zc_impl send timeout");
            std::memcpy(buff->cast<void *>(), &transaction, sizeof(transaction));
            buff->commit(sizeof(transaction));
        }
        {
            shd::transport::managed_recv_buffer::sptr buff = _xport->get_recv_buff(10.0);
            if (not buff or buff->size() < sizeof(transaction))
                throw std::runtime_error("i2c_zc_impl recv timeout");
            std::memcpy(&transaction, buff->cast<const void *>(), sizeof(transaction));
        }
        return transaction.data;
    }

    void set_i2c_reg16(
        const uint8_t addr,
        const uint16_t reg,
        const uint8_t value)
    {
        i2c_transaction_t transaction;
        transaction.type = WRITE | TWOBYTE;
        transaction.addr = addr;
        transaction.reg  = shd::htonx<uint16_t>(reg);
        transaction.data = value;
        {
            shd::transport::managed_send_buffer::sptr buff = _xport->get_send_buff(10.0);
            if (not buff or buff->size() < sizeof(transaction))
                throw shd::runtime_error("i2c_zc_impl send timeout");
            std::memcpy(buff->cast<void *>(), &transaction, sizeof(transaction));
            buff->commit(sizeof(transaction));
        }
    }

    uint8_t get_i2c_reg16(
        const uint8_t addr,
        const uint16_t reg)
    {
        i2c_transaction_t transaction;
        transaction.type = READ | TWOBYTE;
        transaction.addr = addr;
        transaction.reg  = shd::htonx<uint16_t>(reg);
        {
            shd::transport::managed_send_buffer::sptr buff = _xport->get_send_buff(10.0);
            if (not buff or buff->size() < sizeof(transaction))
                throw std::runtime_error("i2c_zc_impl send timeout");
            std::memcpy(buff->cast<void *>(), &transaction, sizeof(transaction));
            buff->commit(sizeof(transaction));
        }
        {
            shd::transport::managed_recv_buffer::sptr buff = _xport->get_recv_buff(10.0);
            if (not buff or buff->size() < sizeof(transaction))
                throw std::runtime_error("i2c_zc_impl recv timeout");
            std::memcpy(&transaction, buff->cast<const void *>(), sizeof(transaction));
        }
        return transaction.data;
    }


private:
    shd::transport::zero_copy_if::sptr _xport;
};

i2c::sptr i2c::make_zc(shd::transport::zero_copy_if::sptr xport)
{
    return sptr(new zc_impl(xport));
}

class simple_udp_impl : public i2c
{
public:
    simple_udp_impl(const std::string &ip_addr, const std::string &port)
    {
        _xport = shd::transport::udp_simple::make_connected(ip_addr, port);
    }

    virtual ~simple_udp_impl(void)
    {
    }

    void set_i2c_reg8(
        const uint8_t addr,
        const uint8_t reg,
        const uint8_t value)
    {
        i2c_transaction_t transaction;
        transaction.type = i2c::WRITE | ONEBYTE;
        transaction.addr = addr;
        transaction.reg  = shd::htonx<uint16_t>(reg);
        transaction.data = value;

        _xport->send(
            boost::asio::buffer(
                &transaction,
                sizeof(transaction)));
    }

    uint8_t get_i2c_reg8(
        const uint8_t addr,
        const uint8_t reg)
    {
        i2c_transaction_t transaction;
        transaction.type = i2c::READ | ONEBYTE;
        transaction.addr = addr;
        transaction.reg  = shd::htonx<uint16_t>(reg);
        transaction.data = 0;

        _xport->send(
            boost::asio::buffer(
                &transaction,
                sizeof(transaction)));

        uint8_t buff[sizeof(i2c_transaction_t)] = {};
        const size_t nbytes = _xport->recv(
            boost::asio::buffer(buff), 0.100);
        if (not (nbytes == sizeof(transaction)))
            throw std::runtime_error("i2c_simple_udp_impl recv timeout");
        i2c_transaction_t *reply = reinterpret_cast<i2c_transaction_t*>(buff);
        return reply->data;
    }

    void set_i2c_reg16(
        const uint8_t addr,
        const uint16_t reg,
        const uint8_t value)
    {
        i2c_transaction_t transaction;
        transaction.type = i2c::WRITE | TWOBYTE;
        transaction.addr = addr;
        transaction.reg = shd::htonx<uint16_t>(reg);
        transaction.data = value;

        _xport->send(
            boost::asio::buffer(
                &transaction,
                sizeof(transaction)));
    }

    uint8_t get_i2c_reg16(
        const uint8_t addr,
        const uint16_t reg)
    {
        i2c_transaction_t transaction;
        transaction.type = i2c::READ | TWOBYTE;
        transaction.addr = addr;
        transaction.reg = shd::htonx<uint16_t>(reg);
        transaction.data = 0;

        _xport->send(
            boost::asio::buffer(
                &transaction,
                sizeof(transaction)));

        uint8_t buff[sizeof(i2c_transaction_t)] = {};
        const size_t nbytes = _xport->recv(
            boost::asio::buffer(buff), 0.100);
        if (not (nbytes == sizeof(transaction)))
            throw std::runtime_error("i2c_simple_udp_impl recv timeout");
        i2c_transaction_t *reply = reinterpret_cast<i2c_transaction_t*>(buff);
        return reply->data;
    }

private:
    shd::transport::udp_simple::sptr _xport;
};

i2c::sptr i2c::make_simple_udp(
    const std::string &ip_addr,
    const std::string &port)
{
    return sptr(new simple_udp_impl(ip_addr,port));
}

}}} // namespace

#ifdef E300_NATIVE

#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include <boost/thread.hpp>
#include <stdint.h>

namespace shd { namespace smini { namespace e300 {

class i2cdev_impl : public i2c
{
public:
    i2cdev_impl(const std::string &device)
    {
        _fd = ::open(device.c_str(), O_RDWR);
        if (_fd < 0)
            throw shd::system_error("open failed.");
    }

    virtual ~i2cdev_impl(void)
    {
        close(_fd);
    }

    void set_i2c_reg8(
        const uint8_t addr,
        const uint8_t reg,
        const uint8_t value)
    {
        uint8_t outbuf[2];
        i2c_rdwr_ioctl_data packets;
        i2c_msg messages[1];

        messages[0].addr = addr;
        messages[0].flags = 0;
        messages[0].len = sizeof(outbuf);
        messages[0].buf = outbuf;

        outbuf[0] = reg;
        outbuf[1] = value;

        packets.msgs = messages;
        packets.nmsgs = 1;

        if(::ioctl(_fd, I2C_RDWR, &packets) < 0) {
            throw std::runtime_error("ioctl failed");
        }
        // this is ugly
        boost::this_thread::sleep(boost::posix_time::milliseconds(5));
    }

    uint8_t get_i2c_reg8(
        const uint8_t addr,
        const uint8_t reg)
    {
        i2c_rdwr_ioctl_data packets;
        i2c_msg messages[2];

        uint8_t outbuf = reg;
        messages[0].addr = addr;
        messages[0].flags = 0;
        messages[0].len = sizeof(outbuf);
        messages[0].buf = &outbuf;

        uint8_t inbuf;
        messages[1].addr = addr;
        messages[1].flags = I2C_M_RD;
        messages[1].len = sizeof(inbuf);
        messages[1].buf = &inbuf;

        packets.msgs = messages;
        packets.nmsgs = 2;

        if(::ioctl(_fd, I2C_RDWR, &packets) < 0) {
            throw std::runtime_error("ioctl failed.");
        }

        return inbuf;
    }

    // the daughterboard uses 16 bit addresses
    void set_i2c_reg16(
        const uint8_t addr,
        const uint16_t reg,
        const uint8_t value)
    {
        uint8_t outbuf[3];
        i2c_rdwr_ioctl_data packets;
        i2c_msg messages[1];

        messages[0].addr = addr;
        messages[0].flags = 0;
        messages[0].len = sizeof(outbuf);
        messages[0].buf = outbuf;

        outbuf[0] = (reg >> 8) & 0xff;
        outbuf[1] = reg & 0xff;
        outbuf[2] = value;

        packets.msgs = messages;
        packets.nmsgs = 1;

        if(::ioctl(_fd, I2C_RDWR, &packets) < 0) {
            throw std::runtime_error("ioctl failed");
        }
        // this is ugly
        boost::this_thread::sleep(boost::posix_time::milliseconds(5));
    }


    // the daughterboard eeprom uses 16 bit addresses
    uint8_t get_i2c_reg16(
        const uint8_t addr,
        const uint16_t reg)
    {
        i2c_rdwr_ioctl_data packets;
        i2c_msg messages[2];

        // always little endian
        uint8_t outbuf[2];
        outbuf[0] = (reg >> 8) & 0xff;
        outbuf[1] = reg & 0xff;

        messages[0].addr = addr;
        messages[0].flags = 0;
        messages[0].len = sizeof(outbuf);
        messages[0].buf = outbuf;

        uint8_t inbuf;
        messages[1].addr = addr;
        messages[1].flags = I2C_M_RD;
        messages[1].len = sizeof(inbuf);
        messages[1].buf = &inbuf;

        packets.msgs = messages;
        packets.nmsgs = 2;

        if(::ioctl(_fd, I2C_RDWR, &packets) < 0) {
            throw std::runtime_error("ioctl failed.");
        }

        return inbuf;
    }

private:
    int _fd;
};

}}} // namespace

using namespace shd::smini::e300;

i2c::sptr i2c::make_i2cdev(const std::string &device)
{
    return sptr(new i2cdev_impl(device));
}
#else
using namespace shd::smini::e300;

i2c::sptr i2c::make_i2cdev(const std::string &)
{
    throw shd::assertion_error("i2c::make() !E300_NATIVE");
}
#endif // E300_NATIVE
