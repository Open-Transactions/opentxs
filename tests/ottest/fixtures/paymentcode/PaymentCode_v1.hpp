// Copyright (c) 2010-2023 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <opentxs/opentxs.hpp>
#include <cstdint>

#include "ottest/fixtures/paymentcode/Helpers.hpp"

namespace ottest
{
namespace ot = opentxs;

class OPENTXS_EXPORT PaymentCode_v1 : public PC_Fixture_Base
{
public:
    static constexpr auto version_ = std::uint8_t{1};
    static constexpr auto chain_ = ot::blockchain::Type::Bitcoin;

    const ot::crypto::asymmetric::key::EllipticCurve& alice_blind_secret_;
    const ot::crypto::asymmetric::key::EllipticCurve& alice_blind_public_;

    auto KeyToAddress(const ot::crypto::asymmetric::key::EllipticCurve& key)
        const noexcept -> ot::UnallocatedCString;

    PaymentCode_v1();
};
}  // namespace ottest
