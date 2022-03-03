// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/core/UnitType.hpp"

#pragma once

#include <memory>
#include <shared_mutex>
#include <tuple>

#include "Proto.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/core/Types.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/identity/wot/claim/ClaimType.hpp"
#include "opentxs/util/Container.hpp"
#include "serialization/protobuf/StorageBip47Contexts.pb.h"
#include "util/storage/tree/Node.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace proto
{
class Bip47Channel;
class Bip47Direction;
}  // namespace proto

namespace storage
{
class Driver;
class Nym;
}  // namespace storage

class Identifier;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::storage
{
class Bip47Channels final : public Node
{
public:
    using ChannelList = UnallocatedSet<OTIdentifier>;

    auto Chain(const Identifier& channelID) const -> UnitType;
    auto ChannelsByChain(const UnitType chain) const -> ChannelList;
    auto Load(
        const Identifier& id,
        std::shared_ptr<proto::Bip47Channel>& output,
        const bool checking) const -> bool;

    auto Delete(const UnallocatedCString& id) -> bool;
    auto Store(const Identifier& channelID, const proto::Bip47Channel& data)
        -> bool;

    ~Bip47Channels() final = default;

private:
    friend Nym;

    /** chain */
    using ChannelData = UnitType;
    /** channel id, channel data */
    using ChannelIndex = UnallocatedMap<OTIdentifier, ChannelData>;
    using ChainIndex = UnallocatedMap<UnitType, ChannelList>;

    mutable std::shared_mutex index_lock_;
    ChannelIndex channel_data_;
    ChainIndex chain_index_;

    template <typename I, typename V>
    auto extract_set(const I& id, const V& index) const ->
        typename V::mapped_type;
    template <typename L>
    auto get_channel_data(const L& lock, const Identifier& id) const
        -> const ChannelData&;
    auto index(
        const eLock& lock,
        const Identifier& id,
        const proto::Bip47Channel& data) -> void;
    auto init(const UnallocatedCString& hash) -> void final;
    auto repair_indices() noexcept -> void;
    auto save(const Lock& lock) const -> bool final;
    auto serialize() const -> proto::StorageBip47Contexts;

    Bip47Channels(const Driver& storage, const UnallocatedCString& hash);
    Bip47Channels() = delete;
    Bip47Channels(const Bip47Channels&) = delete;
    Bip47Channels(Bip47Channels&&) = delete;
    auto operator=(const Bip47Channels&) -> Bip47Channels = delete;
    auto operator=(Bip47Channels&&) -> Bip47Channels = delete;
};
}  // namespace opentxs::storage
