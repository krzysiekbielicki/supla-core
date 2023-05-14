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

#include "devicechannel.h"

#include <assert.h>
#include <string.h>
#include <value/channel_openclosed_value.h>

#include <memory>

#include "channeljsonconfig/action_trigger_config.h"
#include "channeljsonconfig/impulse_counter_config.h"
#include "db/database.h"
#include "device.h"
#include "device/extended_value/channel_em_extended_value.h"
#include "device/extended_value/channel_ic_extended_value.h"
#include "device/value/channel_binary_sensor_value.h"
#include "device/value/channel_dgf_value.h"
#include "device/value/channel_em_value.h"
#include "device/value/channel_floating_point_sensor_value.h"
#include "device/value/channel_gate_value.h"
#include "device/value/channel_ic_value.h"
#include "device/value/channel_onoff_value.h"
#include "device/value/channel_rgbw_value.h"
#include "device/value/channel_rs_value.h"
#include "device/value/channel_temphum_value.h"
#include "device/value/channel_thermostat_value.h"
#include "device/value/channel_valve_value.h"
#include "lck.h"
#include "user/user.h"
#include "vbt/value_based_triggers.h"

using std::list;
using std::shared_ptr;

supla_device_channel::supla_device_channel(
    supla_device *device, int id, int number, int type, int func, int param1,
    int param2, int param3, int param4, const char *text_param1,
    const char *text_param2, const char *text_param3, bool hidden,
    unsigned int flags, const char value[SUPLA_CHANNELVALUE_SIZE],
    unsigned _supla_int_t validity_time_sec, const char *user_config,
    const char *properties)
    : id(id),
      number(number),
      type(type),
      param2(param2),
      param3(param3),
      param4(param4),
      hidden(hidden) {
  lck = lck_init();
  this->device = device;
  this->func = func;
  this->param1 = param1;
  this->text_param1 = text_param1 ? strndup(text_param1, 255) : nullptr;
  this->text_param2 = text_param2 ? strndup(text_param2, 255) : nullptr;
  this->text_param3 = text_param3 ? strndup(text_param3, 255) : nullptr;
  this->flags = flags;
  this->offline = flags & SUPLA_CHANNEL_FLAG_OFFLINE_DURING_REGISTRATION;
  this->extended_value = nullptr;
  this->value_valid_to.tv_sec = 0;
  this->value_valid_to.tv_usec = 0;
  this->json_config = nullptr;
  this->logger_data = nullptr;
  this->state = nullptr;

  if (validity_time_sec > 0) {
    gettimeofday(&value_valid_to, nullptr);
    value_valid_to.tv_sec += validity_time_sec;
  }

  memcpy(this->value, value, SUPLA_CHANNELVALUE_SIZE);

  json_config = new channel_json_config(nullptr);
  if (json_config) {
    json_config->set_properties(properties);
    json_config->set_user_config(user_config);
    if (type == SUPLA_CHANNELTYPE_ELECTRICITY_METER) {
      electricity_meter_config *config =
          new electricity_meter_config(json_config);
      this->flags |= config->get_channel_user_flags();
      delete config;
    }
  }

  if (type == SUPLA_CHANNELTYPE_ELECTRICITY_METER ||
      SUPLA_CHANNELTYPE_IMPULSE_COUNTER) {
    logger_data = new _logger_purpose_t();
  }

  voltage_analyzers.set_channel_id(id);
}

supla_device_channel::~supla_device_channel() {
  set_extended_value(nullptr);

  if (this->text_param1) {
    free(this->text_param1);
    this->text_param1 = nullptr;
  }

  if (this->text_param2) {
    free(this->text_param2);
    this->text_param2 = nullptr;
  }

  if (this->text_param3) {
    free(this->text_param3);
    this->text_param3 = nullptr;
  }

  if (json_config) {
    delete json_config;
  }

  if (logger_data) {
    if (type == SUPLA_CHANNELTYPE_ELECTRICITY_METER &&
        logger_data->extendedValue) {
      delete logger_data->extendedValue;
    }
    delete logger_data;
    logger_data = nullptr;
  }

  if (state) {
    delete state;
  }

  lck_free(lck);
}

// static
void supla_device_channel::get_defaults(int type, int func, int *param1,
                                        int *param2) {
  *param1 = 0;
  *param2 = 0;

  if (type == SUPLA_CHANNELTYPE_DIGIGLASS) {
    *param1 = 7;
    *param2 = 60;  // 1:00
  }
}

