// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Export.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace session
{
namespace internal
{
class Storage;
}  // namespace internal
}  // namespace session
}  // namespace api

namespace identifier
{
class Nym;
class Notary;
class UnitDefinition;
}  // namespace identifier
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::api::session
{
class OPENTXS_EXPORT Storage
{
public:
    virtual auto DefaultNym() const noexcept -> identifier::Nym = 0;
    virtual auto DefaultSeed() const noexcept -> opentxs::crypto::SeedID = 0;
    OPENTXS_NO_EXPORT virtual auto Internal() const noexcept
        -> const internal::Storage& = 0;
    virtual auto LocalNyms() const noexcept -> Set<identifier::Nym> = 0;
    virtual auto NymList() const noexcept -> ObjectList = 0;
    virtual auto ServerAlias(const identifier::Notary& id) const noexcept
        -> UnallocatedCString = 0;
    virtual auto ServerList() const noexcept -> ObjectList = 0;
    virtual auto SeedList() const noexcept -> ObjectList = 0;
    virtual auto UnitDefinitionAlias(const identifier::UnitDefinition& id)
        const noexcept -> UnallocatedCString = 0;
    virtual auto UnitDefinitionList() const noexcept -> ObjectList = 0;

    OPENTXS_NO_EXPORT virtual auto Internal() noexcept
        -> internal::Storage& = 0;

    Storage(const Storage&) = delete;
    Storage(Storage&&) = delete;
    auto operator=(const Storage&) -> Storage& = delete;
    auto operator=(Storage&&) -> Storage& = delete;

    OPENTXS_NO_EXPORT virtual ~Storage() = default;

protected:
    Storage() = default;
};
}  // namespace opentxs::api::session
