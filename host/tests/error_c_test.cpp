//
// Copyright 2015 Ettus Research LLC
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

#include <shd/error.h>
#include <shd/types/dict.hpp>

#include <boost/test/unit_test.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/assign.hpp>
#include <boost/format.hpp>

/*
 * Test our conversions from exceptions to C-level SHD error codes.
 * We use inline functions separate from the Boost test cases themselves
 * to test our C++ macro, which returns the error code.
 */

typedef struct {
    std::string last_error;
} dummy_handle_t;

template <typename error_type>
SHD_INLINE shd_error throw_shd_exception(dummy_handle_t *handle, const shd::exception* except){
    SHD_SAFE_C_SAVE_ERROR(handle,
        throw *dynamic_cast<const error_type*>(except);
    )
}

SHD_INLINE shd_error throw_boost_exception(dummy_handle_t *handle){
    SHD_SAFE_C_SAVE_ERROR(handle,
        std::runtime_error except("This is a std::runtime_error, thrown by Boost.");
        BOOST_THROW_EXCEPTION(except);
    )
}

SHD_INLINE shd_error throw_std_exception(dummy_handle_t *handle){
    SHD_SAFE_C_SAVE_ERROR(handle,
        throw std::runtime_error("This is a std::runtime_error.");
    )
}

SHD_INLINE shd_error throw_unknown_exception(dummy_handle_t *handle){
    SHD_SAFE_C_SAVE_ERROR(handle,
        throw 1;
    )
}

// There are enough non-standard names that we can't just use a conversion function
static const shd::dict<std::string, std::string> pretty_exception_names =
    boost::assign::map_list_of
        ("assertion_error",       "AssertionError")
        ("lookup_error",          "LookupError")
        ("index_error",           "LookupError: IndexError")
        ("key_error",             "LookupError: KeyError")
        ("type_error",            "TypeError")
        ("value_error",           "ValueError")
        ("runtime_error",         "RuntimeError")
        ("not_implemented_error", "RuntimeError: NotImplementedError")
        ("usb_error",             "RuntimeError: USBError 1")
        ("environment_error",     "EnvironmentError")
        ("io_error",              "EnvironmentError: IOError")
        ("os_error",              "EnvironmentError: OSError")
        ("system_error",          "SystemError")
    ;

#define SHD_TEST_CHECK_ERROR_CODE(cpp_exception_type, c_error_code) \
    expected_msg = str(boost::format("This is a shd::%s.") % BOOST_STRINGIZE(cpp_exception_type)); \
    shd::cpp_exception_type cpp_exception_type ## _foo(expected_msg); \
    error_code = throw_shd_exception<shd::cpp_exception_type>(&handle, &cpp_exception_type ## _foo); \
    BOOST_CHECK_EQUAL(error_code, c_error_code); \
    expected_msg = str(boost::format("%s: %s") \
                       % pretty_exception_names.get(BOOST_STRINGIZE(cpp_exception_type)) \
                       % expected_msg); \
    BOOST_CHECK_EQUAL(handle.last_error, expected_msg); \
    BOOST_CHECK_EQUAL(get_c_global_error_string(), expected_msg);

// shd::usb_error has a different constructor
#define SHD_TEST_CHECK_USB_ERROR_CODE() \
    expected_msg = "This is a shd::usb_error."; \
    shd::usb_error usb_error_foo(1, expected_msg); \
    error_code = throw_shd_exception<shd::usb_error>(&handle, &usb_error_foo); \
    BOOST_CHECK_EQUAL(error_code, SHD_ERROR_USB); \
    expected_msg = str(boost::format("%s: %s") \
                       % pretty_exception_names.get("usb_error") \
                       % expected_msg); \
    BOOST_CHECK_EQUAL(handle.last_error, expected_msg); \
    BOOST_CHECK_EQUAL(get_c_global_error_string(), expected_msg);

BOOST_AUTO_TEST_CASE(test_shd_exception){
    dummy_handle_t handle;
    std::string expected_msg;
    shd_error error_code;

    SHD_TEST_CHECK_ERROR_CODE(assertion_error,       SHD_ERROR_ASSERTION);
    SHD_TEST_CHECK_ERROR_CODE(lookup_error,          SHD_ERROR_LOOKUP);
    SHD_TEST_CHECK_ERROR_CODE(index_error,           SHD_ERROR_INDEX);
    SHD_TEST_CHECK_ERROR_CODE(key_error,             SHD_ERROR_KEY);
    SHD_TEST_CHECK_ERROR_CODE(type_error,            SHD_ERROR_TYPE);
    SHD_TEST_CHECK_ERROR_CODE(value_error,           SHD_ERROR_VALUE);
    SHD_TEST_CHECK_ERROR_CODE(runtime_error,         SHD_ERROR_RUNTIME);
    SHD_TEST_CHECK_ERROR_CODE(not_implemented_error, SHD_ERROR_NOT_IMPLEMENTED);
    SHD_TEST_CHECK_ERROR_CODE(io_error,              SHD_ERROR_IO);
    SHD_TEST_CHECK_ERROR_CODE(os_error,              SHD_ERROR_OS);
    SHD_TEST_CHECK_ERROR_CODE(system_error,          SHD_ERROR_SYSTEM);
    SHD_TEST_CHECK_USB_ERROR_CODE();
}

BOOST_AUTO_TEST_CASE(test_boost_exception){
    dummy_handle_t handle;
    shd_error error_code = throw_boost_exception(&handle);

    // Boost error message cannot be determined here, so just check code
    BOOST_CHECK_EQUAL(error_code, SHD_ERROR_BOOSTEXCEPT);
}

BOOST_AUTO_TEST_CASE(test_std_exception){
    dummy_handle_t handle;
    shd_error error_code = throw_std_exception(&handle);

    BOOST_CHECK_EQUAL(error_code, SHD_ERROR_STDEXCEPT);
    BOOST_CHECK_EQUAL(handle.last_error, "This is a std::runtime_error.");
}

BOOST_AUTO_TEST_CASE(test_unknown_exception){
    dummy_handle_t handle;
    shd_error error_code = throw_unknown_exception(&handle);

    BOOST_CHECK_EQUAL(error_code, SHD_ERROR_UNKNOWN);
    BOOST_CHECK_EQUAL(handle.last_error, "Unrecognized exception caught.");
}
