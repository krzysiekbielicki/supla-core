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

#ifndef SRPC_ADAPTER_H_
#define SRPC_ADAPTER_H_

#include <functional>

#include "abstract_srpc_adapter.h"

class supla_srpc_adapter : public supla_abstract_srpc_adapter {
 public:
  explicit supla_srpc_adapter(void *srpc);
  virtual ~supla_srpc_adapter();

  template <typename TSuplaDataPack, typename TSuplaDataPackItem>
  static bool datapack_add(TSuplaDataPack *pack, int max_count,
                           std::function<void(TSuplaDataPackItem *)> fill) {
    if (pack->count < max_count) {
      fill(&pack->items[pack->count]);
      pack->items[pack->count].EOL = 0;
      pack->count++;
      return true;
    } else {
      pack->total_left++;
    }
    return false;
  }

  template <typename TSuplaDataPack>
  static void datapack_set_eol(TSuplaDataPack *pack) {
    if (pack && pack->count > 0) {
      if (pack->total_left == 0) pack->items[pack->count - 1].EOL = 1;
    }
  }

  virtual void lock(void);

  virtual void unlock(void);

  virtual char get_proto_version(void);

  virtual _supla_int_t sc_async_scene_pack_update(
      TSC_SuplaScenePack *scene_pack);  // ver. >= 18
  virtual _supla_int_t sc_async_scene_state_pack_update(
      TSC_SuplaSceneStatePack *scene_state_pack);  // ver. >= 18
};

#endif /* SRPC_ADAPTER_H_ */
