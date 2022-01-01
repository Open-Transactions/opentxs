// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <string>
#include <vector>

namespace ottest
{
struct VectorV1 {
    std::string words_{};
    std::string payment_code_{};
    std::pmr::vector<std::string> receiving_address_{};
    std::string private_key_{};
    std::string outpoint_{};
    std::string blinded_{};
};

struct VectorsV1 {
    VectorV1 alice_{};
    VectorV1 bob_{};
};

auto GetVectors1() noexcept -> const VectorsV1&;
}  // namespace ottest
