// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/blockchain/crypto/Imported.hpp"  // IWYU pragma: associated

#include "internal/blockchain/crypto/Ethereum.hpp"
#include "opentxs/blockchain/crypto/Ethereum.hpp"
#include "opentxs/crypto/asymmetric/key/EllipticCurve.hpp"

namespace opentxs::blockchain::crypto::internal
{
auto Imported::asEthereum() const noexcept -> const internal::Ethereum&
{
    return internal::Ethereum::Blank();
}

auto Imported::asEthereum() noexcept -> internal::Ethereum&
{
    return internal::Ethereum::Blank();
}

auto Imported::asEthereumPublic() const noexcept -> const crypto::Ethereum&
{
    return crypto::Ethereum::Blank();
}

auto Imported::asEthereumPublic() noexcept -> crypto::Ethereum&
{
    return crypto::Ethereum::Blank();
}

auto Imported::Blank() noexcept -> Imported&
{
    static auto blank = Imported{};

    return blank;
}

auto Imported::Key() const noexcept
    -> const opentxs::crypto::asymmetric::key::EllipticCurve&
{
    static const auto blank = opentxs::crypto::asymmetric::key::EllipticCurve{};

    return blank;
}
}  // namespace opentxs::blockchain::crypto::internal
