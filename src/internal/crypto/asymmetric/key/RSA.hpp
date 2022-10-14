// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "internal/crypto/asymmetric/Key.hpp"

namespace opentxs::crypto::asymmetric::internal::key
{
class RSA : virtual public Key
{
public:
    static auto Blank() noexcept -> RSA&;

    RSA(const RSA&) = delete;
    RSA(RSA&&) = delete;
    auto operator=(const RSA& rhs) noexcept -> RSA& = delete;
    auto operator=(RSA&& rhs) noexcept -> RSA& = delete;

    ~RSA() override = default;

protected:
    RSA() = default;
};
}  // namespace opentxs::crypto::asymmetric::internal::key
