// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/blockchain/crypto/Imported.hpp"  // IWYU pragma: associated

#include <utility>

#include "internal/blockchain/crypto/Imported.hpp"
#include "internal/blockchain/crypto/Subaccount.hpp"

namespace opentxs::blockchain::crypto
{
Imported::Imported(std::shared_ptr<internal::Subaccount> imp) noexcept
    : Subaccount(std::move(imp))
{
}

Imported::Imported(const Imported& rhs) noexcept
    : Subaccount(rhs)
{
}

Imported::Imported(Imported&& rhs) noexcept
    : Subaccount(std::move(rhs))
{
}

auto Imported::asEthereum() const noexcept -> const crypto::Ethereum&
{
    return const_cast<Imported*>(this)->asEthereum();
}

auto Imported::asEthereum() noexcept -> crypto::Ethereum&
{
    if (auto p = imp_.lock(); p) {
        return p->asImported().asEthereumPublic();
    } else {
        return internal::Imported::Blank().asEthereumPublic();
    }
}

auto Imported::Blank() noexcept -> Imported&
{
    static auto blank = Imported{std::make_shared<internal::Imported>()};

    return blank;
}

Imported::~Imported() = default;
}  // namespace opentxs::blockchain::crypto
