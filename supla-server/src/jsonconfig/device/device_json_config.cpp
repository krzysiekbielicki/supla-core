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

#include "device_json_config.h"

using std::map;
using std::string;

const map<unsigned _supla_int16_t, string> device_json_config::field_map = {
    {SUPLA_DEVICE_CONFIG_FIELD_STATUS_LED, "statusLed"},
    {SUPLA_DEVICE_CONFIG_FIELD_SCREEN_BRIGHTNESS, "screenBrightness"},
    {SUPLA_DEVICE_CONFIG_FIELD_BUTTON_VOLUME, "buttonVolume"},
    {SUPLA_DEVICE_CONFIG_FIELD_DISABLE_USER_INTERFACE, "userInterfaceDisabled"},
    {SUPLA_DEVICE_CONFIG_FIELD_AUTOMATIC_TIME_SYNC, "automaticTimeSync"},
    {SUPLA_DEVICE_CONFIG_FIELD_SCREENSAVER_DELAY, "delay"},
    {SUPLA_DEVICE_CONFIG_FIELD_SCREENSAVER_MODE, "mode"}};

const map<unsigned _supla_int16_t, string>
    device_json_config::screen_saver_modes_map = {
        {SUPLA_DEVCFG_SCREENSAVER_MODE_OFF, "OFF"},
        {SUPLA_DEVCFG_SCREENSAVER_MODE_TEMPERATURE, "TEMPERATURE"},
        {SUPLA_DEVCFG_SCREENSAVER_MODE_HUMIDITY, "HUMIDITY"},
        {SUPLA_DEVCFG_SCREENSAVER_MODE_TIME, "TIME"},
        {SUPLA_DEVCFG_SCREENSAVER_MODE_TIME_DATE, "TIME_DATE"},
        {SUPLA_DEVCFG_SCREENSAVER_MODE_TEMPERATURE_TIME, "TEMPERATURE_TIME"},
        {SUPLA_DEVCFG_SCREENSAVER_MODE_MAIN_AND_AUX_TEMPERATURE,
         "MAIN_AND_AUX_TEMPERATURE"}};

const char device_json_config::modesAvailable[] = "modesAvailable";

device_json_config::device_json_config(void) : supla_json_config() {}

device_json_config::device_json_config(supla_json_config *root)
    : supla_json_config(root) {}

device_json_config::~device_json_config(void) {}

string device_json_config::status_led_to_string(unsigned char status) {
  switch (status) {
    case SUPLA_DEVCFG_STATUS_LED_OFF_WHEN_CONNECTED:
      return "OFF_WHEN_CONNECTED";
    case SUPLA_DEVCFG_STATUS_LED_ALWAYS_OFF:
      return "ALWAYS_OFF";
  }

  return "ON_WHEN_CONNECTED";
}

unsigned char device_json_config::string_to_status_led(
    const std::string &status) {
  if (status == "OFF_WHEN_CONNECTED") {
    return SUPLA_DEVCFG_STATUS_LED_OFF_WHEN_CONNECTED;
  } else if (status == "ALWAYS_OFF") {
    return SUPLA_DEVCFG_STATUS_LED_ALWAYS_OFF;
  }

  return SUPLA_DEVCFG_STATUS_LED_ON_WHEN_CONNECTED;
}

string device_json_config::screen_saver_mode_to_string(unsigned char mode) {
  for (auto it = screen_saver_modes_map.cbegin();
       it != screen_saver_modes_map.cend(); ++it) {
    if (it->first == mode) {
      return it->second;
    }
  }

  return "";
}

unsigned char device_json_config::string_to_screen_saver_mode(
    const std::string &mode) {
  for (auto it = screen_saver_modes_map.cbegin();
       it != screen_saver_modes_map.cend(); ++it) {
    if (it->second == mode) {
      return it->first;
    }
  }

  return SUPLA_DEVCFG_SCREENSAVER_MODE_OFF;
}

void device_json_config::set_status_led(TDeviceConfig_StatusLed *status_led) {
  if (status_led && status_led->StatusLedType >= 0 &&
      status_led->StatusLedType <= 2) {
    set_item_value(get_user_root(),
                   field_map.at(SUPLA_DEVICE_CONFIG_FIELD_STATUS_LED),
                   cJSON_String, true,
                   status_led_to_string(status_led->StatusLedType).c_str(), 0);
  }
}

