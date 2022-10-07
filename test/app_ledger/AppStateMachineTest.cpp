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

#include <gtest/gtest.h>
#include <INIReader.h>

#include "../../src/app_ledger/should_be_generated/domain/BusinessCode.h"
#include "../../src/app_ledger/v2/MemoryBackedAppStateMachine.h"
#include "../../src/app_util/AppInfo.h"
#include "../../src/infra/util/HttpCode.h"
#include "../../src/infra/util/PerfConfig.h"

namespace gringofts {
namespace ledger {

class LedgerAppStateMachineTest : public ::testing::Test {
 public:
  void SetUp() override {
    gringofts::Util::executeCmd("rm -rf ../test/app_ledger/data && mkdir ../test/app_ledger/data");
    mRocksDBBackedStateMachine = std::make_unique<v2::RocksDBBackedAppStateMachine>("../test/app_ledger/data/rocksdb",
                                                                                    "../test/app_ledger/data/rocksdb");
    auto configPath = "../test/app_ledger/config/app.ini";
    INIReader reader(configPath);
    if (reader.ParseError() < 0) {
      SPDLOG_WARN("Cannot load config file {}, exiting", configPath);
      assert(1);
    }
    app::AppInfo::init(reader);
    initMemoryPool();

    mInMemoryStateMachine = std::make_unique<v2::MemoryBackedAppStateMachine>(mFactory);

    mInMemoryStateMachine->swapState(mRocksDBBackedStateMachine.get());
    mRocksDBBackedStateMachine->recoverSelf();
  }

  void TearDown() override {
  }

 private:
  void initMemoryPool() {
    if (gringofts::PerfConfig::getInstance().getMemoryPoolType() == "monotonic") {
      mFactory = std::make_shared<gringofts::PMRContainerFactory>(
          "PMRFactory",
          std::make_unique<gringofts::MonotonicPMRMemoryPool>(
              "monotonicPool",
              gringofts::PerfConfig::getInstance().getMaxMemoryPoolSizeInMB()));
    } else {
      mFactory = std::make_shared<gringofts::PMRContainerFactory>(
          "PMRFactory",
          std::make_unique<gringofts::NewDeleteMemoryPool>(
              "newDeletePool"));
    }
  }

 protected:
  std::shared_ptr<CreateAccountCommand> createSampleCreateAccountCommand() {
    protos::CreateAccount::Request request;
    request.mutable_account()->set_version(1);
    request.mutable_account()->set_type(protos::AccountType::Asset);
    request.mutable_account()->set_nominal_code(1000);
    request.mutable_account()->set_name("creditCard");
    request.mutable_account()->set_desc("Shanghai credit card");
    request.mutable_account()->set_currency_code(156);  // CNY
    request.mutable_account()->mutable_balance()->set_version(1);
    request.mutable_account()->mutable_balance()->set_value(100);
    auto createdTimeInNanos = TimeUtil::currentTimeInNanos();
    auto command = std::make_shared<CreateAccountCommand>(createdTimeInNanos, request);
    command->setRequestHandle(nullptr);
    command->setCreatorId(app::AppInfo::subsystemId());
    command->setGroupId(app::AppInfo::groupId());
    command->setGroupVersion(app::AppInfo::groupVersion());

    return command;
  }

  std::shared_ptr<gringofts::PMRContainerFactory> mFactory;
  std::unique_ptr<v2::RocksDBBackedAppStateMachine> mRocksDBBackedStateMachine;
  std::unique_ptr<v2::MemoryBackedAppStateMachine> mInMemoryStateMachine;
};

TEST_F(LedgerAppStateMachineTest, CreateAccount) {
  /// 1. arrange
  auto command = createSampleCreateAccountCommand();
  std::vector<std::shared_ptr<gringofts::Event>> events;

  /// 2. act
  auto result = mInMemoryStateMachine->processCommandAndApply(*command, &events);
  for (const auto &event : events) {
    mRocksDBBackedStateMachine->applyEvent(*event);
  }

  /// 3. assert
  EXPECT_EQ(result.mCode, HttpCode::OK);
  EXPECT_EQ(events.size(), 1);
  const auto &event = *(events[0]);
  ASSERT_EQ(&typeid(AccountCreatedEvent), &typeid(event));
  const auto &accountCreatedEvent = dynamic_cast<const AccountCreatedEvent &>(event);
  const auto &accountFromCommandOpt = command->accountOpt();
  EXPECT_TRUE(accountFromCommandOpt);
  const auto &accountFromCommand = *accountFromCommandOpt;
  const auto &accountFromEvent = accountCreatedEvent.account();
  EXPECT_TRUE(accountFromCommand.isSame(accountFromEvent));

  /// flush to disk and re-init the state from persisted
  mRocksDBBackedStateMachine->flushToRocksDB();
  mRocksDBBackedStateMachine->recoverSelf();
  EXPECT_TRUE(mInMemoryStateMachine->hasSameState(*mRocksDBBackedStateMachine));
}

TEST_F(LedgerAppStateMachineTest, CreateExistingAccount) {
  /// 1. arrange
  auto command = createSampleCreateAccountCommand();
  std::vector<std::shared_ptr<gringofts::Event>> events;
  /// create an account
  mInMemoryStateMachine->processCommandAndApply(*command, &events);
  for (const auto &event : events) {
    mRocksDBBackedStateMachine->applyEvent(*event);
  }

  /// act
  /// try to create an account that already exists
  auto command1 = createSampleCreateAccountCommand();
  std::vector<std::shared_ptr<gringofts::Event>> events1;
  /// create an account
  auto result = mInMemoryStateMachine->processCommandAndApply(*command1, &events1);
  for (const auto &event : events1) {
    mRocksDBBackedStateMachine->applyEvent(*event);
  }

  /// 3. assert
  EXPECT_EQ(result.mCode, BusinessCode::ACCOUNT_ALREADY_EXISTS);
  EXPECT_TRUE(events1.empty());
}

}  // namespace ledger
}  // namespace gringofts
