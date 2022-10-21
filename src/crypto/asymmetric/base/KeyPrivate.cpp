// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                           // IWYU pragma: associated
#include "crypto/asymmetric/base/KeyPrivate.hpp"  // IWYU pragma: associated

#include <cstdint>
#include <utility>

#include "crypto/asymmetric/key/ed25519/Ed25519Private.hpp"
#include "crypto/asymmetric/key/ellipticcurve/EllipticCurvePrivate.hpp"
#include "crypto/asymmetric/key/hd/HDPrivate.hpp"
#include "crypto/asymmetric/key/rsa/RSAPrivate.hpp"
#include "crypto/asymmetric/key/secp256k1/Secp256k1Private.hpp"
#include "internal/crypto/asymmetric/key/EllipticCurve.hpp"
#include "internal/crypto/asymmetric/key/RSA.hpp"
#include "internal/crypto/library/Null.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/crypto/HashType.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/crypto/asymmetric/Algorithm.hpp"
#include "opentxs/crypto/asymmetric/Key.hpp"
#include "opentxs/crypto/asymmetric/Role.hpp"
#include "opentxs/crypto/asymmetric/key/EllipticCurve.hpp"
#include "opentxs/crypto/asymmetric/key/RSA.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Writer.hpp"

namespace opentxs::crypto::asymmetric::internal
{
auto Key::asEllipticCurve() const noexcept -> const key::EllipticCurve&
{
    return internal::key::EllipticCurve::Blank();
}

auto Key::asEllipticCurve() noexcept -> key::EllipticCurve&
{
    return internal::key::EllipticCurve::Blank();
}

auto Key::asRSA() const noexcept -> const key::RSA&
{
    return internal::key::RSA::Blank();
}

auto Key::asRSA() noexcept -> key::RSA& { return internal::key::RSA::Blank(); }

auto Key::CalculateHash(const crypto::HashType, const PasswordPrompt&)
    const noexcept -> ByteArray
{
    return {};
}

auto Key::CalculateID(identifier::Generic&) const noexcept -> bool
{
    return {};
}

auto Key::CalculateSessionPassword(
    const asymmetric::Key&,
    const PasswordPrompt&,
    Secret&) const noexcept -> bool
{
    return {};
}

auto Key::CalculateTag(
    const asymmetric::Key&,
    const identifier::Generic&,
    const PasswordPrompt&,
    std::uint32_t&) const noexcept -> bool
{
    return {};
}

auto Key::CalculateTag(
    const identity::Authority&,
    const Algorithm,
    const PasswordPrompt&,
    std::uint32_t&,
    Secret&) const noexcept -> bool
{
    return {};
}

auto Key::GetMetadata() const noexcept -> const OTSignatureMetadata*
{
    return {};
}

auto Key::operator==(const Serialized&) const noexcept -> bool { return {}; }

auto Key::Params() const noexcept -> ReadView { return {}; }

auto Key::Path() const noexcept -> const UnallocatedCString { return {}; }

auto Key::Path(proto::HDPath&) const noexcept -> bool { return {}; }

auto Key::Provider() const noexcept -> const crypto::AsymmetricProvider&
{
    static const auto provider = opentxs::crypto::blank::AsymmetricProvider{};

    return provider;
}

auto Key::Serialize(Serialized&) const noexcept -> bool { return {}; }

auto Key::Sign(
    const GetPreimage,
    const crypto::SignatureRole,
    proto::Signature&,
    const identifier::Generic&,
    const crypto::HashType,
    const PasswordPrompt&) const noexcept -> bool
{
    return {};
}

auto Key::Sign(
    const GetPreimage,
    const crypto::SignatureRole,
    proto::Signature&,
    const identifier::Generic&,
    const PasswordPrompt&) const noexcept -> bool
{
    return {};
}

auto Key::TransportKey(Data&, Secret&, const PasswordPrompt&) const noexcept
    -> bool
{
    return {};
}

auto Key::Verify(const Data&, const proto::Signature&) const noexcept -> bool
{
    return {};
}
}  // namespace opentxs::crypto::asymmetric::internal

