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

#include <mqtt_publisher_datasource.h>

supla_mqtt_publisher_datasource::supla_mqtt_publisher_datasource(void)
    : supla_mqtt_client_datasource() {}

supla_mqtt_publisher_datasource::~supla_mqtt_publisher_datasource(void) {}

void *supla_mqtt_publisher_datasource::cursor_init(
    const _mqtt_ds_context_t *context) {
  return NULL;
}

bool supla_mqtt_publisher_datasource::_pop(const _mqtt_ds_context_t *context,
                                           void *cursor, char **topic_name,
                                           void **message, size_t *message_size,
                                           bool *eof) {
  return false;
}

void supla_mqtt_publisher_datasource::cursor_release(
    const _mqtt_ds_context_t *context, void *cursor) {}
