// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>

#include "opentxs/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
class Writer;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::crypto
{
class Scrypt
{
public:
    virtual auto Generate(
        const ReadView input,
        const ReadView salt,
        const std::uint64_t N,
        const std::uint32_t r,
        const std::uint32_t p,
        const std::size_t bytes,
        Writer&& writer) const noexcept -> bool = 0;

    Scrypt(const Scrypt&) = delete;
    Scrypt(Scrypt&&) = delete;
    auto operator=(const Scrypt&) -> Scrypt& = delete;
    auto operator=(Scrypt&&) -> Scrypt& = delete;

    virtual ~Scrypt() = default;

protected:
    Scrypt() = default;
};
}  // namespace opentxs::crypto
