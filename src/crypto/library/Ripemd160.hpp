// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "internal/crypto/library/Ripemd160.hpp"

#include <cstddef>
#include <cstdint>
#include <iosfwd>
#include <mutex>

namespace opentxs::crypto::implementation
{
class Ripemd160 : virtual public crypto::Ripemd160
{
public:
    auto RIPEMD160(
        const std::uint8_t* input,
        const std::size_t inputSize,
        std::uint8_t* output) const -> bool final;

    ~Ripemd160() override = default;

protected:
    Ripemd160() noexcept;

private:
    mutable std::mutex ripemd_lock_;

    Ripemd160(const Ripemd160&) = delete;
    Ripemd160(Ripemd160&&) = delete;
    auto operator=(const Ripemd160&) -> Ripemd160& = delete;
    auto operator=(Ripemd160&&) -> Ripemd160& = delete;
};
}  // namespace opentxs::crypto::implementation
