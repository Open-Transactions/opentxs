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
using namespace opentxs::literals;

class OPENTXS_EXPORT PaymentCode_v3 : public PC_Fixture_Base
{
public:
    static constexpr auto version_ = std::uint8_t{3};

    const ot::crypto::asymmetric::key::EllipticCurve& alice_blind_secret_;
    const ot::crypto::asymmetric::key::EllipticCurve& alice_blind_public_;
    const ot::crypto::asymmetric::key::EllipticCurve& bob_blind_secret_;
    const ot::crypto::asymmetric::key::EllipticCurve& bob_blind_public_;

    PaymentCode_v3();
};
}  // namespace ottest
