// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "internal/blockchain/crypto/Deterministic.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"

namespace opentxs::blockchain::crypto::internal
{
class HD : virtual public Deterministic
{
public:
    static auto Blank() noexcept -> HD&;

    virtual auto Standard() const noexcept -> HDProtocol;

    HD() = default;
    HD(const HD&) = delete;
    HD(HD&&) = delete;
    auto operator=(const HD&) -> HD& = delete;
    auto operator=(HD&&) -> HD& = delete;

    ~HD() override = default;
};
}  // namespace opentxs::blockchain::crypto::internal
