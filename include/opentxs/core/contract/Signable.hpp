// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include "opentxs/Types.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace proto
{
class Signature;
}  // namespace proto
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::contract
{
class OPENTXS_EXPORT Signable
{
public:
    using Signature = std::shared_ptr<proto::Signature>;

    virtual auto Alias() const noexcept -> UnallocatedCString = 0;
    virtual auto ID() const noexcept -> OTIdentifier = 0;
    virtual auto Name() const noexcept -> UnallocatedCString = 0;
    virtual auto Nym() const noexcept -> Nym_p = 0;
    virtual auto Terms() const noexcept -> const UnallocatedCString& = 0;
    virtual auto Serialize() const noexcept -> OTData = 0;
    virtual auto Validate() const noexcept -> bool = 0;
    virtual auto Version() const noexcept -> VersionNumber = 0;

    virtual auto SetAlias(const UnallocatedCString& alias) noexcept -> bool = 0;

    virtual ~Signable() = default;

protected:
    Signable() noexcept = default;

private:
#ifdef _WIN32
public:
#endif
    virtual auto clone() const noexcept -> Signable* = 0;
#ifdef _WIN32
private:
#endif

    Signable(const Signable&) = delete;
    Signable(Signable&&) = delete;
    auto operator=(const Signable&) -> Signable& = delete;
    auto operator=(Signable&&) -> Signable& = delete;
};
}  // namespace opentxs::contract
