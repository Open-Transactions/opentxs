// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "internal/serialization/protobuf/Basic.hpp"
#include "opentxs/Version.hpp"

namespace opentxs::proto
{
auto BasketParamsAllowedBasketItem() noexcept -> const VersionMap&;
auto CurrencyParamsAllowedDisplayScales() noexcept -> const VersionMap&;
auto DisplayScaleAllowedScaleRatios() noexcept -> const VersionMap&;
auto IssuerAllowedPeerRequestHistory() noexcept -> const VersionMap&;
auto IssuerAllowedUnitAccountMap() noexcept -> const VersionMap&;
auto PeerRequestHistoryAllowedPeerRequestWorkflow() noexcept
    -> const VersionMap&;
auto ServerContractAllowedListenAddress() noexcept -> const VersionMap&;
auto ServerContractAllowedNym() noexcept -> const VersionMap&;
auto ServerContractAllowedSignature() noexcept -> const VersionMap&;
auto UnitDefinitionAllowedBasketParams() noexcept -> const VersionMap&;
auto UnitDefinitionAllowedCurrencyParams() noexcept -> const VersionMap&;
auto UnitDefinitionAllowedNym() noexcept -> const VersionMap&;
auto UnitDefinitionAllowedSecurityParams() noexcept -> const VersionMap&;
auto UnitDefinitionAllowedSignature() noexcept -> const VersionMap&;
}  // namespace opentxs::proto