// static
int supla_device_channel::func_list_filter(int func_list, int type) {
  switch (type) {
    case SUPLA_CHANNELTYPE_ACTIONTRIGGER:
      func_list = 0;
      break;
  }

  return func_list;
}

void supla_device_channel::lock(void) { lck_lock(lck); }

void supla_device_channel::unlock(void) { lck_unlock(lck); }

int supla_device_channel::get_id(void) { return id; }

int supla_device_channel::get_number(void) { return number; }

int supla_device_channel::get_func(void) {
  lock();
  int result = func;
  unlock();
  return result;
}

supla_device *supla_device_channel::get_device() { return device; }

supla_user *supla_device_channel::get_user(void) {
  return device ? device->get_user() : nullptr;
}

int supla_device_channel::get_user_id(void) {
  supla_user *user = get_user();
  if (user) {
    return user->getUserID();
  }

  return 0;
}

void supla_device_channel::set_func(int func) {
  lock();
  this->func = func;
  unlock();
}

int supla_device_channel::get_type(void) { return type; }

int supla_device_channel::get_param1(void) {
  lock();
  int result = param1;
  unlock();
  return result;
}

int supla_device_channel::get_param2(void) { return param2; }

int supla_device_channel::get_param3(void) { return param3; }

int supla_device_channel::get_param4(void) { return param4; }

bool supla_device_channel::is_hidden(void) { return hidden; }

unsigned int supla_device_channel::get_flags() {
  lock();
  int result = flags;
  unlock();
  return result;
}

void supla_device_channel::add_flags(unsigned int flags) {
  unsigned int current_flags = get_flags();
  if ((current_flags | flags) != current_flags) {
    current_flags |= flags;
    lock();
    this->flags = current_flags;
    unlock();

    database *db = new database();

    if (db) {
      if (db->connect() == true) {
        db->update_channel_flags(get_id(), get_user_id(), current_flags);
      }
      delete db;
      db = nullptr;
    }
  }
}

const char *supla_device_channel::get_text_param1(void) { return text_param1; }

const char *supla_device_channel::get_text_param2(void) { return text_param2; }

const char *supla_device_channel::get_text_param3(void) { return text_param3; }

bool supla_device_channel::is_offline(void) {
  bool result = offline;
  lock();
  if (offline && (value_valid_to.tv_sec > 0 || value_valid_to.tv_usec)) {
    struct timeval now;
    gettimeofday(&now, nullptr);

    result =
        (now.tv_sec * 1000000 + now.tv_usec) -
                    (value_valid_to.tv_sec * 1000000 + value_valid_to.tv_usec) >
                0
            ? true
            : false;
  }
  unlock();

  return result;
}

bool supla_device_channel::set_offline(bool offline) {
  bool result = false;
  lock();
  if (this->offline != offline) {
    this->offline = offline;
    result = true;
  }
  unlock();
  return result;
}

void supla_device_channel::get_value(char value[SUPLA_CHANNELVALUE_SIZE]) {
  lock();
  memcpy(value, this->value, SUPLA_CHANNELVALUE_SIZE);
  unlock();
}

unsigned _supla_int_t supla_device_channel::get_value_validity_time_sec(void) {
  unsigned _supla_int_t result = 0;
  lock();
  if (value_valid_to.tv_sec > 0 || value_valid_to.tv_usec) {
    struct timeval now;
    gettimeofday(&now, nullptr);

    if (now.tv_sec < value_valid_to.tv_sec) {
      result = value_valid_to.tv_sec - now.tv_sec;
    }
  }
  unlock();
  return result;
}

supla_channel_extended_value *supla_device_channel::_get_extended_value(
    bool for_data_logger_purposes) {
  supla_channel_extended_value *result = nullptr;
  lock();
  int func = get_func();
  if (supla_channel_ic_extended_value::is_function_supported(func)) {
    supla_channel_ic_value *icval = nullptr;
    if (for_data_logger_purposes) {
      if (logger_data) {
        icval = new supla_channel_ic_value(logger_data->value);
      }
    } else {
      icval = get_value<supla_channel_ic_value>();
    }

    if (icval) {
      result = new supla_channel_ic_extended_value(
          func, icval->get_ic_value(), get_text_param1(), get_text_param2(),
          get_param2(), get_param3());

      delete icval;
    }
  } else if (extended_value &&
             supla_channel_em_extended_value::is_function_supported(func)) {
    if (for_data_logger_purposes) {
      if (logger_data) {
        result = new supla_channel_em_extended_value(
            logger_data->extendedValue, get_text_param1(), get_param2());
      }
    } else {
      result = new supla_channel_em_extended_value(
          extended_value, get_text_param1(), get_param2());
    }

  } else if (extended_value) {
    result = new supla_channel_extended_value(extended_value);
  }
  unlock();
  return result;
}

