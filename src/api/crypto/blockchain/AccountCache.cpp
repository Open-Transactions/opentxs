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
AccountCache::AccountCache(
    const api::Session& api,
    alloc::Default alloc) noexcept
    : HasUpstreamAllocator(alloc.resource())
    , AllocatesChildren(parent_resource())
    , api_(api)
    , populated_(false)
    , nym_index_(child_alloc_)
    , account_index_(child_alloc_)
    , account_params_(child_alloc_)
    , subaccount_params_(child_alloc_)
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

auto AccountCache::AccountData(const identifier::Account& id) const noexcept
    -> crypto::Blockchain::AccountData
{
    if (const auto i = account_params_.find(id); account_params_.end() != i) {

        return i->second;
    } else {

        return {};
    }
}

auto AccountCache::AccountList(const identifier::Nym& nymID) const noexcept
    -> UnallocatedSet<identifier::Account>
{
    auto out = UnallocatedSet<identifier::Account>{};
    const auto get_matching_accounts = [&](const auto& item) {
        const auto& [chain, map] = item;

        if (const auto i = map.find(nymID); map.end() != i) {
            const auto& [id, subaccounts] = i->second;
            out.emplace(id);
        }
    };
    std::for_each(nym_index_.begin(), nym_index_.end(), get_matching_accounts);

    return out;
}

auto AccountCache::AccountList(const opentxs::blockchain::Type chain)
    const noexcept -> UnallocatedSet<identifier::Account>
{
    auto out = UnallocatedSet<identifier::Account>{};
    const auto get_account = [&](const auto& item) {
        const auto& [nymID, index] = item;
        const auto& [id, subaccounts] = index;
        out.emplace(id);
    };

    if (const auto i = nym_index_.find(chain); nym_index_.end() != i) {
        const auto& map = i->second;
        std::for_each(map.begin(), map.end(), get_account);
    }

    return out;
}

auto AccountCache::AccountList() const noexcept
    -> UnallocatedSet<identifier::Account>
{
    auto out = UnallocatedSet<identifier::Account>{};
    const auto get_account = [&](const auto& item) {
        const auto& [nymID, index] = item;
        const auto& [id, subaccounts] = index;
        out.emplace(id);
    };
    const auto get_accounts = [&](const auto& item) {
        const auto& [chain, map] = item;
        std::for_each(map.begin(), map.end(), get_account);
    };
    std::for_each(nym_index_.begin(), nym_index_.end(), get_accounts);

    return out;
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

auto AccountCache::SubaccountList(
    const identifier::Nym& nymID,
    const opentxs::blockchain::Type chain) const noexcept
    -> UnallocatedSet<identifier::Account>
{
    OT_ASSERT(populated_);

    if (const auto i = nym_index_.find(chain); nym_index_.end() != i) {
        const auto& index = i->second;

        if (const auto j = index.find(nymID); index.end() != j) {
            const auto& in = j->second.second;
            auto out = UnallocatedSet<identifier::Account>{};
            std::copy(in.begin(), in.end(), std::inserter(out, out.end()));

            return out;
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
        account_index_[parent].emplace(subaccount);
        account_params_.try_emplace(parent, chain, nym);
        subaccount_params_.try_emplace(subaccount, type, chain, nym, parent);
        auto& index = output[nym];

        if (index.first.empty()) { index.first = parent; }

        index.second.emplace(std::move(subaccount));
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

    const auto& account = account_params_;
    const auto& subaccount = subaccount_params_;

    if (const auto i = subaccount.find(id); subaccount.end() != i) {

        return i->second.owner_;
    } else if (const auto j = account.find(id); account.end() != j) {

        return j->second.owner_;
    } else {
        static const auto blank = identifier::Nym{};

        return blank;
    }
}

auto AccountCache::Populate() noexcept -> void
{
    for (const auto& chain : opentxs::blockchain::supported_chains()) {
        auto& map = nym_index_[chain];
        build_account_map(chain, map);
    }

    populated_ = true;
}

auto AccountCache::RegisterAccount(
    const opentxs::blockchain::Type chain,
    const identifier::Nym& owner,
    const identifier::Account& account) noexcept -> bool
{
    if (owner.empty()) {
        LogError()(OT_PRETTY_CLASS())("invalid owner").Flush();

        return false;
    }

    const auto expected = opentxs::blockchain::crypto::internal::Account::GetID(
        api_, owner, chain);

    if (account != expected) {
        LogError()(OT_PRETTY_CLASS())("invalid account").Flush();

        return false;
    }

    account_params_.try_emplace(account, chain, owner);
    nym_index_[chain][owner].first = account;
    account_index_[account];

    return true;
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

    if (subaccount_params_.try_emplace(subaccount, type, chain, owner, account)
            .second) {
        account_params_.try_emplace(account, chain, owner);
        auto& index = nym_index_[chain][owner];

        if (index.first.empty()) {
            index.first = account;
        } else if (index.first != account) {
            LogError()(OT_PRETTY_CLASS())(
                "aubaccount parent does not match existing recorded value")
                .Flush();

            return false;
        }

        index.second.emplace(subaccount);
        account_index_[account].emplace(subaccount);
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

auto AccountCache::SubaccountType(const identifier::Account& id) const noexcept
    -> opentxs::blockchain::crypto::SubaccountType
{
    OT_ASSERT(populated_);

    if (const auto i = subaccount_params_.find(id);
        subaccount_params_.end() != i) {

        return i->second.type_;
    } else {

        return opentxs::blockchain::crypto::SubaccountType::Error;
    }
}
}  // namespace opentxs::api::crypto::blockchain
