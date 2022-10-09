/************************************************************************
Copyright 2022 MySuperLedger
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at
    https://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
**************************************************************************/

#ifndef SRC_APP_LEDGER_SHOULD_BE_GENERATED_DOMAIN_COMMANDS_CREATEACCOUNTCOMMAND_H_
#define SRC_APP_LEDGER_SHOULD_BE_GENERATED_DOMAIN_COMMANDS_CREATEACCOUNTCOMMAND_H_

#include "../../../../infra/es/Command.h"
#include "../../../../infra/es/ProcessCommandStateMachine.h"
#include "../Account.h"

namespace gringofts {
namespace ledger {

class CreateAccountCommand : public Command {
 public:
  CreateAccountCommand(TimestampInNanos, const protos::CreateAccount::Request &origRequest);

  CreateAccountCommand(TimestampInNanos, const std::string &commandStr);

  std::string encodeToString() const override {
    return mOrigRequest.SerializeAsString();
  }

  void decodeFromString(std::string_view encodedString) override {
    mOrigRequest.ParseFromString(std::string(encodedString));
  }

  std::string verifyCommand() const override {
    return kVerifiedSuccess;
  }

  const std::optional<Account> &accountOpt() const {
    return mAccountOpt;
  }

 private:
  void tryInitAccount();
  void onPersisted(const std::string &message) override;
  void onPersistFailed(uint32_t code, const std::string &errorMessage, std::optional<uint64_t> reserved) override;

  protos::CreateAccount::Request mOrigRequest;
  std::optional<Account> mAccountOpt;
};

}  ///  namespace ledger
}  ///  namespace gringofts

#endif  // SRC_APP_LEDGER_SHOULD_BE_GENERATED_DOMAIN_COMMANDS_CREATEACCOUNTCOMMAND_H_