void device_json_config::set_screen_brightness(
    TDeviceConfig_ScreenBrightness *brightness) {
  if (brightness) {
    if (brightness->Automatic == 1) {
      set_item_value(get_user_root(),
                     field_map.at(SUPLA_DEVICE_CONFIG_FIELD_SCREEN_BRIGHTNESS),
                     cJSON_String, true, "auto", 0);
    } else if (brightness->ScreenBrightness >= 0 &&
               brightness->ScreenBrightness <= 100) {
      set_item_value(get_user_root(),
                     field_map.at(SUPLA_DEVICE_CONFIG_FIELD_SCREEN_BRIGHTNESS),
                     cJSON_Number, true, nullptr, brightness->ScreenBrightness);
    }
  }
}

void device_json_config::set_button_volume(TDeviceConfig_ButtonVolume *volume) {
  unsigned char v = volume ? volume->Volume : 0;

  if (v <= 100) {
    set_item_value(get_user_root(),
                   field_map.at(SUPLA_DEVICE_CONFIG_FIELD_BUTTON_VOLUME),
                   cJSON_Number, true, nullptr, v);
  }
}

void device_json_config::set_user_interface_disabled(
    TDeviceConfig_DisableUserInterface *disabled) {
  if (disabled && (disabled->DisableUserInterface == 0 ||
                   disabled->DisableUserInterface == 1)) {
    set_item_value(
        get_user_root(),
        field_map.at(SUPLA_DEVICE_CONFIG_FIELD_DISABLE_USER_INTERFACE),
        disabled->DisableUserInterface ? cJSON_True : cJSON_False, true,
        nullptr, 0);
  }
}

void device_json_config::set_automatic_time_sync(
    TDeviceConfig_AutomaticTimeSync *time_sync) {
  if (time_sync && (time_sync->AutomaticTimeSync == 0 ||
                    time_sync->AutomaticTimeSync == 1)) {
    set_item_value(get_user_root(),
                   field_map.at(SUPLA_DEVICE_CONFIG_FIELD_AUTOMATIC_TIME_SYNC),
                   time_sync->AutomaticTimeSync ? cJSON_True : cJSON_False,
                   true, nullptr, 0);
  }
}

cJSON *device_json_config::get_root(bool create,
                                    unsigned _supla_int64_t field) {
  cJSON *root = get_user_root();

  if (field != SUPLA_DEVICE_CONFIG_FIELD_SCREENSAVER_DELAY &&
      field != SUPLA_DEVICE_CONFIG_FIELD_SCREENSAVER_MODE) {
    return root;
  }

  if (root) {
    const char name[] = "screenSaver";
    cJSON *result = cJSON_GetObjectItem(root, name);
    if (!result && create) {
      result = cJSON_AddObjectToObject(root, name);
    }

    return result;
  }
  return nullptr;
}

void device_json_config::set_screen_saver_delay(
    TDeviceConfig_ScreensaverDelay *delay) {
  if (delay) {
    set_item_value(get_root(true, SUPLA_DEVICE_CONFIG_FIELD_SCREENSAVER_DELAY),
                   field_map.at(SUPLA_DEVICE_CONFIG_FIELD_SCREENSAVER_DELAY),
                   cJSON_Number, true, nullptr, delay->ScreensaverDelayMs);
  }
}

void device_json_config::set_screen_saver_mode(
    TDeviceConfig_ScreensaverMode *mode) {
  cJSON *root = get_root(true, SUPLA_DEVICE_CONFIG_FIELD_SCREENSAVER_MODE);
  string mode_str = screen_saver_mode_to_string(mode->ScreensaverMode);
  if (!mode_str.empty()) {
    set_item_value(root,
                   field_map.at(SUPLA_DEVICE_CONFIG_FIELD_SCREENSAVER_MODE),
                   cJSON_String, true, mode_str.c_str(), 0);
  }

  cJSON *modes = cJSON_GetObjectItem(root, modesAvailable);

  if (modes) {
    cJSON_DetachItemViaPointer(root, modes);
    cJSON_Delete(modes);
  }

  modes = cJSON_AddArrayToObject(root, modesAvailable);
  if (modes) {
    unsigned char size = sizeof(mode->ModesAvailable) * 8;
    unsigned _supla_int64_t n = 1;
    for (unsigned char a = 0; a < size; a++) {
      if (mode->ModesAvailable & n) {
        mode_str = screen_saver_mode_to_string(n);
        if (!mode_str.empty()) {
          cJSON *mode_json = cJSON_CreateString(mode_str.c_str());
          if (mode_json) {
            cJSON_AddItemToArray(modes, mode_json);
          }
        }
      }
      n <<= 1;
    }
  }
}

