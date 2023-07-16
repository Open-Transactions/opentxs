// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::crypto::asymmetric::KeyPrivate

#pragma once

#include "crypto/asymmetric/base/KeyPrivate.hpp"
#include "internal/crypto/asymmetric/key/EllipticCurve.hpp"
#include "internal/util/PMR.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/crypto/asymmetric/key/EllipticCurve.hpp"
#include "opentxs/util/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace crypto
{
namespace asymmetric
{

namespace key
{
class Ed25519;
class HD;
class Secp256k1;
}  // namespace key
}  // namespace asymmetric

}  // namespace crypto

class PasswordPrompt;
class Secret;
class Writer;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::crypto::asymmetric::key
{
class EllipticCurvePrivate : virtual public internal::key::EllipticCurve,
                             virtual public KeyPrivate
{
public:
    static auto Blank(allocator_type alloc) noexcept -> EllipticCurvePrivate*
    {
        return pmr::default_construct<EllipticCurvePrivate>({alloc});
    }

    virtual auto asEd25519Public() const noexcept
        -> const asymmetric::key::Ed25519&;
    auto asEllipticCurve() const noexcept
        -> const internal::key::EllipticCurve& override
    {
        return *this;
    }
    [[nodiscard]] auto asEllipticCurvePrivate() const noexcept
        -> const key::EllipticCurvePrivate* override
    {
        return this;
    }
    virtual auto asHDPublic() const noexcept -> const asymmetric::key::HD&;
    virtual auto asSecp256k1Public() const noexcept
        -> const asymmetric::key::Secp256k1&;
    [[nodiscard]] auto get_deleter() noexcept -> delete_function override
    {
        return pmr::make_deleter(this);
    }
    [[nodiscard]] auto clone(allocator_type alloc) const noexcept
        -> asymmetric::KeyPrivate* override
    {
        return pmr::clone_as<asymmetric::KeyPrivate>(this, {alloc});
    }
    virtual auto IncrementPrivate(
        const Secret& scalar,
        const PasswordPrompt& reason,
        allocator_type alloc) const noexcept -> asymmetric::key::EllipticCurve;
    virtual auto IncrementPublic(const Secret& scalar, allocator_type alloc)
        const noexcept -> asymmetric::key::EllipticCurve;
    virtual auto SignDER(
        const ReadView preimage,
        const crypto::HashType hash,
        Writer&& output,
        const PasswordPrompt& reason) const noexcept -> bool;

    virtual auto asEd25519Public() noexcept -> asymmetric::key::Ed25519&;
    auto asEllipticCurve() noexcept -> internal::key::EllipticCurve& override
    {
        return *this;
    }
    [[nodiscard]] auto asEllipticCurvePrivate() noexcept
        -> key::EllipticCurvePrivate* override
    {
        return this;
    }
    virtual auto asHDPublic() noexcept -> asymmetric::key::HD&;
    virtual auto asSecp256k1Public() noexcept -> asymmetric::key::Secp256k1&;

    EllipticCurvePrivate(allocator_type alloc) noexcept;
    EllipticCurvePrivate() = delete;
    EllipticCurvePrivate(
        const EllipticCurvePrivate& rhs,
        allocator_type alloc) noexcept;
    EllipticCurvePrivate(const EllipticCurvePrivate&) = delete;
    EllipticCurvePrivate(EllipticCurvePrivate&&) = delete;
    auto operator=(const EllipticCurvePrivate&)
        -> EllipticCurvePrivate& = delete;
    auto operator=(EllipticCurvePrivate&&) -> EllipticCurvePrivate& = delete;

    ~EllipticCurvePrivate() override;
};
}  // namespace opentxs::crypto::asymmetric::key
