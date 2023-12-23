// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <StorageBip47Contexts.pb.h>
#include <functional>
#include <memory>
#include <shared_mutex>

#include "internal/util/Mutex.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/identifier/Account.hpp"
#include "opentxs/storage/Types.hpp"
#include "opentxs/storage/Types.internal.hpp"
#include "opentxs/util/Container.hpp"
#include "util/storage/tree/Node.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace session
{
class Factory;
}  // namespace session

class Crypto;
}  // namespace api

namespace proto
{
class Bip47Channel;
}  // namespace proto

namespace storage
{
namespace driver
{
class Plugin;
}  // namespace driver

namespace tree
{
class Nym;
}  // namespace tree
}  // namespace storage
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::storage::tree
{
class Bip47Channels final : public Node
{
public:
    using ChannelList = UnallocatedSet<identifier::Account>;

    auto Chain(const identifier::Account& channelID) const -> UnitType;
    auto ChannelsByChain(const UnitType chain) const -> ChannelList;
    auto Load(
        const identifier::Account& id,
        std::shared_ptr<proto::Bip47Channel>& output,
        ErrorReporting checking) const -> bool;

    auto Store(
        const identifier::Account& channelID,
        const proto::Bip47Channel& data) -> bool;

    Bip47Channels() = delete;
    Bip47Channels(const Bip47Channels&) = delete;
    Bip47Channels(Bip47Channels&&) = delete;
    auto operator=(const Bip47Channels&) -> Bip47Channels = delete;
    auto operator=(Bip47Channels&&) -> Bip47Channels = delete;

    ~Bip47Channels() final = default;

private:
    friend Nym;

    /** chain */
    using ChannelData = UnitType;
    /** channel id, channel data */
    using ChannelIndex = UnallocatedMap<identifier::Account, ChannelData>;
    using ChainIndex = UnallocatedMap<UnitType, ChannelList>;

    mutable std::shared_mutex index_lock_;
    ChannelIndex channel_data_;
    ChainIndex chain_index_;
    bool repair_indices_;

    template <typename I, typename V>
    auto extract_set(const I& id, const V& index) const ->
        typename V::mapped_type;
    template <typename L>
    auto get_channel_data(const L& lock, const identifier::Account& id) const
        -> const ChannelData&;
    auto index(
        const eLock& lock,
        const identifier::Account& id,
        const proto::Bip47Channel& data) -> void;
    auto init(const Hash& hash) -> void final;
    auto repair_indices() noexcept -> void;
    auto save(const Lock& lock) const -> bool final;
    auto serialize() const -> proto::StorageBip47Contexts;
    auto upgrade(const Lock& lock) noexcept -> bool final;

    Bip47Channels(
        const api::Crypto& crypto,
        const api::session::Factory& factory,
        const driver::Plugin& storage,
        const Hash& hash);
};
}  // namespace opentxs::storage::tree
