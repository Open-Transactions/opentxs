// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>

#include "opentxs/Export.hpp"
#include "opentxs/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
class Amount;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::protocol::bitcoin::bitcoincash::token::cashtoken
{
enum class Capability : std::uint8_t;  // IWYU pragma: export

struct OPENTXS_EXPORT View {
    ReadView category_id_{};
    bool nft_{};
    Capability capability_{};
    ReadView commitment_{};
    const Amount* amount_{};

    auto HasAmount() const noexcept -> bool { return nullptr != amount_; }
};
}  // namespace
   // opentxs::blockchain::protocol::bitcoin::bitcoincash::token::cashtoken
