// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <SpentTokenList.pb.h>
#include <StorageNotary.pb.h>
#include <cstdint>
#include <functional>
#include <memory>
#include <string_view>

#include "internal/util/Mutex.hpp"
#include "opentxs/identifier/Notary.hpp"
#include "opentxs/identifier/UnitDefinition.hpp"
#include "opentxs/storage/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
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

namespace storage
{
namespace driver
{
class Plugin;
}  // namespace driver

namespace tree
{
class Trunk;
}  // namespace tree
}  // namespace storage
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::storage::tree
{
class Notary final : public Node
{
public:
    using MintSeries = std::uint64_t;

    auto CheckSpent(
        const identifier::UnitDefinition& unit,
        const MintSeries series,
        std::string_view key) const -> bool;

    auto MarkSpent(
        const identifier::UnitDefinition& unit,
        const MintSeries series,
        std::string_view key) -> bool;

    Notary() = delete;
    Notary(const Notary&) = delete;
    Notary(Notary&&) = delete;
    auto operator=(const Notary&) -> Notary = delete;
    auto operator=(Notary&&) -> Notary = delete;

    ~Notary() final = default;

private:
    friend Trunk;
    using SeriesMap = UnallocatedMap<MintSeries, Hash>;
    using UnitMap = UnallocatedMap<identifier::UnitDefinition, SeriesMap>;

    identifier::Notary id_;

    mutable UnitMap mint_map_;

    auto create_list(
        const identifier::UnitDefinition& unitID,
        const MintSeries series,
        std::shared_ptr<proto::SpentTokenList>& output) const -> Hash;
    auto dump(const Lock&, const Log&, Vector<Hash>& out) const noexcept
        -> bool final;
    auto get_or_create_list(
        const Lock& lock,
        const identifier::UnitDefinition& unitID,
        const MintSeries series) const -> proto::SpentTokenList;
    auto save(const Lock& lock) const -> bool final;
    auto serialize() const -> proto::StorageNotary;

    auto init(const Hash& hash) noexcept(false) -> void final;
    auto upgrade(const Lock& lock) noexcept -> bool final;

    Notary(
        const api::Crypto& crypto,
        const api::session::Factory& factory,
        const driver::Plugin& storage,
        const Hash& key,
        const identifier::Notary& id);
};
}  // namespace opentxs::storage::tree
