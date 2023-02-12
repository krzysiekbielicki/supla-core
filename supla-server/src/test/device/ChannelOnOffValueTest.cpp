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

#include <device/ChannelOnOffValueTest.h>

#include "device/channel_onoff_value.h"
#include "devicechannel.h"  // NOLINT
#include "gtest/gtest.h"

namespace {

class ChannelOnOffValueTest : public ::testing::Test {
 protected:
};

TEST_F(ChannelOnOffValueTest, voidConstructor) {
  supla_channel_onoff_value v;
  EXPECT_FALSE(v.is_on());
}

TEST_F(ChannelOnOffValueTest, rawDataConstructor) {
  char raw_value[SUPLA_CHANNELVALUE_SIZE] = {};
  supla_channel_onoff_value v1(raw_value);
  EXPECT_FALSE(v1.is_on());

  raw_value[0] = 1;
  supla_channel_onoff_value v2(raw_value);
  EXPECT_TRUE(v2.is_on());
}

TEST_F(ChannelOnOffValueTest, setterAndGetter) {
  char raw_value[SUPLA_CHANNELVALUE_SIZE] = {};
  supla_channel_onoff_value v;
  v.get_raw_value(raw_value);
  EXPECT_EQ(raw_value[0], 0);
  EXPECT_FALSE(v.is_on());
  v.set_on(true);
  v.get_raw_value(raw_value);
  EXPECT_EQ(raw_value[0], 1);
}

}  // namespace
