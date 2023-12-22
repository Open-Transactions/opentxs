// Copyright (c) 2010-2023 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "ottest/fixtures/paymentcode/PaymentCode_v3.hpp"  // IWYU pragma: associated

#include <opentxs/opentxs.hpp>

#include "ottest/data/crypto/PaymentCodeV3.hpp"
#include "ottest/fixtures/paymentcode/Helpers.hpp"

namespace ottest
{
using namespace opentxs::literals;

PaymentCode_v3::PaymentCode_v3()
    : PC_Fixture_Base(
          version_,
          version_,
          GetPaymentCodeVector3().alice_.words_,
          GetPaymentCodeVector3().bob_.words_,
          GetPaymentCodeVector3().alice_.payment_code_,
          GetPaymentCodeVector3().bob_.payment_code_)
    , alice_blind_secret_(user_1_.blinding_key_secret(
          api_,
          GetPaymentCodeVector3().bob_.receive_chain_,
          reason_))
    , alice_blind_public_(user_1_.blinding_key_public())
    , bob_blind_secret_(user_2_.blinding_key_secret(
          api_,
          GetPaymentCodeVector3().alice_.receive_chain_,
          reason_))
    , bob_blind_public_(user_2_.blinding_key_public())
{
}
}  // namespace ottest
