/*
 Copyright (C) AC SOFTWARE SP. Z O.O.

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include <device/value/ChannelHvacValueTest.h>

#include "device/value/channel_hvac_value.h"
#include "devicechannel.h"  // NOLINT

namespace testing {

TEST_F(ChannelHvacValueTest, mode) {
  supla_channel_hvac_value value;
  EXPECT_EQ(value.get_mode(), SUPLA_HVAC_MODE_NOT_SET);
  value.set_mode(100);
  EXPECT_EQ(value.get_mode(), SUPLA_HVAC_MODE_NOT_SET);
  value.set_mode(SUPLA_HVAC_MODE_HEAT);
  EXPECT_EQ(value.get_mode(), SUPLA_HVAC_MODE_HEAT);
}

TEST_F(ChannelHvacValueTest, turnOn) {
  supla_channel_hvac_value value;
  EXPECT_EQ(value.get_mode(), SUPLA_HVAC_MODE_NOT_SET);
  value.turn_on();
  EXPECT_EQ(value.get_mode(), SUPLA_HVAC_MODE_CMD_TURN_ON);
}

TEST_F(ChannelHvacValueTest, turnOff) {
  supla_channel_hvac_value value;
  value.set_mode(SUPLA_HVAC_MODE_HEAT);
  value.turn_off();
  EXPECT_EQ(value.get_mode(), SUPLA_HVAC_MODE_OFF);
}

TEST_F(ChannelHvacValueTest, switchToManual) {
  supla_channel_hvac_value value;
  EXPECT_EQ(value.get_mode(), SUPLA_HVAC_MODE_NOT_SET);
  value.switch_to_manual();
  EXPECT_EQ(value.get_mode(), SUPLA_HVAC_MODE_CMD_SWITCH_TO_MANUAL);
}

TEST_F(ChannelHvacValueTest, switchToProgram) {
  supla_channel_hvac_value value;
  EXPECT_EQ(value.get_mode(), SUPLA_HVAC_MODE_NOT_SET);
  value.switch_to_program();
  EXPECT_EQ(value.get_mode(), SUPLA_HVAC_MODE_CMD_WEEKLY_SCHEDULE);
}

TEST_F(ChannelHvacValueTest, clear) {
  char raw_value1[SUPLA_CHANNELVALUE_SIZE] = {};
  char raw_value2[SUPLA_CHANNELVALUE_SIZE] = {};

  supla_channel_hvac_value value;
  value.set_temperature_heat(1);
  value.set_temperature_cool(2);
  value.turn_on();

  value.get_raw_value(raw_value2);
  EXPECT_NE(memcmp(raw_value1, raw_value2, SUPLA_CHANNELVALUE_SIZE), 0);

  value.clear();

  value.get_raw_value(raw_value2);
  EXPECT_EQ(memcmp(raw_value1, raw_value2, SUPLA_CHANNELVALUE_SIZE), 0);
}

TEST_F(ChannelHvacValueTest, isOn) {
  supla_channel_hvac_value value;
  EXPECT_FALSE(value.is_on());

  char raw_value[SUPLA_CHANNELVALUE_SIZE] = {};
  ((THVACValue*)raw_value)->IsOn = 1;
  value.set_raw_value(raw_value);

  EXPECT_TRUE(value.is_on());
}

TEST_F(ChannelHvacValueTest, isHeating) {
  supla_channel_hvac_value value;
  EXPECT_FALSE(value.is_heating());

  char raw_value[SUPLA_CHANNELVALUE_SIZE] = {};
  ((THVACValue*)raw_value)->IsOn = 1;
  ((THVACValue*)raw_value)->Flags = SUPLA_HVAC_VALUE_FLAG_HEATING;
  value.set_raw_value(raw_value);

  EXPECT_TRUE(value.is_heating());
}

TEST_F(ChannelHvacValueTest, isCooling) {
  supla_channel_hvac_value value;
  EXPECT_FALSE(value.is_cooling());

  char raw_value[SUPLA_CHANNELVALUE_SIZE] = {};
  ((THVACValue*)raw_value)->IsOn = 1;
  ((THVACValue*)raw_value)->Flags = SUPLA_HVAC_VALUE_FLAG_COOLING;
  value.set_raw_value(raw_value);

  EXPECT_TRUE(value.is_cooling());
}

TEST_F(ChannelHvacValueTest, getTemperature) {
  supla_channel_hvac_value value;
  EXPECT_EQ(value.get_temperature(), 0);
  EXPECT_DOUBLE_EQ(value.get_temperature_dbl(), 0.0);

  char raw_value[SUPLA_CHANNELVALUE_SIZE] = {};
  ((THVACValue*)raw_value)->SetpointTemperatureCool = 1234;
  ((THVACValue*)raw_value)->SetpointTemperatureHeat = 5678;
  value.set_raw_value(raw_value);

  EXPECT_EQ(value.get_temperature(), 5678);
  EXPECT_DOUBLE_EQ(value.get_temperature_dbl(), 56.78);

  value.set_mode(SUPLA_HVAC_MODE_COOL);

  EXPECT_EQ(value.get_temperature(), 1234);
  EXPECT_DOUBLE_EQ(value.get_temperature_dbl(), 12.34);
}

}  // namespace testing
