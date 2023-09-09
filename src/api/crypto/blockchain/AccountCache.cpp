// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "api/crypto/blockchain/AccountCache.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <iterator>
#include <span>
#include <utility>

#include "internal/api/session/Storage.hpp"
#include "internal/blockchain/crypto/Account.hpp"
#include "internal/network/zeromq/Context.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/session/Endpoints.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/api/session/Storage.hpp"
#include "opentxs/api/session/Wallet.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/crypto/SubaccountType.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/network/zeromq/message/Message.tpp"
#include "opentxs/network/zeromq/socket/SocketType.hpp"  // IWYU pragma: keep
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/WorkType.hpp"

namespace opentxs::api::crypto::blockchain
{
AccountCache::AccountCache(const api::Session& api) noexcept
    : api_(api)
    , populated_(false)
    , index_()
    , params_()
    , socket_([&] {
        using enum opentxs::network::zeromq::socket::Type;
        auto out = api_.Network().ZeroMQ().Internal().RawSocket(Publish);
        const auto rc =
            out.Bind(api_.Endpoints().BlockchainAccountCreated().data());

        OT_ASSERT(rc);

        return out;
    }())
{
}

auto AccountCache::build_account_map(
    const opentxs::blockchain::Type chain,
    NymIndex& map) noexcept -> void
{
    const auto nyms = api_.Wallet().LocalNyms();
    auto load_nym = [&, this](const auto& nym) mutable {
        this->load_nym(chain, nym, map);
    };
    std::for_each(std::begin(nyms), std::end(nyms), load_nym);
}

auto AccountCache::List(
    const identifier::Nym& nymID,
    const opentxs::blockchain::Type chain) const noexcept
    -> UnallocatedSet<identifier::Account>
{
    OT_ASSERT(populated_);

    if (const auto i = index_.find(chain); index_.end() != i) {
        const auto& index = i->second;

        if (const auto j = index.find(nymID); index.end() != j) {

            return j->second;
        } else {

            return {};
        }
    } else {

        return {};
    }
}

auto AccountCache::load_nym(
    const opentxs::blockchain::Type chain,
    const identifier::Nym& nym,
    NymIndex& output) noexcept -> void
{
    const auto parent =
        opentxs::blockchain::crypto::internal::Account::GetID(api_, nym, chain);
    auto populate = [&, this](const auto type, auto& subaccount) {
        params_.try_emplace(subaccount, type, chain, nym, parent);
        auto& set = output[nym];
        set.emplace(std::move(subaccount));
    };
    using enum opentxs::blockchain::crypto::SubaccountType;
    auto populate_hd = [&](auto& id) { populate(HD, id); };
    auto populate_pc = [&](auto& id) { populate(PaymentCode, id); };
    auto hd = api_.Storage().Internal().BlockchainAccountList(
        nym, blockchain_to_unit(chain));
    auto pc = api_.Storage().Internal().Bip47ChannelsByChain(
        nym, blockchain_to_unit(chain));
    std::for_each(std::begin(hd), std::end(hd), populate_hd);
    std::for_each(std::begin(pc), std::end(pc), populate_pc);
}

auto AccountCache::Owner(const identifier::Account& id) const noexcept
    -> const identifier::Nym&
{
    OT_ASSERT(populated_);

    if (const auto i = params_.find(id); params_.end() != i) {

        return i->second.owner_;
    } else {
        static const auto blank = identifier::Nym{};

        return blank;
    }
}

auto AccountCache::Populate() noexcept -> void
{
    for (const auto& chain : opentxs::blockchain::supported_chains()) {
        auto& map = index_[chain];
        build_account_map(chain, map);
    }

    populated_ = true;
}

auto AccountCache::RegisterSubaccount(
    const opentxs::blockchain::crypto::SubaccountType type,
    const opentxs::blockchain::Type chain,
    const identifier::Nym& owner,
    const identifier::Account& account,
    const identifier::Account& subaccount) noexcept -> bool
{
    if (owner.empty()) {
        LogError()(OT_PRETTY_CLASS())("invalid owner").Flush();

        return false;
    }

    if (account.empty()) {
        LogError()(OT_PRETTY_CLASS())("invalid parent account").Flush();

        return false;
    }

    if (subaccount.empty()) {
        LogError()(OT_PRETTY_CLASS())("invalid subaccount").Flush();

        return false;
    }

    if (params_.try_emplace(subaccount, type, chain, owner, account).second) {
        index_[chain][owner].emplace(subaccount);
        socket_.SendDeferred(
            [&] {
                auto work = opentxs::network::zeromq::tagged_message(
                    WorkType::BlockchainAccountCreated, true);
                work.AddFrame(chain);
                work.AddFrame(owner);
                work.AddFrame(type);
                subaccount.Serialize(work);

                return work;
            }(),
            __FILE__,
            __LINE__);

        return true;
    } else {

        return false;
    }
}

auto AccountCache::Type(const identifier::Account& id) const noexcept
    -> opentxs::blockchain::crypto::SubaccountType
{
    OT_ASSERT(populated_);

    if (const auto i = params_.find(id); params_.end() != i) {

        return i->second.type_;
    } else {

        return opentxs::blockchain::crypto::SubaccountType::Error;
    }
}
}  // namespace opentxs::api::crypto::blockchain