void device_json_config::set_config(TSDS_SetDeviceConfig *config) {
  if (!config) {
    return;
  }

  unsigned short offset = 0;

  // map is sorted
  for (auto it = field_map.cbegin(); it != field_map.cend(); ++it) {
    if (config->Fields & it->first) {
      unsigned _supla_int16_t left = config->ConfigSize;
      if (left > SUPLA_DEVICE_CONFIG_MAXSIZE) {
        left = SUPLA_DEVICE_CONFIG_MAXSIZE;
      }
      if (left > offset) {
        left -= offset;
      } else {
        return;
      }

      void *ptr = &config->Config[offset];
      unsigned short size = 0;

      switch (it->first) {
        case SUPLA_DEVICE_CONFIG_FIELD_STATUS_LED:
          if (left >= (size = sizeof(TDeviceConfig_StatusLed))) {
            set_status_led(static_cast<TDeviceConfig_StatusLed *>(ptr));
          }
          break;
        case SUPLA_DEVICE_CONFIG_FIELD_SCREEN_BRIGHTNESS:
          if (left >= (size = sizeof(TDeviceConfig_ScreenBrightness))) {
            set_screen_brightness(
                static_cast<TDeviceConfig_ScreenBrightness *>(ptr));
          }
          break;
        case SUPLA_DEVICE_CONFIG_FIELD_BUTTON_VOLUME:
          if (left >= (size = sizeof(TDeviceConfig_ButtonVolume))) {
            set_button_volume(static_cast<TDeviceConfig_ButtonVolume *>(ptr));
          }
          break;
        case SUPLA_DEVICE_CONFIG_FIELD_DISABLE_USER_INTERFACE:
          if (left >= (size = sizeof(TDeviceConfig_DisableUserInterface))) {
            set_user_interface_disabled(
                static_cast<TDeviceConfig_DisableUserInterface *>(ptr));
          }
          break;
        case SUPLA_DEVICE_CONFIG_FIELD_AUTOMATIC_TIME_SYNC:
          if (left >= (size = sizeof(TDeviceConfig_AutomaticTimeSync))) {
            set_automatic_time_sync(
                static_cast<TDeviceConfig_AutomaticTimeSync *>(ptr));
          }
          break;
        case SUPLA_DEVICE_CONFIG_FIELD_SCREENSAVER_DELAY:
          if (left >= (size = sizeof(TDeviceConfig_ScreensaverDelay))) {
            set_screen_saver_delay(
                static_cast<TDeviceConfig_ScreensaverDelay *>(ptr));
          }
          break;
        case SUPLA_DEVICE_CONFIG_FIELD_SCREENSAVER_MODE:
          if (left >= (size = sizeof(TDeviceConfig_ScreensaverMode))) {
            set_screen_saver_mode(
                static_cast<TDeviceConfig_ScreensaverMode *>(ptr));
          }
          break;
      }

      offset += size;
    }
  }
}

bool device_json_config::get_status_led(TDeviceConfig_StatusLed *status_led) {
  std::string str_value;
  if (status_led &&
      get_string(get_user_root(),
                 field_map.at(SUPLA_DEVICE_CONFIG_FIELD_STATUS_LED).c_str(),
                 &str_value)) {
    status_led->StatusLedType = string_to_status_led(str_value);
    return true;
  }
  return false;
}

bool device_json_config::get_screen_brightness(
    TDeviceConfig_ScreenBrightness *brightness) {
  double value = 0;
  if (brightness &&
      get_double(
          get_user_root(),
          field_map.at(SUPLA_DEVICE_CONFIG_FIELD_SCREEN_BRIGHTNESS).c_str(),
          &value) &&
      value >= 0 && value <= 100) {
    brightness->Automatic = 0;
    brightness->ScreenBrightness = value;
    return true;
  }

  std::string str_value;
  if (brightness &&
      get_string(
          get_user_root(),
          field_map.at(SUPLA_DEVICE_CONFIG_FIELD_SCREEN_BRIGHTNESS).c_str(),
          &str_value) &&
      str_value == "auto") {
    brightness->Automatic = 1;
    brightness->ScreenBrightness = 0;
    return true;
  }

  return false;
}

bool device_json_config::get_button_volume(TDeviceConfig_ButtonVolume *volume) {
  double value = 0;
  if (volume &&
      get_double(get_user_root(),
                 field_map.at(SUPLA_DEVICE_CONFIG_FIELD_BUTTON_VOLUME).c_str(),
                 &value) &&
      value >= 0 && value <= 100) {
    volume->Volume = value;
    return true;
  }

  return false;
}

