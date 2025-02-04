//
// Copyright 2011-2014 Ettus Research LLC
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

#include "db_wbx_common.hpp"
#include <shd/utils/log.hpp>
#include <shd/types/dict.hpp>
#include <shd/types/ranges.hpp>
#include <shd/types/sensors.hpp>
#include <shd/utils/assert_has.hpp>
#include <shd/utils/algorithm.hpp>
#include <shd/utils/msg.hpp>
#include <shd/smini/dboard_base.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/format.hpp>
#include <boost/math/special_functions/round.hpp>
#include <boost/algorithm/string.hpp>

using namespace shd;
using namespace shd::smini;
using namespace boost::assign;


/***********************************************************************
 * WBX Version 4 Constants
 **********************************************************************/
static const shd::dict<std::string, gain_range_t> wbx_v4_tx_gain_ranges = map_list_of
    ("PGA0", gain_range_t(0, 31, 1.0))
;

static const freq_range_t wbx_v4_freq_range(25.0e6, 2.2e9);


/***********************************************************************
 * Gain-related functions
 **********************************************************************/
static int tx_pga0_gain_to_iobits(double &gain){
    //clip the input
    gain = wbx_v4_tx_gain_ranges["PGA0"].clip(gain);

    //convert to attenuation
    double attn = wbx_v4_tx_gain_ranges["PGA0"].stop() - gain;

    //calculate the attenuation
    int attn_code = boost::math::iround(attn);
    int iobits = (
            (attn_code & 16 ? 0 : TX_ATTN_16) |
            (attn_code &  8 ? 0 : TX_ATTN_8) |
            (attn_code &  4 ? 0 : TX_ATTN_4) |
            (attn_code &  2 ? 0 : TX_ATTN_2) |
            (attn_code &  1 ? 0 : TX_ATTN_1)
        ) & TX_ATTN_MASK;

    SHD_LOGV(often) << boost::format(
        "WBX TX Attenuation: %f dB, Code: %d, IO Bits %x, Mask: %x"
    ) % attn % attn_code % (iobits & TX_ATTN_MASK) % TX_ATTN_MASK << std::endl;

    //the actual gain setting
    gain = wbx_v4_tx_gain_ranges["PGA0"].stop() - double(attn_code);

    return iobits;
}


/***********************************************************************
 * WBX Common Implementation
 **********************************************************************/
