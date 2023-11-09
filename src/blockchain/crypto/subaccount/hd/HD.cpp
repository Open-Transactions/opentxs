// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/blockchain/crypto/HD.hpp"  // IWYU pragma: associated

#include <utility>

#include "internal/blockchain/crypto/Deterministic.hpp"
#include "internal/blockchain/crypto/HD.hpp"
#include "internal/blockchain/crypto/Subaccount.hpp"

namespace opentxs::blockchain::crypto
{
HD::HD(std::shared_ptr<internal::Subaccount> imp) noexcept
    : Deterministic(std::move(imp))
{
}

HD::HD(const HD& rhs) noexcept
    : Deterministic(rhs)
{
}

HD::HD(HD&& rhs) noexcept
    : Deterministic(std::move(rhs))
{
}

auto HD::Blank() noexcept -> HD&
{
    static auto blank = HD{std::make_shared<internal::HD>()};

    return blank;
}

auto HD::Standard() const noexcept -> HDProtocol
{
    if (auto p = imp_.lock(); p) {
        return p->asDeterministic().asHD().Standard();
    } else {
        return internal::HD::Blank().Standard();
    }
}

HD::~HD() = default;
}  // namespace opentxs::blockchain::crypto
