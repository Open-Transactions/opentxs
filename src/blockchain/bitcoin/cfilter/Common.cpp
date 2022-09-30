// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"  // IWYU pragma: associated
#include "opentxs/blockchain/bitcoin/cfilter/Types.hpp"  // IWYU pragma: associated

#include "opentxs/blockchain/bitcoin/cfilter/FilterType.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::blockchain::cfilter
{
using namespace std::literals;

auto print(Type in) noexcept -> std::string_view
{
    try {
        static const auto map = Map<Type, std::string_view>{
            {Type::Basic_BIP158, "Basic_BIP158"sv},
            {Type::Basic_BCHVariant, "Basic_BCHVariant"sv},
            {Type::ES, "ES"sv},
            {Type::Unknown, "Unknown"sv},
        };

        return map.at(in);
    } catch (...) {
        LogAbort()(__FUNCTION__)("invalid Type: ")(static_cast<TypeEnum>(in))
            .Abort();
    }
}
}  // namespace opentxs::blockchain::cfilter
