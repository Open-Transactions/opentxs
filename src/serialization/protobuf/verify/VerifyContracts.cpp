// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/serialization/protobuf/verify/VerifyContracts.hpp"  // IWYU pragma: associated

namespace opentxs::proto
{
auto BasketParamsAllowedBasketItem() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{
        {1, {1, 1}},
    };

    return output;
}
auto CurrencyParamsAllowedDisplayScales() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{
        {1, {1, 1}},
    };

    return output;
}
auto DisplayScaleAllowedScaleRatios() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{
        {1, {1, 1}},
    };

    return output;
}
auto IssuerAllowedPeerRequestHistory() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{
        {1, {1, 1}},
    };

    return output;
}
auto IssuerAllowedUnitAccountMap() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{
        {1, {1, 1}},
    };

    return output;
}
auto PeerRequestHistoryAllowedPeerRequestWorkflow() noexcept
    -> const VersionMap&
{
    static const auto output = VersionMap{
        {1, {1, 1}},
    };

    return output;
}
auto ServerContractAllowedIdentifier() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{
        {1, {1, 1}},
        {2, {1, 1}},
    };

    return output;
}
auto ServerContractAllowedListenAddress() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{
        {1, {1, 1}},
        {2, {1, 2}},
    };

    return output;
}
auto ServerContractAllowedNym() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{
        {1, {1, 5}},
        {2, {1, 6}},
    };

    return output;
}
auto ServerContractAllowedSignature() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{
        {1, {1, 1}},
        {2, {1, 1}},
    };

    return output;
}
auto SignatureAllowedIdentifier() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{
        {1, {1, 1}},
        {2, {1, 1}},
        {3, {1, 1}},
    };

    return output;
}
auto UnitDefinitionAllowedBasketParams() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{
        {1, {1, 1}},
        {2, {1, 1}},
    };

    return output;
}
auto UnitDefinitionAllowedCurrencyParams() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{
        {1, {1, 1}},
        {2, {1, 1}},
    };

    return output;
}
auto UnitDefinitionAllowedIdentifier() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{
        {1, {1, 1}},
        {2, {1, 1}},
    };

    return output;
}
auto UnitDefinitionAllowedNym() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{
        {1, {1, 6}},
        {2, {1, 6}},
    };

    return output;
}
auto UnitDefinitionAllowedSecurityParams() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{
        {1, {1, 1}},
        {2, {1, 1}},
    };

    return output;
}
auto UnitDefinitionAllowedSignature() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{
        {1, {1, 1}},
        {2, {1, 1}},
    };

    return output;
}
}  // namespace opentxs::proto
