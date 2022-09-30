// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Export.hpp"
#include "opentxs/identity/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace identifier
{
class Generic;
}  // namespace identifier

namespace proto
{
class Signature;
}  // namespace proto

class ByteArray;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::contract
{
class OPENTXS_EXPORT Signable
{
public:
    using Signature = std::shared_ptr<proto::Signature>;

    virtual auto Alias() const noexcept -> UnallocatedCString = 0;
    virtual auto ID() const noexcept -> identifier::Generic = 0;
    virtual auto Name() const noexcept -> UnallocatedCString = 0;
    virtual auto Nym() const noexcept -> Nym_p = 0;
    virtual auto Terms() const noexcept -> const UnallocatedCString& = 0;
    virtual auto Serialize() const noexcept -> ByteArray = 0;
    virtual auto Validate() const noexcept -> bool = 0;
    virtual auto Version() const noexcept -> VersionNumber = 0;

    virtual auto SetAlias(const UnallocatedCString& alias) noexcept -> bool = 0;

    Signable(const Signable&) = delete;
    Signable(Signable&&) = delete;
    auto operator=(const Signable&) -> Signable& = delete;
    auto operator=(Signable&&) -> Signable& = delete;

    virtual ~Signable() = default;

protected:
    Signable() noexcept = default;

private:
#ifdef _WIN32
public:
#endif
    // tmp solution this function is toremove once all classes which inherites
    // by it will be rewritten
    virtual auto clone() const noexcept -> Signable* { return nullptr; }
};
}  // namespace opentxs::contract