void supla_device_channel::get_double(double *value) {
  if (value == nullptr) return;

  char current_value[SUPLA_CHANNELVALUE_SIZE] = {};
  get_value(current_value);

  switch (type) {
    case SUPLA_CHANNELTYPE_SENSORNO:
    case SUPLA_CHANNELTYPE_SENSORNC:
      *value = current_value[0] == 1 ? 1 : 0;
      break;
    case SUPLA_CHANNELTYPE_THERMOMETER:
    case SUPLA_CHANNELTYPE_THERMOMETERDS18B20:
    case SUPLA_CHANNELTYPE_DISTANCESENSOR:
    case SUPLA_CHANNELTYPE_WINDSENSOR:
    case SUPLA_CHANNELTYPE_PRESSURESENSOR:
    case SUPLA_CHANNELTYPE_RAINSENSOR:
    case SUPLA_CHANNELTYPE_WEIGHTSENSOR:
      memcpy(value, current_value, sizeof(double));
      break;
    default:
      *value = 0;
  }
}

void supla_device_channel::get_char(char *value) {
  if (value == nullptr) return;

  char current_value[SUPLA_CHANNELVALUE_SIZE] = {};
  get_value(current_value);

  switch (get_func()) {
    case SUPLA_CHANNELFNC_THERMOSTAT:
    case SUPLA_CHANNELFNC_THERMOSTAT_HEATPOL_HOMEPLUS: {
      TThermostat_Value *tv = (TThermostat_Value *)current_value;
      *value = tv->IsOn;
    } break;
    default:
      *value = current_value[0];
      break;
  }
}

bool supla_device_channel::get_config(TSD_ChannelConfig *config,
                                      unsigned char config_type,
                                      unsigned _supla_int_t flags) {
  if (config_type != SUPLA_CONFIG_TYPE_DEFAULT || flags != 0) {
    return false;
  }

  memset(config, 0, sizeof(TSD_ChannelConfig));
  config->Func = get_func();
  config->ChannelNumber = get_number();

  switch (config->Func) {
    case SUPLA_CHANNELFNC_STAIRCASETIMER: {
      config->ConfigSize = sizeof(TSD_ChannelConfig_StaircaseTimer);
      TSD_ChannelConfig_StaircaseTimer *cfg =
          (TSD_ChannelConfig_StaircaseTimer *)config->Config;
      cfg->TimeMS = get_param1() * 100;
    } break;

    case SUPLA_CHANNELFNC_CONTROLLINGTHEROLLERSHUTTER:
    case SUPLA_CHANNELFNC_CONTROLLINGTHEROOFWINDOW: {
      config->ConfigSize = sizeof(TSD_ChannelConfig_Rollershutter);
      TSD_ChannelConfig_Rollershutter *cfg =
          (TSD_ChannelConfig_Rollershutter *)config->Config;
      cfg->OpeningTimeMS = get_param1() * 100;
      cfg->ClosingTimeMS = get_param3() * 100;
    } break;

    case SUPLA_CHANNELFNC_ACTIONTRIGGER: {
      config->ConfigSize = sizeof(TSD_ChannelConfig_ActionTrigger);
      TSD_ChannelConfig_ActionTrigger *cfg =
          (TSD_ChannelConfig_ActionTrigger *)config->Config;
      cfg->ActiveActions = 0;
      lock();
      action_trigger_config *at_config = new action_trigger_config(json_config);
      if (at_config) {
        cfg->ActiveActions = at_config->get_active_actions();
        delete at_config;
        at_config = nullptr;
      }
      unlock();
    } break;
  }

  return true;
}

