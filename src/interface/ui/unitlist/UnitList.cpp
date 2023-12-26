// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "interface/ui/unitlist/UnitList.hpp"  // IWYU pragma: associated

#include <memory>
#include <span>
#include <thread>
#include <utility>

#include "interface/ui/base/Widget.hpp"
#include "internal/api/crypto/blockchain/Types.hpp"
#include "internal/api/session/Storage.hpp"
#include "internal/network/zeromq/Context.hpp"
#include "internal/serialization/protobuf/verify/VerifyContacts.hpp"
#include "opentxs/api/Network.hpp"
#include "opentxs/api/crypto/Blockchain.hpp"
#include "opentxs/api/network/ZeroMQ.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Endpoints.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Storage.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/identifier/Account.hpp"
#include "opentxs/identity/wot/claim/Types.hpp"
#include "opentxs/identity/wot/claim/Types.internal.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/network/zeromq/message/Message.tpp"
#include "opentxs/network/zeromq/socket/Direction.hpp"  // IWYU pragma: keep
#include "opentxs/network/zeromq/socket/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"

namespace zmq = opentxs::network::zeromq;

namespace opentxs::factory
{
auto UnitListModel(
    const api::session::Client& api,
    const identifier::Nym& nymID,
    const SimpleCallback& cb) noexcept
    -> std::unique_ptr<ui::internal::UnitList>
{
    using ReturnType = ui::implementation::UnitList;

    return std::make_unique<ReturnType>(api, nymID, cb);
}
}  // namespace opentxs::factory

namespace opentxs::ui::implementation
{
UnitList::UnitList(
    const api::session::Client& api,
    const identifier::Nym& nymID,
    const SimpleCallback& cb) noexcept
    : UnitListList(api, nymID, cb, false)
    , api_(api)
    , blockchain_balance_cb_(zmq::ListenCallback::Factory(
          [this](const auto& in) { process_blockchain_balance(in); }))
    , blockchain_balance_(
          api.Network().ZeroMQ().Context().Internal().DealerSocket(
              blockchain_balance_cb_,
              zmq::socket::Direction::Connect,
              "UnitList"))
    , listeners_{
          {api.Endpoints().AccountUpdate().data(),
           new MessageProcessor<UnitList>(&UnitList::process_account)}}
{
    setup_listeners(api, listeners_);
    startup_ = std::make_unique<std::thread>(&UnitList::startup, this);

    assert_false(nullptr == startup_);
}

auto UnitList::construct_row(
    const UnitListRowID& id,
    const UnitListSortKey& index,
    CustomData& custom) const noexcept -> RowPointer
{
    return factory::UnitListItem(*this, api_, id, index, custom);
}

auto UnitList::process_account(const Message& message) noexcept -> void
{
    wait_for_startup();
    const auto body = message.Payload();

    assert_true(2 < body.size());

    const auto accountID = api_.Factory().AccountIDFromZMQ(body[1]);

    assert_false(accountID.empty());

    process_account(accountID);
}

auto UnitList::process_account(const identifier::Account& id) noexcept -> void
{
    process_unit(api_.Storage().Internal().AccountUnit(id));
}

auto UnitList::process_blockchain_balance(const Message& message) noexcept
    -> void
{
    wait_for_startup();
    const auto body = message.Payload();

    assert_true(3 < body.size());

    const auto& chainFrame = body[1];

    try {
        process_unit(blockchain_to_unit(chainFrame.as<blockchain::Type>()));
    } catch (...) {
        LogError()()("Invalid chain").Flush();

        return;
    }
}

auto UnitList::process_unit(const UnitListRowID& id) noexcept -> void
{
    auto custom = CustomData{};
    add_item(
        id,
        UnallocatedCString{
            proto::TranslateItemType(translate(UnitToClaim(id)))},
        custom);
}

auto UnitList::setup_listeners(
    const api::session::Client& api,
    const ListenerDefinitions& definitions) noexcept -> void
{
    Widget::setup_listeners(api, definitions);
    const auto connected =
        blockchain_balance_->Start(api.Endpoints().BlockchainBalance().data());

    assert_true(connected);
}

auto UnitList::startup() noexcept -> void
{
    const auto accounts =
        api_.Storage().Internal().AccountsByOwner(primary_id_);
    LogDetail()()("Loading ")(accounts.size())(" accounts.").Flush();

    for (const auto& id : accounts) { process_account(id); }

    for (const auto& chain : blockchain::supported_chains()) {
        if (0 < api_.Crypto()
                    .Blockchain()
                    .SubaccountList(primary_id_, chain)
                    .size()) {
            blockchain_balance_->Send([&] {
                using Job = api::crypto::blockchain::BalanceOracleJobs;
                auto work =
                    network::zeromq::tagged_message(Job::registration, true);
                work.AddFrame(chain);

                return work;
            }());
        }
    }

    finish_startup();
}

UnitList::~UnitList()
{
    for (const auto& it : listeners_) { delete it.second; }
}
}  // namespace opentxs::ui::implementation
