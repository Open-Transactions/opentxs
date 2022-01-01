// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/core/UnitType.hpp"

#pragma once

#include <map>
#include <memory>
#include <set>
#include <shared_mutex>
#include <string>
#include <tuple>

#include "Proto.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/contact/ClaimType.hpp"
#include "opentxs/core/Types.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "serialization/protobuf/StorageBip47Contexts.pb.h"
#include "storage/tree/Node.hpp"

namespace opentxs
{
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
}  // namespace opentxs

namespace opentxs::storage
{
class Bip47Channels final : public Node
{
public:
    using ChannelList = std::pmr::set<OTIdentifier>;

    auto Chain(const Identifier& channelID) const -> core::UnitType;
    auto ChannelsByChain(const core::UnitType chain) const -> ChannelList;
    auto Load(
        const Identifier& id,
        std::shared_ptr<proto::Bip47Channel>& output,
        const bool checking) const -> bool;

    auto Delete(const std::string& id) -> bool;
    auto Store(const Identifier& channelID, const proto::Bip47Channel& data)
        -> bool;

    ~Bip47Channels() final = default;

private:
    friend Nym;

    /** chain */
    using ChannelData = core::UnitType;
    /** channel id, channel data */
    using ChannelIndex = std::pmr::map<OTIdentifier, ChannelData>;
    using ChainIndex = std::pmr::map<core::UnitType, ChannelList>;

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
    auto init(const std::string& hash) -> void final;
    auto repair_indices() noexcept -> void;
    auto save(const Lock& lock) const -> bool final;
    auto serialize() const -> proto::StorageBip47Contexts;

    Bip47Channels(const Driver& storage, const std::string& hash);
    Bip47Channels() = delete;
    Bip47Channels(const Bip47Channels&) = delete;
    Bip47Channels(Bip47Channels&&) = delete;
    auto operator=(const Bip47Channels&) -> Bip47Channels = delete;
    auto operator=(Bip47Channels&&) -> Bip47Channels = delete;
};
}  // namespace opentxs::storage