namespace opentxs::crypto::asymmetric
{
KeyPrivate::KeyPrivate(allocator_type alloc) noexcept
    : Allocated(std::move(alloc))
{
}

KeyPrivate::KeyPrivate(const KeyPrivate& rhs, allocator_type alloc) noexcept
    : Allocated(std::move(alloc))
{
}

auto KeyPrivate::asEd25519Private() const noexcept -> const key::Ed25519Private*
{
    static const auto blank = key::Ed25519Private{alloc::System()};

    return std::addressof(blank);
}

auto KeyPrivate::asEd25519Private() noexcept -> key::Ed25519Private*
{
    static auto blank = key::Ed25519Private{alloc::System()};

    return std::addressof(blank);
}

auto KeyPrivate::asEllipticCurvePrivate() const noexcept
    -> const key::EllipticCurvePrivate*
{
    static const auto blank = key::EllipticCurvePrivate{alloc::System()};

    return std::addressof(blank);
}

auto KeyPrivate::asEllipticCurvePrivate() noexcept -> key::EllipticCurvePrivate*
{
    static auto blank = key::EllipticCurvePrivate{alloc::System()};

    return std::addressof(blank);
}

auto KeyPrivate::asEllipticCurvePublic() const noexcept
    -> const key::EllipticCurve&
{
    return asymmetric::key::EllipticCurve::Blank();
}

auto KeyPrivate::asEllipticCurvePublic() noexcept -> key::EllipticCurve&
{
    return asymmetric::key::EllipticCurve::Blank();
}

auto KeyPrivate::asHDPrivate() const noexcept -> const key::HDPrivate*
{
    static const auto blank = key::HDPrivate{alloc::System()};

    return std::addressof(blank);
}

auto KeyPrivate::asHDPrivate() noexcept -> key::HDPrivate*
{
    static auto blank = key::HDPrivate{alloc::System()};

    return std::addressof(blank);
}

auto KeyPrivate::asPublic(allocator_type) const noexcept -> asymmetric::Key
{
    return {};
}

auto KeyPrivate::asRSAPrivate() const noexcept -> const key::RSAPrivate*
{
    static auto blank = key::RSAPrivate{alloc::System()};

    return std::addressof(blank);
}

auto KeyPrivate::asRSAPrivate() noexcept -> key::RSAPrivate*
{
    static auto blank = key::RSAPrivate{alloc::System()};

    return std::addressof(blank);
}

auto KeyPrivate::asRSAPublic() const noexcept -> const key::RSA&
{
    return asymmetric::key::RSA::Blank();
}

auto KeyPrivate::asRSAPublic() noexcept -> key::RSA&
{
    return asymmetric::key::RSA::Blank();
}

auto KeyPrivate::asSecp256k1Private() const noexcept
    -> const key::Secp256k1Private*
{
    static auto blank = key::Secp256k1Private{alloc::System()};

    return std::addressof(blank);
}

auto KeyPrivate::asSecp256k1Private() noexcept -> key::Secp256k1Private*
{
    static auto blank = key::Secp256k1Private{alloc::System()};

    return std::addressof(blank);
}

auto KeyPrivate::Blank(allocator_type alloc) noexcept -> KeyPrivate*
{
    auto pmr = alloc::PMR<KeyPrivate>{alloc};
    auto* out = pmr.allocate(1_uz);

    OT_ASSERT(nullptr != out);

    pmr.construct(out);

    return out;
}

auto KeyPrivate::clone(allocator_type alloc) const noexcept -> KeyPrivate*
{
    auto pmr = alloc::PMR<KeyPrivate>{alloc};
    auto* out = pmr.allocate(1_uz);

    OT_ASSERT(nullptr != out);

    pmr.construct(out, *this);

    return out;
}

auto KeyPrivate::ErasePrivateData() noexcept -> bool { return true; }

auto KeyPrivate::get_deleter() const noexcept
    -> std::function<void(KeyPrivate*)>
{
    return [alloc = alloc::PMR<KeyPrivate>{get_allocator()}](
               KeyPrivate* p) mutable {
        OT_ASSERT(nullptr != p);

        alloc.destroy(p);
        alloc.deallocate(p, 1_uz);
    };
}

auto KeyPrivate::HasCapability(identity::NymCapability) const noexcept -> bool
{
    return false;
}

auto KeyPrivate::HasPrivate() const noexcept -> bool { return false; }

auto KeyPrivate::HasPublic() const noexcept -> bool { return false; }

auto KeyPrivate::IsValid() const noexcept -> bool { return false; }

auto KeyPrivate::operator delete(
    KeyPrivate* ptr,
    std::destroying_delete_t) noexcept -> void
{
    auto deleter = ptr->get_deleter();
    std::invoke(deleter, ptr);
}

auto KeyPrivate::PreferredHash() const noexcept -> crypto::HashType
{
    return HashType::Error;
}

auto KeyPrivate::PrivateKey(const PasswordPrompt& reason) const noexcept
    -> ReadView
{
    return {};
}

auto KeyPrivate::PublicKey() const noexcept -> ReadView { return {}; }

auto KeyPrivate::Reset(asymmetric::Key& key) noexcept -> void
{
    key.imp_ = nullptr;
}

auto KeyPrivate::Role() const noexcept -> asymmetric::Role
{
    return Role::Error;
}

auto KeyPrivate::Sign(
    ReadView,
    Writer&&,
    crypto::HashType,
    const PasswordPrompt&) const noexcept -> bool
{
    return false;
}

auto KeyPrivate::Type() const noexcept -> asymmetric::Algorithm
{
    return Algorithm::Error;
}

auto KeyPrivate::Verify(ReadView, ReadView) const noexcept -> bool
{
    return false;
}

auto KeyPrivate::Version() const noexcept -> VersionNumber { return 0; }

KeyPrivate::~KeyPrivate() = default;
}  // namespace opentxs::crypto::asymmetric
