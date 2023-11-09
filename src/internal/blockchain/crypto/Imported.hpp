// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "internal/blockchain/crypto/Subaccount.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace blockchain
{
namespace crypto
{
namespace internal
{
class Ethereum;
}  // namespace internal

class Ethereum;
}  // namespace crypto
}  // namespace blockchain

namespace crypto
{
namespace asymmetric
{
namespace key
{
class EllipticCurve;
}  // namespace key
}  // namespace asymmetric
}  // namespace crypto
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::crypto::internal
{
class Imported : virtual public Subaccount
{
public:
    static auto Blank() noexcept -> Imported&;

    virtual auto asEthereum() const noexcept -> const internal::Ethereum&;
    virtual auto asEthereumPublic() const noexcept -> const crypto::Ethereum&;
    virtual auto Key() const noexcept
        -> const opentxs::crypto::asymmetric::key::EllipticCurve&;

    virtual auto asEthereum() noexcept -> internal::Ethereum&;
    virtual auto asEthereumPublic() noexcept -> crypto::Ethereum&;

    Imported() = default;
    Imported(const Imported&) = delete;
    Imported(Imported&&) = delete;
    auto operator=(const Imported&) -> Imported& = delete;
    auto operator=(Imported&&) -> Imported& = delete;

    ~Imported() override = default;
};
}  // namespace opentxs::blockchain::crypto::internal
