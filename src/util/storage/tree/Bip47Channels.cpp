// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "util/storage/tree/Bip47Channels.hpp"  // IWYU pragma: associated

#include <Bip47Channel.pb.h>
#include <BlockchainAccountData.pb.h>
#include <BlockchainDeterministicAccountData.pb.h>
#include <StorageBip47ChannelList.pb.h>
#include <StorageBip47Contexts.pb.h>
#include <StorageItemHash.pb.h>
#include <mutex>
#include <stdexcept>
#include <tuple>
#include <utility>

#include "internal/identity/wot/claim/Types.hpp"
#include "internal/serialization/protobuf/Check.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "internal/serialization/protobuf/verify/Bip47Channel.hpp"
#include "internal/serialization/protobuf/verify/StorageBip47Contexts.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Types.hpp"
#include "opentxs/core/UnitType.hpp"  // IWYU pragma: keep
#include "opentxs/core/identifier/Account.hpp"
#include "opentxs/identity/wot/claim/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/storage/Driver.hpp"
#include "util/storage/Plugin.hpp"
#include "util/storage/tree/Node.hpp"

#define CHANNEL_VERSION 1
#define CHANNEL_INDEX_VERSION 1

namespace opentxs::storage
{
Bip47Channels::Bip47Channels(
    const api::Crypto& crypto,
    const api::session::Factory& factory,
    const Driver& storage,
    const UnallocatedCString& hash)
    : Node(crypto, factory, storage, hash)
    , index_lock_()
    , channel_data_()
    , chain_index_()
{
    if (check_hash(hash)) {
        init(hash);
    } else {
        blank(CHANNEL_VERSION);
    }
}

auto Bip47Channels::Chain(const identifier::Account& channelID) const
    -> UnitType
{
    auto lock = sLock{index_lock_};

    return get_channel_data(lock, channelID);
}

auto Bip47Channels::ChannelsByChain(const UnitType chain) const
    -> Bip47Channels::ChannelList
{
    return extract_set(chain, chain_index_);
}

auto Bip47Channels::Delete(const UnallocatedCString& id) -> bool
{
    return delete_item(id);
}

template <typename I, typename V>
auto Bip47Channels::extract_set(const I& id, const V& index) const ->
    typename V::mapped_type
{
    auto lock = sLock{index_lock_};

    try {
        return index.at(id);

    } catch (...) {

        return {};
    }
}

template <typename L>
auto Bip47Channels::get_channel_data(
    const L& lock,
    const identifier::Account& id) const -> const Bip47Channels::ChannelData&
{
    try {

        return channel_data_.at(id);
    } catch (const std::out_of_range&) {
        static auto blank = ChannelData{UnitType::Error};

        return blank;
    }
}

auto Bip47Channels::index(
    const eLock& lock,
    const identifier::Account& id,
    const proto::Bip47Channel& data) -> void
{
    const auto& common = data.deterministic().common();
    auto& chain = channel_data_[id];
    chain = ClaimToUnit(translate(common.chain()));
    chain_index_[chain].emplace(id);
}

auto Bip47Channels::init(const UnallocatedCString& hash) -> void
{
    auto proto = std::shared_ptr<proto::StorageBip47Contexts>{};
    driver_.LoadProto(hash, proto);

    if (!proto) {
        LogError()(OT_PRETTY_CLASS())(
            "Failed to load bip47 channel index file.")
            .Flush();
        OT_FAIL;
    }

    init_version(CHANNEL_VERSION, *proto);

    for (const auto& it : proto->context()) {
        item_map_.emplace(
            it.itemid(), Metadata{it.hash(), it.alias(), 0, false});
    }

    if (proto->context().size() != proto->index().size()) {
        repair_indices();
    } else {
        for (const auto& index : proto->index()) {
            auto id = factory_.AccountIDFromBase58(index.channelid());
            auto& chain = channel_data_[id];
            chain = ClaimToUnit(translate(index.chain()));
            chain_index_[chain].emplace(std::move(id));
        }
    }
}

auto Bip47Channels::Load(
    const identifier::Account& id,
    std::shared_ptr<proto::Bip47Channel>& output,
    const bool checking) const -> bool
{
    UnallocatedCString alias{""};

    return load_proto<proto::Bip47Channel>(
        id.asBase58(crypto_), output, alias, checking);
}

auto Bip47Channels::repair_indices() noexcept -> void
{
    {
        auto lock = eLock{index_lock_};

        for (const auto& [strid, alias] : List()) {
            const auto id = factory_.AccountIDFromBase58(strid);
            auto data = std::shared_ptr<proto::Bip47Channel>{};
            const auto loaded = Load(id, data, false);

            OT_ASSERT(loaded);
            OT_ASSERT(data);

            index(lock, id, *data);
        }
    }

    auto lock = Lock{write_lock_};
    const auto saved = save(lock);

    OT_ASSERT(saved);
}

auto Bip47Channels::save(const std::unique_lock<std::mutex>& lock) const -> bool
{
    if (!verify_write_lock(lock)) {
        LogError()(OT_PRETTY_CLASS())("Lock failure.").Flush();
        OT_FAIL;
    }

    auto serialized = serialize();

    if (!proto::Validate(serialized, VERBOSE)) { return false; }

    return driver_.StoreProto(serialized, root_);
}

auto Bip47Channels::serialize() const -> proto::StorageBip47Contexts
{
    auto serialized = proto::StorageBip47Contexts{};
    serialized.set_version(version_);

    for (const auto& item : item_map_) {
        const bool goodID = !item.first.empty();
        const bool goodHash = check_hash(std::get<0>(item.second));
        const bool good = goodID && goodHash;

        if (good) {
            serialize_index(
                version_, item.first, item.second, *serialized.add_context());
        }
    }

    auto lock = sLock{index_lock_};

    for (const auto& [id, data] : channel_data_) {
        const auto& chain = data;
        auto& index = *serialized.add_index();
        index.set_version(CHANNEL_INDEX_VERSION);
        index.set_channelid(id.asBase58(crypto_));
        index.set_chain(translate(UnitToClaim(chain)));
    }

    return serialized;
}

auto Bip47Channels::Store(
    const identifier::Account& id,
    const proto::Bip47Channel& data) -> bool
{
    {
        auto lock = eLock{index_lock_};
        index(lock, id, data);
    }

    return store_proto(data, id.asBase58(crypto_), "");
}
}  // namespace opentxs::storage
