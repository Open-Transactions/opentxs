// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <opentxs/opentxs.hpp>
#include <utility>

namespace ottest
{
auto OPENTXS_EXPORT GetBtcBlock762580() noexcept
    -> std::pair<opentxs::blockchain::block::Hash, opentxs::ReadView>;
auto OPENTXS_EXPORT GetBtcBlock762580_bad_header() noexcept
    -> opentxs::ReadView;
auto OPENTXS_EXPORT GetBtcBlock762580_bad_txid() noexcept -> opentxs::ReadView;
auto OPENTXS_EXPORT GetBtcBlock762580_bad_wtxid() noexcept -> opentxs::ReadView;
auto OPENTXS_EXPORT GetTnDashBlock7000() noexcept
    -> std::pair<opentxs::blockchain::block::Hash, opentxs::ReadView>;
}  // namespace ottest
