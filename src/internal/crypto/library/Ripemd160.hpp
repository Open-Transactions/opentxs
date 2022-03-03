// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>  // IWYU pragma: keep
#include <cstdint>

namespace opentxs::crypto
{
class Ripemd160
{
public:
    virtual auto RIPEMD160(
        const std::uint8_t* input,
        const std::size_t inputSize,
        std::uint8_t* output) const -> bool = 0;

    virtual ~Ripemd160() = default;

protected:
    Ripemd160() = default;

private:
    Ripemd160(const Ripemd160&) = delete;
    Ripemd160(Ripemd160&&) = delete;
    auto operator=(const Ripemd160&) -> Ripemd160& = delete;
    auto operator=(Ripemd160&&) -> Ripemd160& = delete;
};
}  // namespace opentxs::crypto