bool device_json_config::get_user_interface_disabled(
    TDeviceConfig_DisableUserInterface *disabled) {
  bool value = 0;
  if (disabled &&
      get_bool(get_user_root(),
               field_map.at(SUPLA_DEVICE_CONFIG_FIELD_DISABLE_USER_INTERFACE)
                   .c_str(),
               &value)) {
    disabled->DisableUserInterface = value ? 1 : 0;
    return true;
  }

  return false;
}

bool device_json_config::get_automatic_time_sync(
    TDeviceConfig_AutomaticTimeSync *time_sync) {
  bool value = 0;
  if (time_sync &&
      get_bool(
          get_user_root(),
          field_map.at(SUPLA_DEVICE_CONFIG_FIELD_AUTOMATIC_TIME_SYNC).c_str(),
          &value)) {
    time_sync->AutomaticTimeSync = value ? 1 : 0;
    return true;
  }

  return false;
}

bool device_json_config::get_screen_saver_delay(
    TDeviceConfig_ScreensaverDelay *delay) {
  double value = 0;
  if (delay &&
      get_double(
          get_root(false, SUPLA_DEVICE_CONFIG_FIELD_SCREENSAVER_DELAY),
          field_map.at(SUPLA_DEVICE_CONFIG_FIELD_SCREENSAVER_DELAY).c_str(),
          &value)) {
    delay->ScreensaverDelayMs = value;
    return true;
  }

  return false;
}

bool device_json_config::get_screen_saver_mode(
    TDeviceConfig_ScreensaverMode *mode) {
  std::string str_value;
  bool result = false;

  if (mode) {
    cJSON *root = get_root(false, SUPLA_DEVICE_CONFIG_FIELD_SCREENSAVER_DELAY);
    if (get_string(
            root,
            field_map.at(SUPLA_DEVICE_CONFIG_FIELD_SCREENSAVER_MODE).c_str(),
            &str_value)) {
      mode->ScreensaverMode = string_to_screen_saver_mode(str_value);
      result = true;
    }

    mode->ModesAvailable = 0;

    cJSON *modes = cJSON_GetObjectItem(root, modesAvailable);

    if (modes && cJSON_IsArray(modes)) {
      result = true;
      for (int a = 0; a < cJSON_GetArraySize(modes); a++) {
        cJSON *item = cJSON_GetArrayItem(modes, a);
        if (item && cJSON_IsString(item)) {
          mode->ModesAvailable |=
              string_to_screen_saver_mode(cJSON_GetStringValue(item));
        }
      }
    }
  }
  return result;
}

unsigned _supla_int64_t device_json_config::get_available_fields(void) {
  unsigned _supla_int64_t result = 0;
  for (auto it = field_map.cbegin(); it != field_map.cend(); ++it) {
    cJSON *root = get_root(false, it->first);
    if (root && cJSON_GetObjectItem(root, it->second.c_str())) {
      result |= it->first;
    }
  }
  return result;
}

