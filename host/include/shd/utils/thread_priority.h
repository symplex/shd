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

#ifndef INCLUDED_SHD_UTILS_THREAD_PRIORITY_H
#define INCLUDED_SHD_UTILS_THREAD_PRIORITY_H

#include <shd/config.h>
#include <shd/error.h>

#ifdef __cplusplus
extern "C" {
#endif

static const float shd_default_thread_priority = 0.5;

/*!
 * Set the scheduling priority on the current thread.
 *
 * A new thread or calling process should make this call
 * with the defaults this to enable realtime scheduling.
 *
 * A priority of zero corresponds to normal priority.
 * Positive priority values are higher than normal.
 * Negative priority values are lower than normal.
 *
 * \param priority a value between -1 and 1
 * \param realtime true to use realtime mode
 * \return SHD error code
 */
SHD_API shd_error shd_set_thread_priority(
    float priority,
    bool realtime
);

#ifdef __cplusplus
}
#endif

#endif /* INCLUDED_SHD_UTILS_THREAD_PRIORITY_H */
