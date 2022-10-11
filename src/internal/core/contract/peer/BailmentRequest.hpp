// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "internal/core/contract/peer/PeerRequest.hpp"
#include "internal/util/SharedPimpl.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace identifier
{
class Notary;
class UnitDefinition;
}  // namespace identifier
namespace contract
{
namespace peer
{
namespace request
{
class Bailment;
}  // namespace request
}  // namespace peer
}  // namespace contract

using OTBailmentRequest = SharedPimpl<contract::peer::request::Bailment>;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::contract::peer::request
{
class Bailment : virtual public peer::Request
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

#ifndef _WIN32
    auto clone() const noexcept -> Bailment* override = 0;
#endif
};
}  // namespace opentxs::contract::peer::request
