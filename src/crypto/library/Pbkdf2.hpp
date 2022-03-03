// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <iosfwd>
#include <mutex>

#include "internal/crypto/library/Pbkdf2.hpp"
#include "opentxs/crypto/HashType.hpp"

namespace opentxs::crypto::implementation
{
class Pbkdf2 : virtual public crypto::Pbkdf2
{
public:
    auto PKCS5_PBKDF2_HMAC(
        const void* input,
        const std::size_t inputSize,
        const void* salt,
        const std::size_t saltSize,
        const std::size_t iterations,
        const crypto::HashType hashType,
        const std::size_t bytes,
        void* output) const noexcept -> bool final;

    ~Pbkdf2() override = default;

protected:
    Pbkdf2() noexcept;

private:
    mutable std::mutex pbkdf_lock_;

    Pbkdf2(const Pbkdf2&) = delete;
    Pbkdf2(Pbkdf2&&) = delete;
    auto operator=(const Pbkdf2&) -> Pbkdf2& = delete;
    auto operator=(Pbkdf2&&) -> Pbkdf2& = delete;
};
}  // namespace opentxs::crypto::implementation
