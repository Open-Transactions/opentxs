// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <SpentTokenList.pb.h>
#include <StorageNotary.pb.h>
#include <cstdint>
#include <memory>

#include "internal/util/Mutex.hpp"
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

namespace identifier
{
class UnitDefinition;
}  // namespace identifier

namespace storage
{
class Driver;
class Tree;
}  // namespace storage
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::storage
{
class Notary final : public Node
{
public:
    using MintSeries = std::uint64_t;

    auto CheckSpent(
        const identifier::UnitDefinition& unit,
        const MintSeries series,
        const UnallocatedCString& key) const -> bool;

    auto MarkSpent(
        const identifier::UnitDefinition& unit,
        const MintSeries series,
        const UnallocatedCString& key) -> bool;

    Notary() = delete;
    Notary(const Notary&) = delete;
    Notary(Notary&&) = delete;
    auto operator=(const Notary&) -> Notary = delete;
    auto operator=(Notary&&) -> Notary = delete;

    ~Notary() final = default;

private:
    friend Tree;
    using SeriesMap = UnallocatedMap<MintSeries, UnallocatedCString>;
    using UnitMap = UnallocatedMap<UnallocatedCString, SeriesMap>;

    UnallocatedCString id_;

    mutable UnitMap mint_map_;

    auto create_list(
        const UnallocatedCString& unitID,
        const MintSeries series,
        std::shared_ptr<proto::SpentTokenList>& output) const
        -> UnallocatedCString;
    auto get_or_create_list(
        const Lock& lock,
        const UnallocatedCString& unitID,
        const MintSeries series) const -> proto::SpentTokenList;
    auto save(const Lock& lock) const -> bool final;
    auto serialize() const -> proto::StorageNotary;

    void init(const UnallocatedCString& hash) final;

    Notary(
        const api::Crypto& crypto,
        const api::session::Factory& factory,
        const Driver& storage,
        const UnallocatedCString& key,
        const UnallocatedCString& id);
};
}  // namespace opentxs::storage
