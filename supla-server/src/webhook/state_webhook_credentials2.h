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

#ifndef STATE_WEBHOOK_CREDENTIALS2_H_
#define STATE_WEBHOOK_CREDENTIALS2_H_

#include <list>

#include "http/oauth_credentials.h"

class supla_state_webhook_credentials2 : public supla_http_oauth_credentials {
 private:
  std::string url;
  std::list<int> function_ids;

 public:
  supla_state_webhook_credentials2(void);
  explicit supla_state_webhook_credentials2(supla_user *user);
  ~supla_state_webhook_credentials2(void);

  virtual std::string get_url(void);
  std::list<int> get_function_ids(void);
  virtual void update(const std::string access_token,
                      const std::string refresh_token, int expires_in);
  virtual void remove(void);
  void load(void);
};

#endif /* STATE_WEBHOOK_CREDENTIALS2_H_ */
