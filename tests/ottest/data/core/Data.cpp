// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "ottest/data/core/Data.hpp"  // IWYU pragma: associated

namespace ottest
{
using namespace std::literals;

auto HexWithoutPrefix() noexcept -> std::span<const std::string_view>
{
    static constexpr auto data = {
        ""sv,
        "61"sv,
        "626262"sv,
        "636363"sv,
        "73696d706c792061206c6f6e6720737472696e67"sv,
        "00eb15231dfceb60925886b67d065299925915aeb172c06647"sv,
        "516b6fcd0fbf4f89001e670274dd572e4794"sv,
        "ecac89cad93923c02321"sv,
        "10c8511e"sv,
        "00000000000000000000"sv,
        "000111d38e5fc9071ffcd20b4a763cc9ae4f252bb4e48fd66a835e252ada93ff480d6dd43dc62a641155a5"sv,
        "000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f202122232425262728292a2b2c2d2e2f303132333435363738393a3b3c3d3e3f404142434445464748494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f606162636465666768696a6b6c6d6e6f707172737475767778797a7b7c7d7e7f808182838485868788898a8b8c8d8e8f909192939495969798999a9b9c9d9e9fa0a1a2a3a4a5a6a7a8a9aaabacadaeafb0b1b2b3b4b5b6b7b8b9babbbcbdbebfc0c1c2c3c4c5c6c7c8c9cacbcccdcecfd0d1d2d3d4d5d6d7d8d9dadbdcdddedfe0e1e2e3e4e5e6e7e8e9eaebecedeeeff0f1f2f3f4f5f6f7f8f9fafbfcfdfeff"sv,
    };

    return data;
}

auto HexWithPrefix() noexcept -> std::span<const std::string_view>
{
    static constexpr auto data = {
        "0x000000000000000000"sv,
        "0X000111d38e5fc9071ffcd20b4a763cc9ae4f252bb4e48fd66a835e252ada93ff480d6dd43dc62a641155a5"sv,
    };

    return data;
}
}  // namespace ottest
