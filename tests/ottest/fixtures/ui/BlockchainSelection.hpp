// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <opentxs/opentxs.hpp>

namespace ot = opentxs;

namespace ottest
{
struct OPENTXS_EXPORT BlockchainSelectionRow {
    ot::UnallocatedCString name_{};
    bool enabled_{};
    bool testnet_{};
    ot::blockchain::Type type_{};
};

struct OPENTXS_EXPORT BlockchainSelectionData {
    ot::UnallocatedVector<BlockchainSelectionRow> rows_{};
};

OPENTXS_EXPORT auto check_blockchain_selection(
    const ot::api::session::Client& api,
    const ot::ui::Blockchains type,
    const BlockchainSelectionData& expected) noexcept -> bool;
OPENTXS_EXPORT auto check_blockchain_selection_qt(
    const ot::api::session::Client& api,
    const ot::ui::Blockchains type,
    const BlockchainSelectionData& expected) noexcept -> bool;
}  // namespace ottest
