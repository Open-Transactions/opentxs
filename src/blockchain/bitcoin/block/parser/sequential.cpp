// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "blockchain/bitcoin/block/parser/Base.hpp"  // IWYU pragma: associated

#include <algorithm>

namespace opentxs::blockchain::bitcoin::block
{
auto ParserBase::get_transactions(std::span<Data> view) const noexcept -> void
{
    std::for_each(
        view.begin(), view.end(), [this](auto& t) { get_transaction(t); });
}
}  // namespace opentxs::blockchain::bitcoin::block