void supla_device_channel::db_set_properties(channel_json_config *config) {
  database *db = new database();

  if (db) {
    if (db->connect() == true) {
      char *cfg_string = config->get_properties();

      db->update_channel_properties(get_id(), get_user_id(), cfg_string);

      if (cfg_string) {
        free(cfg_string);
        cfg_string = nullptr;
      }
    }
    delete db;
    db = nullptr;
  }
}

void supla_device_channel::db_set_params(int param1, int param2, int param3,
                                         int param4) {
  database *db = new database();

  if (db) {
    if (db->connect() == true) {
      db->update_channel_params(get_id(), get_user_id(), param1, param2, param3,
                                param4);
    }
    delete db;
    db = nullptr;
  }
}

void supla_device_channel::set_action_trigger_config(
    unsigned int capabilities, int related_channel_id,
    unsigned int disables_local_operation) {
  if (type != SUPLA_CHANNELTYPE_ACTIONTRIGGER) {
    return;
  }

  lock();
  action_trigger_config *at_config = new action_trigger_config(json_config);
  if (at_config) {
    if (!json_config) {
      json_config = at_config;
    }

    bool at_config_changed = at_config->set_capabilities(capabilities);

    if (at_config->set_caps_that_disables_local_operation(
            disables_local_operation)) {
      at_config_changed = true;
    }

    if (at_config_changed) {
      db_set_properties(at_config);
    }

    if (related_channel_id != get_param1()) {
      lock();
      param1 = related_channel_id;
      unlock();
      db_set_params(related_channel_id, param2, param3, param4);
    }

    if (at_config != json_config) {
      delete at_config;
      at_config = nullptr;
    }
  }
  unlock();
}

bool supla_device_channel::set_value(
    const char value[SUPLA_CHANNELVALUE_SIZE],
    const unsigned _supla_int_t *validity_time_sec, bool *offline) {
  lock();
  if (validity_time_sec == nullptr &&
      (value_valid_to.tv_usec || value_valid_to.tv_sec)) {
    struct timeval now;
    gettimeofday(&now, nullptr);

    if ((value_valid_to.tv_sec * 1000000 + value_valid_to.tv_usec) -
            (now.tv_sec * 1000000 + now.tv_usec) >
        0) {
      unlock();
      return false;
    }

    value_valid_to.tv_sec = 0;
    value_valid_to.tv_usec = 0;
  }

  supla_channel_value *old_value = _get_value();
  memcpy(this->value, value, SUPLA_CHANNELVALUE_SIZE);
  supla_channel_value *new_value = _get_value();

  new_value->apply_channel_properties(
      type, get_device()->get_connection()->get_protocol_version(), param1,
      param2, param3, param4, json_config, logger_data);

  new_value->get_raw_value(this->value);

  if (validity_time_sec) {
    gettimeofday(&value_valid_to, nullptr);
    value_valid_to.tv_sec += (*validity_time_sec);
  }

  bool significant_change = false;
  bool differ = new_value->is_differ(old_value, &significant_change);

  if (offline && set_offline(*offline)) {
    differ = true;
    significant_change = true;
  }

  unlock();

  if (differ || validity_time_sec) {
    database *db = new database();

    if (db->connect() == true) {
      char current_value[SUPLA_CHANNELVALUE_SIZE] = {};
      new_value->get_raw_value(current_value);
      db->update_channel_value(get_id(), get_user_id(), current_value,
                               validity_time_sec ? *validity_time_sec : 0);
    }

    delete db;
  }

  bool convertible2extended = is_convertible2extended();

  if (differ) {
    on_value_changed(old_value, new_value, significant_change,
                     convertible2extended);
  }

  if (new_value) {
    delete new_value;
  }

  if (old_value) {
    delete old_value;
  }

  return differ;
}

bool supla_device_channel::is_convertible2extended(void) {
  switch (get_func()) {
    case SUPLA_CHANNELFNC_IC_ELECTRICITY_METER:
    case SUPLA_CHANNELFNC_IC_GAS_METER:
    case SUPLA_CHANNELFNC_IC_WATER_METER:
    case SUPLA_CHANNELFNC_IC_HEAT_METER:
      return true;
  }

  return false;
}

