// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"              // IWYU pragma: associated
#include "api/crypto/Symmetric.hpp"  // IWYU pragma: associated

#include <Ciphertext.pb.h>
#include <memory>

#include "internal/api/Crypto.hpp"
#include "internal/api/crypto/Factory.hpp"
#include "internal/api/session/FactoryAPI.hpp"
#include "internal/crypto/library/SymmetricProvider.hpp"
#include "internal/serialization/protobuf/Proto.tpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/crypto/symmetric/Algorithm.hpp"
#include "opentxs/crypto/symmetric/Key.hpp"
#include "opentxs/crypto/symmetric/Source.hpp"

namespace opentxs::factory
{
auto Symmetric(const api::Session& api) noexcept
    -> std::unique_ptr<api::crypto::Symmetric>
{
    using ReturnType = api::crypto::imp::Symmetric;

    return std::make_unique<ReturnType>(api);
}
}  // namespace opentxs::factory

namespace opentxs::api::crypto::imp
{
Symmetric::Symmetric(const api::Session& api) noexcept
    : api_(api)
{
}

auto Symmetric::IvSize(const opentxs::crypto::symmetric::Algorithm mode)
    const noexcept -> std::size_t
{
    const auto& provider = api_.Crypto().Internal().SymmetricProvider(mode);

    return provider.IvSize(mode);
}

auto Symmetric::Key(
    opentxs::crypto::symmetric::Algorithm mode,
    const PasswordPrompt& password,
    alloc::Default alloc) const noexcept -> opentxs::crypto::symmetric::Key
{
    const auto& provider = api_.Crypto().Internal().SymmetricProvider(mode);

    return api_.Factory().InternalSession().SymmetricKey(
        provider, mode, password, alloc);
}

auto Symmetric::Key(const PasswordPrompt& password, alloc::Default alloc)
    const noexcept -> opentxs::crypto::symmetric::Key
{
    return Key(
        opentxs::crypto::symmetric::Algorithm::ChaCha20Poly1305,
        password,
        alloc);
}

auto Symmetric::Key(
    ReadView serializedCiphertext,
    const opentxs::crypto::symmetric::Algorithm mode,
    alloc::Default alloc) const noexcept -> opentxs::crypto::symmetric::Key
{
    const auto& provider = api_.Crypto().Internal().SymmetricProvider(mode);
    auto ciphertext = proto::Factory<proto::Ciphertext>(serializedCiphertext);

    return api_.Factory().InternalSession().SymmetricKey(
        provider, ciphertext.key(), alloc);
}

auto Symmetric::Key(
    const Secret& seed,
    const opentxs::crypto::symmetric::Algorithm mode,
    const opentxs::crypto::symmetric::Source type,
    const std::uint64_t operations,
    const std::uint64_t difficulty,
    alloc::Default alloc) const noexcept -> opentxs::crypto::symmetric::Key
{
    const auto& provider = api_.Crypto().Internal().SymmetricProvider(mode);

    return api_.Factory().InternalSession().SymmetricKey(
        provider,
        seed,
        operations,
        difficulty,
        provider.KeySize(mode),
        type,
        alloc);
}

auto Symmetric::Key(
    const Secret& seed,
    const opentxs::crypto::symmetric::Source type,
    const std::uint64_t operations,
    const std::uint64_t difficulty,
    alloc::Default alloc) const noexcept -> opentxs::crypto::symmetric::Key
{
    return Key(
        seed,
        opentxs::crypto::symmetric::Algorithm::ChaCha20Poly1305,
        type,
        operations,
        difficulty,
        alloc);
}

auto Symmetric::Key(
    const Secret& seed,
    const opentxs::crypto::symmetric::Algorithm mode,
    const std::uint64_t operations,
    const std::uint64_t difficulty,
    alloc::Default alloc) const noexcept -> opentxs::crypto::symmetric::Key
{
    return Key(
        seed,
        mode,
        opentxs::crypto::symmetric::Source::Argon2i,
        operations,
        difficulty,
        alloc);
}

auto Symmetric::Key(
    const Secret& seed,
    const std::uint64_t operations,
    const std::uint64_t difficulty,
    alloc::Default alloc) const noexcept -> opentxs::crypto::symmetric::Key
{
    return Key(
        seed,
        opentxs::crypto::symmetric::Algorithm::ChaCha20Poly1305,
        opentxs::crypto::symmetric::Source::Argon2i,
        operations,
        difficulty,
        alloc);
}

auto Symmetric::Key(
    const Secret& seed,
    ReadView salt,
    std::uint64_t operations,
    std::uint64_t difficulty,
    std::uint64_t parallel,
    std::size_t bytes,
    opentxs::crypto::symmetric::Source type,
    alloc::Default alloc) const noexcept -> opentxs::crypto::symmetric::Key
{
    const auto& provider = api_.Crypto().Internal().SymmetricProvider(type);

    return api_.Factory().InternalSession().SymmetricKey(
        provider,
        seed,
        salt,
        operations,
        difficulty,
        parallel,
        bytes,
        type,
        alloc);
}

auto Symmetric::Key(
    const proto::SymmetricKey& serialized,
    opentxs::crypto::symmetric::Algorithm mode,
    alloc::Default alloc) const noexcept -> opentxs::crypto::symmetric::Key
{
    const auto& provider = api_.Crypto().Internal().SymmetricProvider(mode);

    return api_.Factory().InternalSession().SymmetricKey(
        provider, serialized, alloc);
}
}  // namespace opentxs::api::crypto::imp
