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

#ifndef INCLUDED_TRACE_H
#define INCLUDED_TRACE_H

#include <stdint.h>
#include <stdbool.h>
#include <printf.h>

/*
 * Enables basic conditional tracing support
 * If SHD_FW_TRACE_LEVEL is defined, all messages
 * with a verbosity >= SHD_FW_TRACE_LEVEL will be
 * printed.
 *
 * An alternate way of defining the level is the "TRACE_LEVEL"
 * variable in cmake. (eg. -DTRACE_LEVEL=13).
 */

//#define SHD_FW_TRACE_LEVEL 13

typedef enum
{
    /* 0-9: Use for performance/restricted debugging */
    ERROR = 10,
    WARN  = 11,
    INFO  = 12,
    /* 13-19: Use for general debugging */
    DEBUG = 20, //Verbose!
} trace_level_t;

static inline int _trace_typecheck_conv(trace_level_t lvl) {
    return (int)lvl;
}

#define SHD_FW_PRINTF(...) printf(__VA_ARGS__)
#define SHD_FW_BEAUTIFY_LVL(lvl) "[" #lvl "] "

/*
 * SHD_FW_TRACE(<log level>, <message>)
 *  - Simple trace. Print the messages with the log level as the prefix and \n at the end
 *
 * SHD_FW_TRACE_FSTR(<log level>, <format string>, <args>)
 *  - Trace format string with the log level as the prefix and \n at the end
 *
 * SHD_FW_TRACE_SHORT(<log level>, <message>)
 *  - Simple trace. Print the messages without the log level prefix or \n at the end
 *
 * SHD_FW_TRACE_FSTR_SHORT(<log level>, <format string>, <args>)
 *  - Trace format string without the log level prefix or \n at the end
 */
#ifdef SHD_FW_TRACE_LEVEL
    #define SHD_FW_TRACE(lvl, fmt) \
        if (SHD_FW_TRACE_LEVEL >= _trace_typecheck_conv(lvl)) SHD_FW_PRINTF(SHD_FW_BEAUTIFY_LVL(lvl) fmt "\r\n");
    #define SHD_FW_TRACE_FSTR(lvl, fmt, ...) \
        if (SHD_FW_TRACE_LEVEL >= _trace_typecheck_conv(lvl)) SHD_FW_PRINTF(SHD_FW_BEAUTIFY_LVL(lvl) fmt "\r\n", __VA_ARGS__);
    #define SHD_FW_TRACE_SHORT(lvl, fmt) \
        if (SHD_FW_TRACE_LEVEL >= _trace_typecheck_conv(lvl)) SHD_FW_PRINTF(fmt);
    #define SHD_FW_TRACE_FSTR_SHORT(lvl, fmt, ...) \
        if (SHD_FW_TRACE_LEVEL >= _trace_typecheck_conv(lvl)) SHD_FW_PRINTF(fmt, __VA_ARGS__);
#else
    #define SHD_FW_TRACE(lvl, fmt) ;
    #define SHD_FW_TRACE_FSTR(lvl, fmt, ...) ;
    #define SHD_FW_TRACE_SHORT(lvl, fmt) ;
    #define SHD_FW_TRACE_FSTR_SHORT(lvl, fmt, ...) ;
#endif

#endif /* INCLUDED_TRACE_H */
