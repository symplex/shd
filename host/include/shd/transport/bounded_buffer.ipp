//
// Copyright 2010-2013 Ettus Research LLC
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

#ifndef INCLUDED_SHD_TRANSPORT_BOUNDED_BUFFER_IPP
#define INCLUDED_SHD_TRANSPORT_BOUNDED_BUFFER_IPP

#include <shd/config.hpp>
#include <boost/bind.hpp>
#include <boost/utility.hpp>
#include <boost/function.hpp>
#include <boost/circular_buffer.hpp>
#include <boost/thread/condition.hpp>
#include <boost/thread/locks.hpp>

namespace shd{ namespace transport{

    template <typename elem_type> class bounded_buffer_detail : boost::noncopyable
    {
    public:

        bounded_buffer_detail(size_t capacity):
            _buffer(capacity)
        {
            _not_full_fcn  = boost::bind(&bounded_buffer_detail<elem_type>::not_full, this);
            _not_empty_fcn = boost::bind(&bounded_buffer_detail<elem_type>::not_empty, this);
        }

        SHD_INLINE bool push_with_haste(const elem_type &elem)
        {
            boost::mutex::scoped_lock lock(_mutex);
            if (_buffer.full())
            {
                return false;
            }
            _buffer.push_front(elem);
            _empty_cond.notify_one();
            return true;
        }

        SHD_INLINE bool push_with_pop_on_full(const elem_type &elem)
        {
            boost::mutex::scoped_lock lock(_mutex);
            if (_buffer.full())
            {
                _buffer.pop_back();
                _buffer.push_front(elem);
                _empty_cond.notify_one();
                return false;
            }
            else {
                _buffer.push_front(elem);
                _empty_cond.notify_one();
                return true;
            }
        }

        SHD_INLINE void push_with_wait(const elem_type &elem)
        {
            boost::mutex::scoped_lock lock(_mutex);
            if (_buffer.full())
            {
                _full_cond.wait(lock, _not_full_fcn);
            }
            _buffer.push_front(elem);
            _empty_cond.notify_one();
        }

        SHD_INLINE bool push_with_timed_wait(const elem_type &elem, double timeout)
        {
            boost::mutex::scoped_lock lock(_mutex);
            if (_buffer.full())
            {
                if (not _full_cond.timed_wait(lock,
                    to_time_dur(timeout), _not_full_fcn))
                {
                    return false;
                }
            }
            _buffer.push_front(elem);
            _empty_cond.notify_one();
            return true;
        }

        SHD_INLINE bool pop_with_haste(elem_type &elem)
        {
            boost::mutex::scoped_lock lock(_mutex);
            if (_buffer.empty()) 
            {
                return false;
            }
            this->pop_back(elem);
            _full_cond.notify_one();
            return true;
        }

        SHD_INLINE void pop_with_wait(elem_type &elem)
        {
            boost::mutex::scoped_lock lock(_mutex);
            if (_buffer.empty())
            {
                _empty_cond.wait(lock, _not_empty_fcn);
            }
            this->pop_back(elem);
            _full_cond.notify_one();
        }

        SHD_INLINE bool pop_with_timed_wait(elem_type &elem, double timeout)
        {
            boost::mutex::scoped_lock lock(_mutex);
            if (_buffer.empty()) 
            {
                if (not _empty_cond.timed_wait(lock, to_time_dur(timeout),
                    _not_empty_fcn))
                {
                    return false;
                }
            }
            this->pop_back(elem);
            _full_cond.notify_one();
            return true;
        }

    private:
        boost::mutex _mutex;
        boost::condition _empty_cond, _full_cond;
        boost::circular_buffer<elem_type> _buffer;

        bool not_full(void) const{return not _buffer.full();}
        bool not_empty(void) const{return not _buffer.empty();}

        boost::function<bool(void)> _not_full_fcn, _not_empty_fcn;

        /*!
         * Three part operation to pop an element:
         * 1) assign elem to the back element
         * 2) assign the back element to empty
         * 3) pop the back to move the counter
         */
        SHD_INLINE void pop_back(elem_type &elem)
        {
            elem = _buffer.back();
            _buffer.back() = elem_type();
            _buffer.pop_back();
        }

        static SHD_INLINE boost::posix_time::time_duration to_time_dur(double timeout)
        {
            return boost::posix_time::microseconds(long(timeout*1e6));
        }

    };
}} //namespace

#endif /* INCLUDED_SHD_TRANSPORT_BOUNDED_BUFFER_IPP */
