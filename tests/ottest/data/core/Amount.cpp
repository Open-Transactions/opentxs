// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "ottest/data/core/Amount.hpp"  // IWYU pragma: associated

#include <initializer_list>

namespace ottest
{
using namespace std::literals;

// NOTE: https://ethereum.org/pt/developers/docs/apis/json-rpc/#hex-encoding
// TODO check the ethereum submodule for more test vectors
auto EthereumTestAmounts() noexcept
    -> std::span<const std::pair<int, std::string_view>>
{
    static constexpr auto data =
        std::initializer_list<std::pair<int, std::string_view>>{
            {65, "0x41"sv},
            {255, "0xff"sv},
            {1024, "0x400"sv},
        };

    return data;
}
}  // namespace ottest
