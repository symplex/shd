//
// Copyright 2011 Ettus Research LLC
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

#include <shd/utils/msg.hpp>
#include <shd/utils/log.hpp>
#include <shd/utils/static.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/foreach.hpp>
#include <boost/tokenizer.hpp>
#include <sstream>
#include <iostream>

/***********************************************************************
 * Helper functions
 **********************************************************************/
#define tokenizer(inp, sep) \
    boost::tokenizer<boost::char_separator<char> > \
    (inp, boost::char_separator<char>(sep))

static void msg_to_cout(const std::string &msg){
    std::stringstream ss;

    static bool just_had_a_newline = true;
    BOOST_FOREACH(char ch, msg){
        if (just_had_a_newline){
            just_had_a_newline = false;
            ss << "-- ";
        }
        if (ch == '\n'){
            just_had_a_newline = true;
        }
        ss << ch;
    }

    std::cout << ss.str() << std::flush;
}

static void msg_to_cerr(const std::string &title, const std::string &msg){
    std::stringstream ss;

    ss << std::endl << title << ":" << std::endl;
    BOOST_FOREACH(const std::string &line, tokenizer(msg, "\n")){
        ss << "    " << line << std::endl;
    }

    std::cerr << ss.str() << std::flush;
}

/***********************************************************************
 * Global resources for the messenger
 **********************************************************************/
struct msg_resource_type{
    boost::mutex mutex;
    shd::msg::handler_t handler;
};

SHD_SINGLETON_FCN(msg_resource_type, msg_rs);

/***********************************************************************
 * Setup the message handlers
 **********************************************************************/
void shd::msg::register_handler(const handler_t &handler){
    boost::mutex::scoped_lock lock(msg_rs().mutex);
    msg_rs().handler = handler;
}

static void default_msg_handler(shd::msg::type_t type, const std::string &msg){
    static boost::mutex msg_mutex;
    boost::mutex::scoped_lock lock(msg_mutex);
    switch(type){
    case shd::msg::fastpath:
        std::cerr << msg << std::flush;
        break;

    case shd::msg::status:
        msg_to_cout(msg);
        SHD_LOG << "Status message" << std::endl << msg;
        break;

    case shd::msg::warning:
        msg_to_cerr("SHD Warning", msg);
        SHD_LOG << "Warning message" << std::endl << msg;
        break;

    case shd::msg::error:
        msg_to_cerr("SHD Error", msg);
        SHD_LOG << "Error message" << std::endl << msg;
        break;
    }
}

SHD_STATIC_BLOCK(msg_register_default_handler){
    shd::msg::register_handler(&default_msg_handler);
}

/***********************************************************************
 * The message object implementation
 **********************************************************************/
struct shd::msg::_msg::impl{
    std::ostringstream ss;
    type_t type;
};

shd::msg::_msg::_msg(const type_t type){
    _impl = SHD_PIMPL_MAKE(impl, ());
    _impl->type = type;
}

shd::msg::_msg::~_msg(void){
    boost::mutex::scoped_lock lock(msg_rs().mutex);
    msg_rs().handler(_impl->type, _impl->ss.str());
}

std::ostream & shd::msg::_msg::operator()(void){
    return _impl->ss;
}
