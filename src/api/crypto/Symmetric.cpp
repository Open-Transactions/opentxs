// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"              // IWYU pragma: associated
#include "api/crypto/Symmetric.hpp"  // IWYU pragma: associated

#include <Ciphertext.pb.h>
#include <memory>

#include "Proto.tpp"
#include "internal/api/Crypto.hpp"
#include "internal/api/crypto/Factory.hpp"
#include "internal/api/session/FactoryAPI.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/crypto/key/Symmetric.hpp"
#include "opentxs/crypto/library/SymmetricProvider.hpp"

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

auto Symmetric::IvSize(
    const opentxs::crypto::key::symmetric::Algorithm mode) const -> std::size_t
{
    const auto& provider = api_.Crypto().Internal().SymmetricProvider(mode);

    return provider.IvSize(mode);
}

auto Symmetric::Key(
    const PasswordPrompt& password,
    const opentxs::crypto::key::symmetric::Algorithm mode) const
    -> OTSymmetricKey
{
    const auto& provider = api_.Crypto().Internal().SymmetricProvider(mode);

    return api_.Factory().SymmetricKey(provider, password, mode);
}

auto Symmetric::Key(
    const proto::SymmetricKey& serialized,
    const opentxs::crypto::key::symmetric::Algorithm mode) const
    -> OTSymmetricKey
{
    const auto& provider = api_.Crypto().Internal().SymmetricProvider(mode);

    return api_.Factory().InternalSession().SymmetricKey(provider, serialized);
}

auto Symmetric::Key(
    const ReadView& serializedCiphertext,
    const opentxs::crypto::key::symmetric::Algorithm mode) const
    -> OTSymmetricKey
{
    const auto& provider = api_.Crypto().Internal().SymmetricProvider(mode);
    auto ciphertext = proto::Factory<proto::Ciphertext>(serializedCiphertext);

    return api_.Factory().InternalSession().SymmetricKey(
        provider, ciphertext.key());
}

auto Symmetric::Key(
    const Secret& seed,
    const std::uint64_t operations,
    const std::uint64_t difficulty,
    const opentxs::crypto::key::symmetric::Algorithm mode,
    const opentxs::crypto::key::symmetric::Source type) const -> OTSymmetricKey
{
    const auto& provider = api_.Crypto().Internal().SymmetricProvider(mode);

    return api_.Factory().SymmetricKey(
        provider, seed, operations, difficulty, provider.KeySize(mode), type);
}

auto Symmetric::Key(
    const Secret& seed,
    const ReadView salt,
    const std::uint64_t operations,
    const std::uint64_t difficulty,
    const std::uint64_t parallel,
    const std::size_t bytes,
    const opentxs::crypto::key::symmetric::Source type) const -> OTSymmetricKey
{
    const auto& provider = api_.Crypto().Internal().SymmetricProvider(type);

    return api_.Factory().SymmetricKey(
        provider, seed, salt, operations, difficulty, parallel, bytes, type);
}
}  // namespace opentxs::api::crypto::imp