wbx_base::wbx_version4::wbx_version4(wbx_base *_self_wbx_base) {
    //register our handle on the primary wbx_base instance
    self_base = _self_wbx_base;
    _txlo = adf435x_iface::make_adf4351(boost::bind(&wbx_base::wbx_versionx::write_lo_regs, this, dboard_iface::UNIT_TX, _1));
    _rxlo = adf435x_iface::make_adf4351(boost::bind(&wbx_base::wbx_versionx::write_lo_regs, this, dboard_iface::UNIT_RX, _1));

    ////////////////////////////////////////////////////////////////////
    // Register RX properties
    ////////////////////////////////////////////////////////////////////
    uint16_t rx_id = _self_wbx_base->get_rx_id().to_uint16();

    if(rx_id == 0x0063) this->get_rx_subtree()->create<std::string>("name").set("WBXv4 RX");
    else if(rx_id == 0x0081) this->get_rx_subtree()->create<std::string>("name").set("WBX-120 RX");
    this->get_rx_subtree()->create<double>("freq/value")
         .set_coercer(boost::bind(&wbx_base::wbx_version4::set_lo_freq, this, dboard_iface::UNIT_RX, _1))
         .set((wbx_v4_freq_range.start() + wbx_v4_freq_range.stop())/2.0);
    this->get_rx_subtree()->create<meta_range_t>("freq/range").set(wbx_v4_freq_range);

    ////////////////////////////////////////////////////////////////////
    // Register TX properties
    ////////////////////////////////////////////////////////////////////

    //get_tx_id() will always return GDB ID, so use RX ID to determine WBXv4 vs. WBX-120
    if(rx_id == 0x0063) this->get_tx_subtree()->create<std::string>("name").set("WBXv4 TX");
    else if(rx_id == 0x0081) this->get_tx_subtree()->create<std::string>("name").set("WBX-120 TX");
    BOOST_FOREACH(const std::string &name, wbx_v4_tx_gain_ranges.keys()){
        self_base->get_tx_subtree()->create<double>("gains/"+name+"/value")
            .set_coercer(boost::bind(&wbx_base::wbx_version4::set_tx_gain, this, _1, name))
            .set(wbx_v4_tx_gain_ranges[name].start());
        self_base->get_tx_subtree()->create<meta_range_t>("gains/"+name+"/range")
            .set(wbx_v4_tx_gain_ranges[name]);
    }
    this->get_tx_subtree()->create<double>("freq/value")
         .set_coercer(boost::bind(&wbx_base::wbx_version4::set_lo_freq, this, dboard_iface::UNIT_TX, _1))
         .set((wbx_v4_freq_range.start() + wbx_v4_freq_range.stop())/2.0);
    this->get_tx_subtree()->create<meta_range_t>("freq/range").set(wbx_v4_freq_range);
    this->get_tx_subtree()->create<bool>("enabled")
        .add_coerced_subscriber(boost::bind(&wbx_base::wbx_version4::set_tx_enabled, this, _1))
        .set(true); //start enabled

    //set attenuator control bits
    int v4_iobits = TX_ATTN_MASK;
    int v4_tx_mod = ADF435X_PDBRF;

    //set the gpio directions and atr controls
    self_base->get_iface()->set_pin_ctrl(dboard_iface::UNIT_TX, \
            v4_tx_mod|v4_iobits);
    self_base->get_iface()->set_pin_ctrl(dboard_iface::UNIT_RX, \
            RXBB_PDB|ADF435X_PDBRF);
    self_base->get_iface()->set_gpio_ddr(dboard_iface::UNIT_TX, \
            TX_PUP_5V|TX_PUP_3V|v4_tx_mod|v4_iobits);
    self_base->get_iface()->set_gpio_ddr(dboard_iface::UNIT_RX, \
            RX_PUP_5V|RX_PUP_3V|ADF435X_CE|RXBB_PDB|ADF435X_PDBRF|RX_ATTN_MASK);

    //setup ATR for the mixer enables (always enabled to prevent phase slip
    //between bursts) set TX gain iobits to min gain (max attenuation) when
    //RX_ONLY or IDLE to suppress LO leakage
    self_base->get_iface()->set_atr_reg(dboard_iface::UNIT_TX, \
            gpio_atr::ATR_REG_IDLE, v4_tx_mod, \
            TX_ATTN_MASK | TX_MIXER_DIS | v4_tx_mod);
    self_base->get_iface()->set_atr_reg(dboard_iface::UNIT_TX, \
            gpio_atr::ATR_REG_RX_ONLY, v4_tx_mod, \
            TX_ATTN_MASK | TX_MIXER_DIS | v4_tx_mod);
    self_base->get_iface()->set_atr_reg(dboard_iface::UNIT_TX, \
            gpio_atr::ATR_REG_TX_ONLY, v4_tx_mod, \
            TX_ATTN_MASK | TX_MIXER_DIS | v4_tx_mod);
    self_base->get_iface()->set_atr_reg(dboard_iface::UNIT_TX, \
            gpio_atr::ATR_REG_FULL_DUPLEX, v4_tx_mod, \
            TX_ATTN_MASK | TX_MIXER_DIS | v4_tx_mod);

    self_base->get_iface()->set_atr_reg(dboard_iface::UNIT_RX, \
            gpio_atr::ATR_REG_IDLE, \
            RX_MIXER_ENB, RX_MIXER_DIS | RX_MIXER_ENB);
    self_base->get_iface()->set_atr_reg(dboard_iface::UNIT_RX, \
            gpio_atr::ATR_REG_TX_ONLY, \
            RX_MIXER_ENB, RX_MIXER_DIS | RX_MIXER_ENB);
    self_base->get_iface()->set_atr_reg(dboard_iface::UNIT_RX, \
            gpio_atr::ATR_REG_RX_ONLY, \
            RX_MIXER_ENB, RX_MIXER_DIS | RX_MIXER_ENB);
    self_base->get_iface()->set_atr_reg(dboard_iface::UNIT_RX, \
            gpio_atr::ATR_REG_FULL_DUPLEX, \
            RX_MIXER_ENB, RX_MIXER_DIS | RX_MIXER_ENB);
}

wbx_base::wbx_version4::~wbx_version4(void){
    /* NOP */
}


/***********************************************************************
 * Enables
 **********************************************************************/
void wbx_base::wbx_version4::set_tx_enabled(bool enb) {
    self_base->get_iface()->set_gpio_out(dboard_iface::UNIT_TX,
        (enb)? TX_POWER_UP | ADF435X_CE : TX_POWER_DOWN, TX_POWER_UP | TX_POWER_DOWN | 0);
}