void supla_device_channel::on_value_changed(supla_channel_value *old_value,
                                            supla_channel_value *new_value,
                                            bool significant_change,
                                            bool convertible2extended) {
  switch (get_func()) {
    case SUPLA_CHANNELFNC_OPENINGSENSOR_GARAGEDOOR:
    case SUPLA_CHANNELFNC_OPENINGSENSOR_GATE:
      if (get_param1()) {
        shared_ptr<supla_device> rel_device =
            get_device()->get_user()->get_device(device->get_user_id(), 0,
                                                 get_param1());
        if (rel_device) {
          rel_device->get_channels()->on_related_sensor_value_changed(
              get_param1(), get_id(), value[0] == 0);
        }
      }
      break;
  }

  get_device()->get_user()->on_channel_value_changed(
      supla_caller(ctDevice, get_device()->get_id()), get_device()->get_id(),
      get_id(), false, significant_change);

  if (convertible2extended) {
    get_device()->get_user()->on_channel_value_changed(
        supla_caller(ctDevice, get_device()->get_id()), get_device()->get_id(),
        get_id(), true);
  }

  get_device()
      ->get_user()
      ->get_value_based_triggers()
      ->on_channel_value_changed(supla_caller(ctDevice, get_device()->get_id()),
                                 get_id(), old_value, new_value);
}

void supla_device_channel::update_timer_state(void) {
  TTimerState_ExtendedValue *ts_ev = nullptr;

  lock();
  if (extended_value->type == EV_TYPE_TIMER_STATE_V1) {
    ts_ev = (TTimerState_ExtendedValue *)extended_value->value;
  } else if (extended_value->type == EV_TYPE_CHANNEL_AND_TIMER_STATE_V1) {
    TChannelAndTimerState_ExtendedValue *cats =
        (TChannelAndTimerState_ExtendedValue *)extended_value->value;
    ts_ev = &cats->Timer;
  }
  unlock();

  if (ts_ev == nullptr) {
    return;
  }

  if (ts_ev->SenderID) {
    supla_user *user = get_user();
    if (user) {
      shared_ptr<supla_client> client =
          user->get_clients()->get(ts_ev->SenderID);
      if (client != nullptr) {
        client->get_name(ts_ev->SenderName, SUPLA_SENDER_NAME_MAXSIZE);
        ts_ev->SenderNameSize =
            strnlen(ts_ev->SenderName, SUPLA_SENDER_NAME_MAXSIZE) + 1;
      } else {
        ts_ev->SenderID = 0;
        ts_ev->SenderName[0] = 0;
        ts_ev->SenderNameSize = 0;
      }
    }
  }

  if (ts_ev->RemainingTimeMs > 0) {
    struct timeval now;
    gettimeofday(&now, nullptr);

    unsigned _supla_int64_t time =
        now.tv_sec * (unsigned _supla_int64_t)1000000 + now.tv_usec;
    time /= 1000;
    time += ts_ev->RemainingTimeMs;
    time /= 1000;

    ts_ev->CountdownEndsAt = time;
  }
}

void supla_device_channel::update_extended_electricity_meter_value(void) {
  if (type != SUPLA_CHANNELTYPE_ELECTRICITY_METER) {
    return;
  }

  lock();

  TElectricityMeter_ExtendedValue em_ev_v1 = {};
  if (srpc_evtool_v1_extended2emextended(extended_value, &em_ev_v1)) {
    TElectricityMeter_ExtendedValue_V2 em_ev_v2 = {};

    if (!srpc_evtool_emev_v1to2(&em_ev_v1, &em_ev_v2) ||
        !srpc_evtool_v2_emextended2extended(&em_ev_v2, extended_value)) {
      delete extended_value;
      extended_value = nullptr;
    }
  }

  electricity_meter_config *config = new electricity_meter_config(json_config);

  if (config->update_available_counters(extended_value)) {
    db_set_properties(config);
  }

  if (logger_data && !logger_data->extendedValue) {
    logger_data->extendedValue = new TSuplaChannelExtendedValue();
  }

  if (logger_data && logger_data->extendedValue &&
      !config->should_be_added_to_history()) {
    memcpy(logger_data->extendedValue, extended_value,
           sizeof(TSuplaChannelExtendedValue));
  }

  config->add_initial_values(flags, extended_value);

  if (logger_data && logger_data->extendedValue &&
      config->should_be_added_to_history()) {
    memcpy(logger_data->extendedValue, extended_value,
           sizeof(TSuplaChannelExtendedValue));
  }

  voltage_analyzers.add_samples(get_flags(), config, extended_value);

  delete config;

  unlock();
}

