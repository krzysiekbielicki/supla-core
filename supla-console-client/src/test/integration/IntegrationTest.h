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

#ifndef INTEGRATIONTEST_H_
#define INTEGRATIONTEST_H_

#include "gtest/gtest.h"
#include "supla-client.h"

namespace testing {

class IntegrationTest : public Test {
 private:
  void *sclient;
  static char *sqlDir;
  static char *dbName;
  static char *dbHost;
  static char *dbUser;
  static char defaultDbName[];
  static char defaultDbHost[];
  static char defaultDbUser[];
  bool iterationCancelled;

  void clientInit();
  void clientFree();

 protected:
  void iterateUntilTimeout(unsigned int timeoutMS);
  void iterateUntilDefaultTimeout();
  void cancelIteration(void);
  void fillArrayWithOrdinalNumbers(char *arr, int arr_size, char start);
  virtual void beforeClientInit(TSuplaClientCfg *scc);
  void runSqlScript(const char *script);
  void initTestDatabase();

 public:
  static void Init(int argc, char **argv);
  IntegrationTest();
  virtual ~IntegrationTest();
  virtual void onConnected();
  virtual void onDisconnected();
  virtual void onConnectionError(int code);
  virtual void onRegistered(TSC_SuplaRegisterClientResult_B *result);
  virtual void onRegistrationError(int code);
};

} /* namespace testing */

#endif /* INTEGRATIONTEST_H_ */
