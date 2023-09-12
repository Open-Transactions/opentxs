// Copyright (c) 2010-2023 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "ottest/fixtures/paymentcode/PaymentCode_v1.hpp"  // IWYU pragma: associated

#include <opentxs/opentxs.hpp>

#include "ottest/data/crypto/PaymentCodeV1.hpp"
#include "ottest/fixtures/paymentcode/Helpers.hpp"

namespace ottest
{
namespace ot = opentxs;

//    static constexpr auto version_ = std::uint8_t{1};
//    static constexpr auto chain_ = ot::blockchain::Type::Bitcoin;

auto PaymentCode_v1::KeyToAddress(
    const ot::crypto::asymmetric::key::EllipticCurve& key) const noexcept
    -> ot::UnallocatedCString
{
    constexpr auto style = ot::blockchain::crypto::AddressStyle::P2PKH;

    return api_.Crypto().Blockchain().CalculateAddress(
        chain_, style, api_.Factory().DataFromBytes(key.PublicKey()));
}

PaymentCode_v1::PaymentCode_v1()
    : PC_Fixture_Base(
          version_,
          version_,
          GetPaymentCodeVectors1().alice_.words_,
          GetPaymentCodeVectors1().bob_.words_,
          GetPaymentCodeVectors1().alice_.payment_code_,
          GetPaymentCodeVectors1().bob_.payment_code_)
    , alice_blind_secret_(user_1_.blinding_key_secret(
          api_,
          GetPaymentCodeVectors1().alice_.private_key_,
          reason_))
    , alice_blind_public_(user_1_.blinding_key_public())
{
}
}  // namespace ottest