void supla_device_channel::set_extended_value(TSuplaChannelExtendedValue *ev) {
  lock();
  if (ev == nullptr) {
    if (extended_value != nullptr) {
      delete extended_value;
      extended_value = nullptr;
    }
  } else {
    if (extended_value == nullptr) {
      extended_value = new TSuplaChannelExtendedValue;
    }
    memcpy(extended_value, ev, sizeof(TSuplaChannelExtendedValue));

    update_extended_electricity_meter_value();
    update_timer_state();
  }
  unlock();
}

void supla_device_channel::assign_rgbw_value(
    char value[SUPLA_CHANNELVALUE_SIZE], int color, char color_brightness,
    char brightness, char on_off) {
  int func = get_func();

  if (func == SUPLA_CHANNELFNC_DIMMER ||
      func == SUPLA_CHANNELFNC_DIMMERANDRGBLIGHTING) {
    if (brightness < 0 || brightness > 100) brightness = 0;

    value[0] = brightness;
  }

  if (func == SUPLA_CHANNELFNC_RGBLIGHTING ||
      func == SUPLA_CHANNELFNC_DIMMERANDRGBLIGHTING) {
    if (color_brightness < 0 || color_brightness > 100) color_brightness = 0;

    value[1] = color_brightness;
    value[2] = (char)((color & 0x000000FF));
    value[3] = (char)((color & 0x0000FF00) >> 8);
    value[4] = (char)((color & 0x00FF0000) >> 16);
  }

  value[5] = on_off;
}

bool supla_device_channel::is_value_writable(void) {
  switch (get_func()) {
    case SUPLA_CHANNELFNC_CONTROLLINGTHEGATEWAYLOCK:
    case SUPLA_CHANNELFNC_CONTROLLINGTHEGATE:
    case SUPLA_CHANNELFNC_CONTROLLINGTHEGARAGEDOOR:
    case SUPLA_CHANNELFNC_CONTROLLINGTHEDOORLOCK:
    case SUPLA_CHANNELFNC_CONTROLLINGTHEROLLERSHUTTER:
    case SUPLA_CHANNELFNC_CONTROLLINGTHEROOFWINDOW:
    case SUPLA_CHANNELFNC_POWERSWITCH:
    case SUPLA_CHANNELFNC_LIGHTSWITCH:
    case SUPLA_CHANNELFNC_DIMMER:
    case SUPLA_CHANNELFNC_RGBLIGHTING:
    case SUPLA_CHANNELFNC_DIMMERANDRGBLIGHTING:
    case SUPLA_CHANNELFNC_STAIRCASETIMER:
    case SUPLA_CHANNELFNC_VALVE_OPENCLOSE:
    case SUPLA_CHANNELFNC_VALVE_PERCENTAGE:
    case SUPLA_CHANNELFNC_DIGIGLASS_VERTICAL:
    case SUPLA_CHANNELFNC_DIGIGLASS_HORIZONTAL:
      return true;
  }

  return false;
}

bool supla_device_channel::is_char_value_writable(void) {
  switch (get_func()) {
    case SUPLA_CHANNELFNC_CONTROLLINGTHEGATEWAYLOCK:
    case SUPLA_CHANNELFNC_CONTROLLINGTHEGATE:
    case SUPLA_CHANNELFNC_CONTROLLINGTHEGARAGEDOOR:
    case SUPLA_CHANNELFNC_CONTROLLINGTHEDOORLOCK:
    case SUPLA_CHANNELFNC_CONTROLLINGTHEROLLERSHUTTER:
    case SUPLA_CHANNELFNC_CONTROLLINGTHEROOFWINDOW:
    case SUPLA_CHANNELFNC_POWERSWITCH:
    case SUPLA_CHANNELFNC_LIGHTSWITCH:
    case SUPLA_CHANNELFNC_STAIRCASETIMER:
    case SUPLA_CHANNELFNC_THERMOMETER:
    case SUPLA_CHANNELFNC_THERMOSTAT_HEATPOL_HOMEPLUS:
    case SUPLA_CHANNELFNC_VALVE_OPENCLOSE:
    case SUPLA_CHANNELFNC_VALVE_PERCENTAGE:
      return true;
  }

  return false;
}

