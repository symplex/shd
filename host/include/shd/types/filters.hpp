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

#ifndef INCLUDED_SHD_TYPES_FILTERS_HPP
#define INCLUDED_SHD_TYPES_FILTERS_HPP

#include <shd/config.hpp>
#include <shd/utils/log.hpp>
#include <shd/utils/msg.hpp>
#include <stdint.h>
#include <boost/shared_ptr.hpp>
#include <boost/scoped_array.hpp>
#include <string>
#include <vector>
#include <iostream>
#include <ostream>
#include <sstream>

namespace shd{

    class SHD_API filter_info_base
    {
    public:
        typedef boost::shared_ptr<filter_info_base> sptr;
        enum filter_type
        {
            ANALOG_LOW_PASS,
            ANALOG_BAND_PASS,
            DIGITAL_I16,
            DIGITAL_FIR_I16
        };

        filter_info_base(
            filter_type type,
            bool bypass,
            size_t position_index
        ):
            _type(type), _bypass(bypass),
            _position_index(position_index)
        {
            //NOP
        }

        SHD_INLINE virtual bool is_bypassed()
        {
            return _bypass;
        }

        SHD_INLINE filter_type get_type()
        {
            return _type;
        }

        virtual ~filter_info_base()
        {
            //NOP
        }

        virtual std::string to_pp_string();

    protected:
        filter_type _type;
        bool _bypass;
        size_t _position_index;

    };

    SHD_API std::ostream& operator<<(std::ostream& os, filter_info_base& f);

    class SHD_API analog_filter_base : public filter_info_base
    {
        std::string _analog_type;
        public:
            typedef boost::shared_ptr<analog_filter_base> sptr;
            analog_filter_base(
                filter_type type,
                bool bypass,
                size_t position_index,
                const std::string& analog_type
            ):
                filter_info_base(type, bypass, position_index),
                _analog_type(analog_type)
            {
                //NOP
            }

            SHD_INLINE const std::string& get_analog_type()
            {
                return _analog_type;
            }

            virtual std::string to_pp_string();
    };

    class SHD_API analog_filter_lp : public analog_filter_base
    {
        double _cutoff;
        double _rolloff;

    public:
        typedef boost::shared_ptr<analog_filter_lp> sptr;
        analog_filter_lp(
            filter_type type,
            bool bypass,
            size_t position_index,
            const std::string& analog_type,
            double cutoff,
            double rolloff
        ):
            analog_filter_base(type, bypass, position_index, analog_type),
            _cutoff(cutoff),
            _rolloff(rolloff)
        {
            //NOP
        }

        SHD_INLINE double get_cutoff()
        {
            return _cutoff;
        }

        SHD_INLINE double get_rolloff()
        {
            return _cutoff;
        }

        SHD_INLINE void set_cutoff(const double cutoff)
        {
            _cutoff = cutoff;
        }

        virtual std::string to_pp_string();
    };

    template<typename tap_t>
    class SHD_API digital_filter_base : public filter_info_base
    {
    protected:
        double _rate;
        uint32_t _interpolation;
        uint32_t _decimation;
        tap_t _tap_full_scale;
        uint32_t _max_num_taps;
        std::vector<tap_t> _taps;

    public:
        typedef boost::shared_ptr<digital_filter_base> sptr;
        digital_filter_base(
            filter_type type,
            bool bypass,
            size_t position_index,
            double rate,
            size_t interpolation,
            size_t decimation,
            double tap_full_scale,
            size_t max_num_taps,
            const std::vector<tap_t>& taps
        ):
            filter_info_base(type, bypass, position_index),
            _rate(rate),
            _interpolation(interpolation),
            _decimation(decimation),
            _tap_full_scale(tap_full_scale),
            _max_num_taps(max_num_taps),
            _taps(taps)
        {
            //NOP
        }

        SHD_INLINE double get_output_rate()
        {
            return (_bypass ? _rate : (_rate / _decimation * _interpolation));
        }

        SHD_INLINE double get_input_rate()
        {
            return _rate;
        }

        SHD_INLINE double get_interpolation()
        {
            return _interpolation;
        }

        SHD_INLINE double get_decimation()
        {
            return _decimation;
        }

        SHD_INLINE double get_tap_full_scale()
        {
            return _tap_full_scale;
        }

        SHD_INLINE std::vector<tap_t>& get_taps()
        {
            return _taps;
        }

        virtual std::string to_pp_string()
        {
            std::ostringstream os;
            os<<filter_info_base::to_pp_string()<<
            "\t[digital_filter_base]"<<std::endl<<
            "\tinput rate: "<<_rate<<std::endl<<
            "\tinterpolation: "<<_interpolation<<std::endl<<
            "\tdecimation: "<<_decimation<<std::endl<<
            "\tfull-scale: "<<_tap_full_scale<<std::endl<<
            "\tmax num taps: "<<_max_num_taps<<std::endl<<
            "\ttaps: "<<std::endl;

            os<<"\t\t";
            for(size_t i = 0; i < _taps.size(); i++)
            {
                os<<"(tap "<<i<<": "<<_taps[i]<<")";
                if( ((i%10) == 0) && (i != 0))
                {
                    os<<std::endl<<"\t\t";
                }
            }
            os<<std::endl;
            return std::string(os.str());
        }

    };

    template<typename tap_t>
    class SHD_API digital_filter_fir : public digital_filter_base<tap_t>
    {
    public:
        typedef boost::shared_ptr<digital_filter_fir<tap_t> > sptr;

        digital_filter_fir(
            filter_info_base::filter_type type,
            bool bypass, size_t position_index,
            double rate,
            size_t interpolation,
            size_t decimation,
            size_t tap_bit_width,
            size_t max_num_taps,
            const std::vector<tap_t>& taps
        ):
            digital_filter_base<tap_t>(type, bypass, position_index, rate, interpolation, decimation, tap_bit_width, max_num_taps, taps)
        {
            //NOP
        }

        void set_taps(const std::vector<tap_t>& taps)
        {
            std::size_t num_taps = taps.size();
            if(num_taps < this->_max_num_taps){
                SHD_MSG(warning) << "digital_filter_fir::set_taps not enough coefficients. Appending zeros";
                std::vector<tap_t> coeffs;
                for (size_t i = 0; i < this->_max_num_taps; i++)
                {
                    if(i < num_taps)
                    {
                        coeffs.push_back(taps[i]);
                    } else {
                        coeffs.push_back(0);
                    }
                }
                this->_taps = coeffs;
            } else {
                this->_taps = taps;
            }
        }
    };

} //namespace shd

#endif /* INCLUDED_SHD_TYPES_FILTERS_HPP */
