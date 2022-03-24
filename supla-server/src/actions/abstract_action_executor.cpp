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

#include "actions/abstract_action_executor.h"

#include "converter/any_value_to_action_converter.h"

supla_abstract_action_executor::supla_abstract_action_executor(void) {
  this->user = NULL;
  this->device_id = 0;
  this->subject_id = 0;
  this->is_group = false;
}

supla_abstract_action_executor::supla_abstract_action_executor(supla_user *user,
                                                               int device_id,
                                                               int channel_id) {
  set_channel_id(user, device_id, channel_id);
}

supla_abstract_action_executor::supla_abstract_action_executor(int user_id,
                                                               int device_id,
                                                               int channel_id) {
  set_channel_id(user_id, device_id, channel_id);
}

supla_abstract_action_executor::supla_abstract_action_executor(supla_user *user,
                                                               int group_id) {
  set_group_id(user, group_id);
}

supla_abstract_action_executor::supla_abstract_action_executor(int user_id,
                                                               int group_id) {
  set_group_id(user_id, group_id);
}

supla_abstract_action_executor::~supla_abstract_action_executor(void) {}

void supla_abstract_action_executor::set_channel_id(supla_user *user,
                                                    int device_id,
                                                    int channel_id) {
  this->user = user;
  this->device_id = device_id;
  this->subject_id = channel_id;
  this->is_group = false;
}

void supla_abstract_action_executor::set_channel_id(int user_id, int device_id,
                                                    int channel_id) {
  this->user = user_id ? supla_user::find(user_id, false) : NULL;
  this->device_id = device_id;
  this->subject_id = channel_id;
  this->is_group = false;
}

void supla_abstract_action_executor::set_group_id(supla_user *user,
                                                  int group_id) {
  this->user = user;
  this->device_id = 0;
  this->subject_id = group_id;
  this->is_group = true;
}

void supla_abstract_action_executor::set_group_id(int user_id, int group_id) {
  this->user = user_id ? supla_user::find(user_id, false) : NULL;
  this->device_id = 0;
  this->subject_id = group_id;
  this->is_group = true;
}

void supla_abstract_action_executor::access_device(
    std::function<void(supla_device *device)> on_device) {
  if (user && (device_id || (subject_id && !is_group))) {
    return user->access_device(device_id, is_group ? 0 : subject_id, on_device);
  }
}

void supla_abstract_action_executor::execute_action(
    int user_id, abstract_action_config *config,
    supla_abstract_value_getter *value_getter) {
  int action_id = 0;
  int subject_id = 0;

  if (!config || (action_id = config->get_action_id()) == 0 ||
      (subject_id = config->get_subject_id()) == 0) {
    return;
  }

  if (config->get_subject_type() == stChannelGroup) {
    set_group_id(user_id, subject_id);
  } else {
    set_channel_id(user_id, 0, subject_id);
  }

  switch (action_id) {
    case ACTION_OPEN:
      open();
      break;
    case ACTION_CLOSE:
      close();
      break;
    case ACTION_SHUT:
      shut(NULL);
      break;
    case ACTION_REVEAL:
      reveal();
      break;
    case ACTION_UP_OR_STOP:
      up_or_stop();
      break;
    case ACTION_DOWN_OR_STOP:
      down_or_stop();
      break;
    case ACTION_STEP_BY_STEP:
      step_by_step();
      break;
    case ACTION_SHUT_PARTIALLY:
    case ACTION_REVEAL_PARTIALLY: {
      char percentage = config->get_percentage();
      if (percentage > -1) {
        if (action_id == ACTION_REVEAL_PARTIALLY) {
          percentage = 100 - percentage;
        }

        shut(&percentage);
      }
    } break;
    case ACTION_TURN_ON:
      set_on(true);
      break;
    case ACTION_TURN_OFF:
      set_on(false);
      break;
    case ACTION_SET_RGBW_PARAMETERS: {
      _action_config_rgbw_t rgbw = config->get_rgbw();
      if (rgbw.brightness > -1 || rgbw.color_brightness > -1 || rgbw.color) {
        set_rgbw(rgbw.color ? &rgbw.color : NULL,
                 rgbw.color_brightness > -1 ? &rgbw.color_brightness : NULL,
                 rgbw.brightness > -1 ? &rgbw.brightness : NULL, NULL);
      }
    } break;
    case ACTION_OPEN_CLOSE:
      open_close();
      break;
    case ACTION_STOP:
      stop();
      break;
    case ACTION_TOGGLE:
      toggle();
      break;
    case ACTION_COPY:
      copy(value_getter, config->get_source_device_id(),
           config->get_source_channel_id());
      break;
    case ACTION_FORWARD_OUTSIDE:
      forward_outside(config->get_cap());
      break;
  }
}

void supla_abstract_action_executor::execute_action(
    std::function<void(supla_user_channelgroups *, supla_device_channels *)>
        f) {
  supla_user_channelgroups *channel_groups = get_channel_groups();
  if (channel_groups) {
    f(channel_groups, NULL);
  } else {
    access_device([f](supla_device *device) -> void {
      supla_device_channels *channels = device->get_channels();
      if (channels) {
        f(NULL, channels);
      }
    });
  }
}

supla_user_channelgroups *supla_abstract_action_executor::get_channel_groups(
    void) {
  return is_group && user ? user->get_channel_groups() : NULL;
}

supla_user *supla_abstract_action_executor::get_user(void) { return user; }

int supla_abstract_action_executor::get_user_id(void) {
  return user ? user->getUserID() : 0;
}

int supla_abstract_action_executor::get_device_id(void) { return device_id; }

int supla_abstract_action_executor::get_channel_id(void) {
  return is_group ? 0 : subject_id;
}

int supla_abstract_action_executor::get_group_id(void) {
  return is_group ? subject_id : 0;
}

void supla_abstract_action_executor::copy(
    supla_abstract_value_getter *value_getter, int source_device_id,
    int source_channel_id) {
  if (value_getter) {
    supla_channel_value *value = NULL;
    if ((value = value_getter->get_value(get_user_id(), source_device_id,
                                         source_channel_id))) {
      any_value_to_action_converter *converter =
          new any_value_to_action_converter();
      if (converter) {
        converter->convert(value, this);
        delete converter;
      }

      delete value;
      value = NULL;
    }
  }
}
