// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"
#include "opentxs/util/Container.hpp"

namespace ot = opentxs;

namespace ottest
{
struct VectorV1 {
    ot::UnallocatedCString words_{};
    ot::UnallocatedCString payment_code_{};
    ot::UnallocatedVector<ot::UnallocatedCString> receiving_address_{};
    ot::UnallocatedCString private_key_{};
    ot::UnallocatedCString outpoint_{};
    ot::UnallocatedCString blinded_{};
};

struct VectorsV1 {
    VectorV1 alice_{};
    VectorV1 bob_{};
};

auto GetVectors1() noexcept -> const VectorsV1&;
}  // namespace ottest
