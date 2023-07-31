// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/crypto/Bip32.hpp"
// IWYU pragma: no_include "opentxs/crypto/Seed.hpp"

#pragma once

#include "opentxs/crypto/Types.hpp"
#include "opentxs/util/Time.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace crypto
{
class Symmetric;
}  // namespace crypto

namespace session
{
class Factory;
class Storage;
}  // namespace session

class Crypto;
class Session;
}  // namespace api

namespace crypto
{
class Bip32;
class Bip39;
class Seed;
}  // namespace crypto

namespace proto
{
class Seed;
}  // namespace proto

class PasswordPrompt;
class Secret;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::factory
{
auto Bip32(const api::Crypto& crypto) noexcept -> crypto::Bip32;
auto Seed(
    const api::Session& api,
    const crypto::Bip32& bip32,
    const crypto::Bip39& bip39,
    const api::crypto::Symmetric& symmetric,
    const api::session::Factory& factory,
    const api::session::Storage& storage,
    const crypto::Language lang,
    const crypto::SeedStrength strength,
    const Time createdTime,
    const opentxs::PasswordPrompt& reason) noexcept(false) -> crypto::Seed;
auto Seed(
    const api::Session& api,
    const crypto::Bip32& bip32,
    const crypto::Bip39& bip39,
    const api::crypto::Symmetric& symmetric,
    const api::session::Factory& factory,
    const api::session::Storage& storage,
    const crypto::SeedStyle type,
    const crypto::Language lang,
    const opentxs::Secret& words,
    const opentxs::Secret& passphrase,
    const Time createdTime,
    const opentxs::PasswordPrompt& reason) noexcept(false) -> crypto::Seed;
auto Seed(
    const api::Session& api,
    const crypto::Bip32& bip32,
    const crypto::Bip39& bip39,
    const api::crypto::Symmetric& symmetric,
    const api::session::Factory& factory,
    const api::session::Storage& storage,
    const opentxs::Secret& entropy,
    const Time createdTime,
    const opentxs::PasswordPrompt& reason) noexcept(false) -> crypto::Seed;
auto Seed(
    const api::Session& api,
    const crypto::Bip39& bip39,
    const api::crypto::Symmetric& symmetric,
    const api::session::Factory& factory,
    const api::session::Storage& storage,
    const proto::Seed& proto,
    const opentxs::PasswordPrompt& reason) noexcept(false) -> crypto::Seed;
}  // namespace opentxs::factory
