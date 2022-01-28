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

#include <converter/abstract_value_to_action_converter.h>

abstract_value_to_action_converter::abstract_value_to_action_converter(
    abstract_value_getter *value_getter,
    supla_abstract_action_executor *action_executor) {
  this->value_getter = value_getter;
  this->action_executor = action_executor;
}

abstract_value_to_action_converter::~abstract_value_to_action_converter() {}

bool abstract_value_to_action_converter::convert(void) {
  char value[SUPLA_CHANNELVALUE_SIZE] = {};

  if (!value_getter || !action_executor || !value_getter->get_value(value)) {
    return false;
  }

  return convert(value, action_executor);
}
