// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                        // IWYU pragma: associated
#include "1_Internal.hpp"                      // IWYU pragma: associated
#include "interface/ui/unitlist/UnitList.hpp"  // IWYU pragma: associated

#include <memory>
#include <string_view>
#include <thread>
#include <utility>

#include "internal/api/crypto/blockchain/Types.hpp"
#include "internal/identity/wot/claim/Types.hpp"
#include "internal/serialization/protobuf/verify/VerifyContacts.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/crypto/Blockchain.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Endpoints.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Storage.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/identity/wot/claim/Types.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/message/FrameSection.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/network/zeromq/message/Message.tpp"
#include "opentxs/network/zeromq/socket/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Pimpl.hpp"

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
    , blockchain_balance_cb_(zmq::ListenCallback::Factory(
          [this](const auto& in) { process_blockchain_balance(in); }))
    , blockchain_balance_(api_.Network().ZeroMQ().DealerSocket(
          blockchain_balance_cb_,
          zmq::socket::Direction::Connect))
    , listeners_{
          {api_.Endpoints().AccountUpdate().data(),
           new MessageProcessor<UnitList>(&UnitList::process_account)}}
{
    setup_listeners(listeners_);
    startup_ = std::make_unique<std::thread>(&UnitList::startup, this);

    OT_ASSERT(startup_)
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
    const auto body = message.Body();

    OT_ASSERT(2 < body.size());

    const auto accountID = api_.Factory().Identifier(body.at(1));

    OT_ASSERT(false == accountID->empty());

    process_account(accountID);
}

auto UnitList::process_account(const Identifier& id) noexcept -> void
{
    process_unit(api_.Storage().AccountUnit(id));
}

auto UnitList::process_blockchain_balance(const Message& message) noexcept
    -> void
{
    wait_for_startup();
    const auto body = message.Body();

    OT_ASSERT(3 < body.size());

    const auto& chainFrame = message.Body_at(1);

    try {
        process_unit(BlockchainToUnit(chainFrame.as<blockchain::Type>()));
    } catch (...) {
        LogError()(OT_PRETTY_CLASS())("Invalid chain").Flush();

        return;
    }
}

auto UnitList::process_unit(const UnitListRowID& id) noexcept -> void
{
    auto custom = CustomData{};
    add_item(id, proto::TranslateItemType(translate(UnitToClaim(id))), custom);
}

auto UnitList::setup_listeners(const ListenerDefinitions& definitions) noexcept
    -> void
{
    Widget::setup_listeners(definitions);
    const auto connected =
        blockchain_balance_->Start(api_.Endpoints().BlockchainBalance().data());

    OT_ASSERT(connected);
}

auto UnitList::startup() noexcept -> void
{
    const auto accounts = api_.Storage().AccountsByOwner(primary_id_);
    LogDetail()(OT_PRETTY_CLASS())("Loading ")(accounts.size())(" accounts.")
        .Flush();

    for (const auto& id : accounts) { process_account(id); }

    for (const auto& chain : blockchain::SupportedChains()) {
        if (0 < api_.Crypto()
                    .Blockchain()
                    .SubaccountList(primary_id_, chain)
                    .size()) {
            blockchain_balance_->Send([&] {
                using Job = api::crypto::blockchain::BalanceOracleJobs;
                auto work = network::zeromq::tagged_message(Job::registration);
                work.AddFrame(chain);

                return work;
            }());
        }
    }

    finish_startup();
}

UnitList::~UnitList()
{
    for (auto& it : listeners_) { delete it.second; }
}
}  // namespace opentxs::ui::implementation