/***********************************************************************
 * Gain Handling
 **********************************************************************/
double wbx_base::wbx_version4::set_tx_gain(double gain, const std::string &name) {
    assert_has(wbx_v4_tx_gain_ranges.keys(), name, "wbx tx gain name");
    if(name == "PGA0"){
        uint16_t io_bits = tx_pga0_gain_to_iobits(gain);

        self_base->_tx_gains[name] = gain;

        //write the new gain to tx gpio outputs
        //Update ATR with gain io_bits, only update for TX_ONLY and FULL_DUPLEX ATR states
        self_base->get_iface()->set_atr_reg(dboard_iface::UNIT_TX, gpio_atr::ATR_REG_TX_ONLY,     io_bits, TX_ATTN_MASK);
        self_base->get_iface()->set_atr_reg(dboard_iface::UNIT_TX, gpio_atr::ATR_REG_FULL_DUPLEX, io_bits, TX_ATTN_MASK);

    }
    else SHD_THROW_INVALID_CODE_PATH();
    return self_base->_tx_gains[name];
}


/***********************************************************************
 * Tuning
 **********************************************************************/
double wbx_base::wbx_version4::set_lo_freq(dboard_iface::unit_t unit, double target_freq) {
    //clip to tuning range
    target_freq = wbx_v4_freq_range.clip(target_freq);

    SHD_LOGV(often) << boost::format(
        "WBX tune: target frequency %f MHz"
    ) % (target_freq/1e6) << std::endl;

    /*
     * If the user sets 'mode_n=integer' in the tuning args, the user wishes to
     * tune in Integer-N mode, which can result in better spur
     * performance on some mixers. The default is fractional tuning.
     */
    property_tree::sptr subtree = (unit == dboard_iface::UNIT_RX) ? self_base->get_rx_subtree()
                                                                  : self_base->get_tx_subtree();
    device_addr_t tune_args = subtree->access<device_addr_t>("tune_args").get();
    bool is_int_n = boost::iequals(tune_args.get("mode_n",""), "integer");
    double reference_freq = self_base->get_iface()->get_clock_rate(unit);

    //Select the LO
    adf435x_iface::sptr& lo_iface = unit == dboard_iface::UNIT_RX ? _rxlo : _txlo;
    lo_iface->set_reference_freq(reference_freq);

    //The mixer has a divide-by-2 stage on the LO port so the synthesizer
    //frequency must 2x the target frequency.  This introduces a 180 degree phase
    //ambiguity when trying to synchronize the phase of multiple boards.
    double synth_target_freq = target_freq * 2;

    //Use 8/9 prescaler for vco_freq > 3 GHz (pg.18 prescaler)
    lo_iface->set_prescaler(synth_target_freq > 3.6e9 ?
        adf435x_iface::PRESCALER_8_9 : adf435x_iface::PRESCALER_4_5);

    //The feedback of the divided frequency must be disabled whenever the target frequency
    //divided by the minimum PFD frequency cannot meet the minimum integer divider (N) value.
    //If it is disabled, additional phase ambiguity will be introduced.  With a minimum PFD
    //frequency of 10 MHz, synthesizer frequencies below 230 MHz (LO frequencies below 115 MHz)
    //will have too much ambiguity to synchronize.
    lo_iface->set_feedback_select(
        (int(synth_target_freq / 10e6) >= lo_iface->get_int_range().start() ?
            adf435x_iface::FB_SEL_DIVIDED : adf435x_iface::FB_SEL_FUNDAMENTAL));

    double synth_actual_freq = lo_iface->set_frequency(synth_target_freq, is_int_n);

    //The mixer has a divide-by-2 stage on the LO port so the synthesizer
    //actual_freq must /2 the synth_actual_freq
    double actual_freq = synth_actual_freq / 2;

    if (unit == dboard_iface::UNIT_RX) {
        lo_iface->set_output_power((actual_freq == wbx_rx_lo_5dbm.clip(actual_freq)) ?
            adf435x_iface::OUTPUT_POWER_5DBM : adf435x_iface::OUTPUT_POWER_2DBM);
    } else {
        lo_iface->set_output_power((actual_freq == wbx_tx_lo_5dbm.clip(actual_freq)) ?
            adf435x_iface::OUTPUT_POWER_5DBM : adf435x_iface::OUTPUT_POWER_M1DBM);
    }

    //Write to hardware
    lo_iface->commit();

    return actual_freq;
}
