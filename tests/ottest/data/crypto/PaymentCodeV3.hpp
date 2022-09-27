// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <opentxs/opentxs.hpp>

namespace ot = opentxs;

namespace ottest
{
struct PaymentCodeVectorV3 {
    ot::UnallocatedCString words_{};
    ot::UnallocatedCString payment_code_{};
    ot::UnallocatedVector<ot::UnallocatedCString> locators_{};
    ot::blockchain::Type receive_chain_{};
    ot::UnallocatedVector<ot::UnallocatedCString> receive_keys_{};
    ot::UnallocatedCString change_key_secret_{};
    ot::UnallocatedCString change_key_public_{};
    ot::UnallocatedCString blinded_payment_code_{};
    ot::UnallocatedCString f_{};
    ot::UnallocatedCString g_{};
};

struct PaymentCodeVectorsV3 {
    ot::UnallocatedCString outpoint_{};
    PaymentCodeVectorV3 alice_{};
    PaymentCodeVectorV3 bob_{};
    PaymentCodeVectorV3 chris_{};
    PaymentCodeVectorV3 daniel_{};
};

auto GetPaymentCodeVector3() noexcept -> const PaymentCodeVectorsV3&;
}  // namespace ottest
