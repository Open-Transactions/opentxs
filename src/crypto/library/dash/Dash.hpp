// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "internal/crypto/library/Dash.hpp"

#include "opentxs/Types.hpp"
#include "opentxs/crypto/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
class Writer;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::crypto::implementation
{
class Dash final : virtual public crypto::Dash
{
public:
    auto Digest(
        const crypto::HashType hashType,
        const ReadView data,
        Writer&& output) const noexcept -> bool final;
    auto HMAC(
        const crypto::HashType hashType,
        const ReadView key,
        const ReadView data,
        Writer&& output) const noexcept -> bool final;

    Dash() = default;
    Dash(const Dash&) = delete;
    Dash(Dash&&) = delete;
    auto operator=(const Dash&) -> Dash& = delete;
    auto operator=(Dash&&) -> Dash& = delete;

    ~Dash() final = default;
};
}  // namespace opentxs::crypto::implementation
