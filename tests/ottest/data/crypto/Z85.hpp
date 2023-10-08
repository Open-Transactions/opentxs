// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <array>
#include <string_view>

namespace ottest
{
// NOTE https://rfc.zeromq.org/spec/32/
static constexpr auto z85_plaintext_ = std::array<unsigned char, 8>{
    0x86,
    0x4f,
    0xd2,
    0x6f,
    0xb5,
    0x59,
    0xf7,
    0x5b};
static constexpr auto z85_encoded_ =
    std::array<char, 10>{'H', 'e', 'l', 'l', 'o', 'W', 'o', 'r', 'l', 'd'};
static const auto z85_plaintext_view_ = std::string_view{
    reinterpret_cast<const char*>(z85_plaintext_.data()),
    z85_plaintext_.size()};
static constexpr auto z85_encoded_view_ =
    std::string_view{z85_encoded_.data(), z85_encoded_.size()};
}  // namespace ottest
