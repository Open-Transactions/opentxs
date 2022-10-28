// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <opentxs/opentxs.hpp>

namespace ot = opentxs;

namespace ottest
{
struct OPENTXS_EXPORT PaymentCodeVectorV1 {
    ot::UnallocatedCString words_{};
    ot::UnallocatedCString payment_code_{};
    ot::UnallocatedVector<ot::UnallocatedCString> receiving_address_{};
    ot::UnallocatedCString private_key_{};
    ot::UnallocatedCString outpoint_{};
    ot::UnallocatedCString blinded_{};
};

struct OPENTXS_EXPORT PaymentCodeVectorsV1 {
    PaymentCodeVectorV1 alice_{};
    PaymentCodeVectorV1 bob_{};
};

OPENTXS_EXPORT auto GetPaymentCodeVectors1() noexcept
    -> const PaymentCodeVectorsV1&;
}  // namespace ottest
