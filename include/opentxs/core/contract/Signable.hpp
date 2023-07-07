// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <string_view>

#include "opentxs/Export.hpp"
#include "opentxs/identity/Types.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace identifier
{
class Generic;
class Notary;
class Nym;
class UnitDefinition;
}  // namespace identifier

class Writer;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::contract
{
template <typename IDType>
class Signable
{
public:
    using identifier_type = IDType;

    virtual auto Alias() const noexcept -> UnallocatedCString = 0;
    virtual auto Alias(alloc::Strategy alloc) const noexcept -> CString = 0;
    virtual auto ID() const noexcept -> const identifier_type& = 0;
    virtual auto Name() const noexcept -> std::string_view = 0;
    virtual auto Serialize(Writer&& out) const noexcept -> bool = 0;
    virtual auto Signer() const noexcept -> Nym_p = 0;
    virtual auto Terms() const noexcept -> std::string_view = 0;
    virtual auto Validate() const noexcept -> bool = 0;
    virtual auto Version() const noexcept -> VersionNumber = 0;

    virtual auto SetAlias(std::string_view alias) noexcept -> bool = 0;

    Signable(const Signable&) = delete;
    Signable(Signable&&) = delete;
    auto operator=(const Signable&) -> Signable& = delete;
    auto operator=(Signable&&) -> Signable& = delete;

    virtual ~Signable() = default;

protected:
    Signable() noexcept = default;
};
}  // namespace opentxs::contract