bool supla_device_channel::is_rgbw_value_writable(void) {
  switch (get_func()) {
    case SUPLA_CHANNELFNC_DIMMER:
    case SUPLA_CHANNELFNC_RGBLIGHTING:
    case SUPLA_CHANNELFNC_DIMMERANDRGBLIGHTING:
      return 1;

      break;
  }

  return 0;
}

unsigned int supla_device_channel::get_value_duration(void) {
  switch (get_func()) {
    case SUPLA_CHANNELFNC_CONTROLLINGTHEGATEWAYLOCK:
    case SUPLA_CHANNELFNC_CONTROLLINGTHEGATE:
    case SUPLA_CHANNELFNC_CONTROLLINGTHEGARAGEDOOR:
    case SUPLA_CHANNELFNC_CONTROLLINGTHEDOORLOCK:
      return get_param1();

    case SUPLA_CHANNELFNC_STAIRCASETIMER:
      return get_param1() * 100;

    case SUPLA_CHANNELFNC_CONTROLLINGTHEROLLERSHUTTER:
    case SUPLA_CHANNELFNC_CONTROLLINGTHEROOFWINDOW: {
      unsigned int result = 0;

      result = (unsigned short)get_param1();
      result <<= 16;
      result |= (unsigned short)param3;

      return result;
    }
  }

  return 0;
}

list<int> supla_device_channel::related_channel(void) {
  list<int> result;

  // Only channels associated with NO / NC sensors can return more than one
  // channel!!!
  // See supla_user::get_channel_value

  switch (get_func()) {
    case SUPLA_CHANNELFNC_CONTROLLINGTHEGATEWAYLOCK:
    case SUPLA_CHANNELFNC_CONTROLLINGTHEGATE:
    case SUPLA_CHANNELFNC_CONTROLLINGTHEGARAGEDOOR:
    case SUPLA_CHANNELFNC_CONTROLLINGTHEDOORLOCK:
    case SUPLA_CHANNELFNC_CONTROLLINGTHEROLLERSHUTTER:
    case SUPLA_CHANNELFNC_CONTROLLINGTHEROOFWINDOW:

      // The order is important !!!
      // 1st Param2
      // 2nd Param3

      // Always add Param2
      result.push_back(param2);

      if (param3) {
        result.push_back(param3);
      }

      break;
    case SUPLA_CHANNELFNC_POWERSWITCH:
    case SUPLA_CHANNELFNC_LIGHTSWITCH: {
      int param1 = get_param1();
      if (param1) {
        result.push_back(param1);
      }
    } break;

    case SUPLA_CHANNELFNC_STAIRCASETIMER:

      if (param2) {
        result.push_back(param2);
      }

      break;
  }

  return result;
}

list<int> supla_device_channel::master_channel(void) {
  list<int> result;

  switch (get_func()) {
    case SUPLA_CHANNELFNC_OPENINGSENSOR_GATEWAY:
    case SUPLA_CHANNELFNC_OPENINGSENSOR_GATE:
    case SUPLA_CHANNELFNC_OPENINGSENSOR_GARAGEDOOR:
    case SUPLA_CHANNELFNC_OPENINGSENSOR_DOOR:
    case SUPLA_CHANNELFNC_OPENINGSENSOR_ROLLERSHUTTER:
    case SUPLA_CHANNELFNC_OPENINGSENSOR_ROOFWINDOW: {
      int param1 = get_param1();

      if (param1) {
        result.push_back(param1);
      }

      if (param2) {
        result.push_back(param2);
      }
    } break;
    case SUPLA_CHANNELFNC_ELECTRICITY_METER:
    case SUPLA_CHANNELFNC_IC_ELECTRICITY_METER:
    case SUPLA_CHANNELFNC_IC_GAS_METER:
    case SUPLA_CHANNELFNC_IC_WATER_METER:
    case SUPLA_CHANNELFNC_IC_HEAT_METER:

      if (param4) {
        result.push_back(param4);
      }

      break;
  }

  return result;
}

supla_channel_thermostat_measurement *
supla_device_channel::get_thermostat_measurement(void) {
  switch (get_type()) {
    case SUPLA_CHANNELTYPE_THERMOSTAT:
    case SUPLA_CHANNELTYPE_THERMOSTAT_HEATPOL_HOMEPLUS:
      switch (get_func()) {
        case SUPLA_CHANNELFNC_THERMOSTAT:
        case SUPLA_CHANNELFNC_THERMOSTAT_HEATPOL_HOMEPLUS: {
          char value[SUPLA_CHANNELVALUE_SIZE];
          get_value(value);
          TThermostat_Value *th_val = (TThermostat_Value *)value;

          return new supla_channel_thermostat_measurement(
              get_id(), th_val->IsOn > 0, th_val->MeasuredTemperature * 0.01,
              th_val->PresetTemperature * 0.01);
        }
      }
      break;
  }

  return nullptr;
}

