// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <memory>

#include "opentxs/crypto/key/Asymmetric.hpp"
#include "opentxs/crypto/key/Keypair.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace crypto
{
class EcdsaProvider;
}  // namespace crypto
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::crypto::key
{
class OPENTXS_EXPORT EllipticCurve : virtual public Asymmetric
{
public:
    static const VersionNumber DefaultVersion;
    static const VersionNumber MaxVersion;

    virtual auto asPublicEC() const noexcept
        -> std::unique_ptr<EllipticCurve> = 0;
    virtual auto CloneEC() const noexcept -> std::unique_ptr<EllipticCurve> = 0;
    virtual auto ECDSA() const noexcept -> const crypto::EcdsaProvider& = 0;
    virtual auto IncrementPrivate(
        const Secret& scalar,
        const PasswordPrompt& reason) const noexcept
        -> std::unique_ptr<EllipticCurve> = 0;
    virtual auto IncrementPublic(const Secret& scalar) const noexcept
        -> std::unique_ptr<EllipticCurve> = 0;
    virtual auto SignDER(
        const ReadView preimage,
        const crypto::HashType hash,
        Space& output,
        const PasswordPrompt& reason) const noexcept -> bool = 0;

    ~EllipticCurve() override = default;

protected:
    EllipticCurve() = default;

private:
    EllipticCurve(const EllipticCurve&) = delete;
    EllipticCurve(EllipticCurve&&) = delete;
    auto operator=(const EllipticCurve&) -> EllipticCurve& = delete;
    auto operator=(EllipticCurve&&) -> EllipticCurve& = delete;
};
}  // namespace opentxs::crypto::key
