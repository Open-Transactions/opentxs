// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "internal/core/contract/peer/request/Base.hpp"
#include "internal/util/SharedPimpl.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace contract
{
namespace peer
{
namespace request
{
namespace internal
{
class Bailment;
}  // namespace internal
}  // namespace request
}  // namespace peer
}  // namespace contract

namespace identifier
{
class Notary;
class UnitDefinition;
}  // namespace identifier

using OTBailmentRequest =
    SharedPimpl<contract::peer::request::internal::Bailment>;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::contract::peer::request::internal
{
class Bailment : virtual public internal::Request
{
public:
    virtual auto ServerID() const -> const identifier::Notary& = 0;
    virtual auto UnitID() const -> const identifier::UnitDefinition& = 0;

    Bailment(const Bailment&) = delete;
    Bailment(Bailment&&) = delete;
    auto operator=(const Bailment&) -> Bailment& = delete;
    auto operator=(Bailment&&) -> Bailment& = delete;

    ~Bailment() override = default;

protected:
    Bailment() noexcept = default;

private:
    friend OTBailmentRequest;
};
}  // namespace opentxs::contract::peer::request::internal