channel_json_config *supla_device_channel::get_json_config(void) {
  channel_json_config *result = nullptr;

  lock();
  if (json_config) {
    result = new channel_json_config(json_config, true);
  }
  unlock();

  return result;
}

unsigned int supla_device_channel::get_value_validity_time_left_msec(void) {
  unsigned int result = 0;
  lock();
  if (value_valid_to.tv_sec || value_valid_to.tv_usec) {
    struct timeval now;
    gettimeofday(&now, nullptr);

    unsigned long long now_msec = (now.tv_sec * 1000 + now.tv_usec / 1000);
    unsigned long long valid_to_msec =
        (value_valid_to.tv_sec * 1000 + value_valid_to.tv_usec / 1000);

    if (now_msec < valid_to_msec) {
      result = valid_to_msec - now_msec;
    }
  }

  unlock();
  return result;
}

void supla_device_channel::set_state(TDSC_ChannelState *state) {
  lock();
  if (this->state == nullptr) {
    this->state = new TDSC_ChannelState();
  }

  *this->state = *state;
  unlock();
}

bool supla_device_channel::get_state(TDSC_ChannelState *state) {
  bool result = false;
  lock();
  if (this->state) {
    *state = *this->state;
    result = true;
  }
  unlock();

  return result;
}

bool supla_device_channel::get_voltage_analyzers_with_any_sample_over_threshold(
    supla_voltage_analyzers *voltage_analyzers, bool reset) {
  bool result = false;
  lock();
  if (this->voltage_analyzers.is_any_sample_over_threshold()) {
    *voltage_analyzers = this->voltage_analyzers;
    result = true;
  }

  if (reset) {
    this->voltage_analyzers.reset();
  }
  unlock();
  return result;
}

supla_channel_value *supla_device_channel::_get_value(void) {
  if (!get_func()) {
    return nullptr;
  }

  char value[SUPLA_CHANNELVALUE_SIZE] = {};
  get_value(value);
  int func = get_func();

  if (supla_channel_rs_value::is_function_supported(func)) {
    supla_channel_rs_value *rs_value = new supla_channel_rs_value(value);
    rs_value->update_sensor(get_device()->get_user(), get_param2());
    return rs_value;
  }

  if (supla_channel_gate_value::is_function_supported(func)) {
    supla_channel_gate_value *gate_value = new supla_channel_gate_value(value);

    gate_value->update_sensors(get_device()->get_user(), get_param2(),
                               get_param3());
    return gate_value;
  }

  if (supla_channel_onoff_value::is_function_supported(func)) {
    return new supla_channel_onoff_value(value);
  }

  if (supla_channel_rgbw_value::is_function_supported(func)) {
    return new supla_channel_rgbw_value(value);
  }

  if (supla_channel_valve_value::is_function_supported(func)) {
    return new supla_channel_valve_value(value);
  }

  if (supla_channel_temphum_value::is_function_supported(func)) {
    return new supla_channel_temphum_value(get_type(), get_func(), value);
  }

  if (supla_channel_binary_sensor_value::is_function_supported(func)) {
    return new supla_channel_binary_sensor_value(value);
  }

  if (supla_channel_floating_point_sensor_value::is_function_supported(func)) {
    return new supla_channel_floating_point_sensor_value(value);
  }

  if (supla_channel_openclosed_value::is_function_supported(func)) {
    return new supla_channel_openclosed_value(value);
  }

  if (supla_channel_thermostat_value::is_function_supported(func)) {
    return new supla_channel_thermostat_value(value);
  }

  if (supla_channel_dgf_value::is_function_supported(func)) {
    return new supla_channel_dgf_value(value);
  }

  if (supla_channel_ic_value::is_function_supported(func)) {
    return new supla_channel_ic_value(value);
  }

  if (supla_channel_em_value::is_function_supported(func)) {
    return new supla_channel_em_value(value);
  }

  return new supla_channel_value(value);
}
