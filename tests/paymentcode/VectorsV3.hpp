// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <string>
#include <vector>

#include "opentxs/Version.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/core/Types.hpp"
#include "opentxs/util/Bytes.hpp"

namespace ot = opentxs;

namespace ottest
{
struct VectorV3 {
    std::string words_{};
    std::string payment_code_{};
    std::pmr::vector<std::string> locators_{};
    ot::blockchain::Type receive_chain_{};
    std::pmr::vector<std::string> receive_keys_{};
    std::string change_key_secret_{};
    std::string change_key_public_{};
    std::string blinded_payment_code_{};
    std::string F_{};
    std::string G_{};
};

struct VectorsV3 {
    std::string outpoint_{};
    VectorV3 alice_{};
    VectorV3 bob_{};
    VectorV3 chris_{};
    VectorV3 daniel_{};
};

auto GetVectors3() noexcept -> const VectorsV3&;
}  // namespace ottest
