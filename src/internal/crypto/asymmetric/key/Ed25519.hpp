// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "internal/crypto/asymmetric/key/HD.hpp"

namespace opentxs::crypto::asymmetric::internal::key
{
class Ed25519 : virtual public HD
{
public:
    static auto Blank() noexcept -> Ed25519&;

    Ed25519(const Ed25519&) = delete;
    Ed25519(Ed25519&&) = delete;
    auto operator=(const Ed25519& rhs) noexcept -> Ed25519& = delete;
    auto operator=(Ed25519&& rhs) noexcept -> Ed25519& = delete;

    ~Ed25519() override = default;

protected:
    Ed25519() = default;
};
}  // namespace opentxs::crypto::asymmetric::internal::key