void device_json_config::get_config(TSDS_SetDeviceConfig *config,
                                    unsigned _supla_int64_t fields,
                                    unsigned _supla_int64_t *fields_left) {
  if (fields_left) {
    *fields_left = 0;
    // map is sorted
    for (auto it = field_map.cbegin(); it != field_map.cend(); ++it) {
      if (fields & it->first) {
        *fields_left |= it->first;
      }
    }
  }

  if (!config) {
    return;
  }

  config->Fields = 0;
  config->ConfigSize = 0;

  // map is sorted
  for (auto it = field_map.cbegin(); it != field_map.cend(); ++it) {
    if (fields & it->first) {
      void *ptr = &config->Config[config->ConfigSize];
      unsigned short size = 0;
      bool field_set = false;

      unsigned _supla_int16_t left =
          SUPLA_DEVICE_CONFIG_MAXSIZE - config->ConfigSize;

      switch (it->first) {
        case SUPLA_DEVICE_CONFIG_FIELD_STATUS_LED:
          field_set =
              left >= (size = sizeof(TDeviceConfig_StatusLed)) &&
              get_status_led(static_cast<TDeviceConfig_StatusLed *>(ptr));
          break;
        case SUPLA_DEVICE_CONFIG_FIELD_SCREEN_BRIGHTNESS:
          field_set = left >= (size = sizeof(TDeviceConfig_ScreenBrightness)) &&
                      get_screen_brightness(
                          static_cast<TDeviceConfig_ScreenBrightness *>(ptr));
          break;
        case SUPLA_DEVICE_CONFIG_FIELD_BUTTON_VOLUME:
          field_set =
              left >= (size = sizeof(TDeviceConfig_ButtonVolume)) &&
              get_button_volume(static_cast<TDeviceConfig_ButtonVolume *>(ptr));
          break;
        case SUPLA_DEVICE_CONFIG_FIELD_DISABLE_USER_INTERFACE:
          field_set =
              left >= (size = sizeof(TDeviceConfig_DisableUserInterface)) &&
              get_user_interface_disabled(
                  static_cast<TDeviceConfig_DisableUserInterface *>(ptr));
          break;
        case SUPLA_DEVICE_CONFIG_FIELD_AUTOMATIC_TIME_SYNC:
          field_set =
              left >= (size = sizeof(TDeviceConfig_AutomaticTimeSync)) &&
              get_automatic_time_sync(
                  static_cast<TDeviceConfig_AutomaticTimeSync *>(ptr));
          break;
        case SUPLA_DEVICE_CONFIG_FIELD_SCREENSAVER_DELAY:
          field_set = left >= (size = sizeof(TDeviceConfig_ScreensaverDelay)) &&
                      get_screen_saver_delay(
                          static_cast<TDeviceConfig_ScreensaverDelay *>(ptr));
          break;
        case SUPLA_DEVICE_CONFIG_FIELD_SCREENSAVER_MODE:
          field_set = left >= (size = sizeof(TDeviceConfig_ScreensaverMode)) &&
                      get_screen_saver_mode(
                          static_cast<TDeviceConfig_ScreensaverMode *>(ptr));
          break;
      }

      if (field_set) {
        config->Fields |= it->first;
        config->ConfigSize += size;
      } else if (size > left) {
        return;
      }

      if (fields_left) {
        *fields_left &= ~(it->first);
      }

      if (config->ConfigSize >= SUPLA_DEVICE_CONFIG_MAXSIZE) {
        return;
      }
    }
  }
}

void device_json_config::get_config(TSDS_SetDeviceConfig *config,
                                    unsigned _supla_int64_t *fields_left) {
  get_config(config, 0xFFFFFFFFFFFFFFFF, fields_left);
}

void device_json_config::remove_empty_sub_roots() {
  cJSON *root = get_user_root();
  for (auto it = field_map.cbegin(); it != field_map.cend(); ++it) {
    cJSON *item = get_root(false, it->first);
    if (item && root != item && !item->child) {
      cJSON_DetachItemViaPointer(root, item);
      cJSON_Delete(item);
    }
  }
}

void device_json_config::leave_only_thise_fields(
    unsigned _supla_int64_t fields) {
  for (auto it = field_map.cbegin(); it != field_map.cend(); ++it) {
    if (!(fields & it->first)) {
      cJSON *root = get_root(false, it->first);
      if (root) {
        cJSON *item = cJSON_GetObjectItem(root, it->second.c_str());
        if (item) {
          cJSON_DetachItemViaPointer(root, item);
          cJSON_Delete(item);
        }
      }
    }
  }

  remove_empty_sub_roots();
}

void device_json_config::remove_fields(unsigned _supla_int64_t fields) {
  for (auto it = field_map.cbegin(); it != field_map.cend(); ++it) {
    if (fields & it->first) {
      cJSON *root = get_root(false, it->first);
      if (root) {
        cJSON *item = cJSON_GetObjectItem(root, it->second.c_str());
        if (item) {
          cJSON_DetachItemViaPointer(root, item);
          cJSON_Delete(item);
        }
      }
    }
  }

  remove_empty_sub_roots();
}

void device_json_config::merge(supla_json_config *_dst) {
  device_json_config dst(_dst);

  map<cJSON *, map<unsigned _supla_int16_t, string>> map;

  for (auto it = field_map.cbegin(); it != field_map.cend(); ++it) {
    cJSON *root = get_root(false, it->first);
    if (root) {
      map[root][it->first] = it->second;
    }
  }

  for (auto rit = map.cbegin(); rit != map.cend(); ++rit) {
    for (auto fit = rit->second.cbegin(); fit != rit->second.cend(); ++fit) {
      supla_json_config::merge(rit->first, dst.get_root(true, fit->first),
                               rit->second, false);
    }
  }

  remove_empty_sub_roots();
}
