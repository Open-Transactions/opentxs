// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <cstdint>

#include "opentxs/crypto/symmetric/Types.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

namespace crypto
{
namespace symmetric
{
class KeyPrivate;
}  // namespace symmetric

class SymmetricProvider;
}  // namespace crypto

namespace proto
{
class SymmetricKey;
}  // namespace proto

class PasswordPrompt;
class Secret;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::factory
{
auto SymmetricKey(
    const api::Session& api,
    const crypto::SymmetricProvider& engine,
    const crypto::symmetric::Algorithm mode,
    const opentxs::PasswordPrompt& reason,
    alloc::Strategy alloc) noexcept -> crypto::symmetric::KeyPrivate*;
auto SymmetricKey(
    const api::Session& api,
    const crypto::SymmetricProvider& engine,
    const proto::SymmetricKey& serialized,
    alloc::Strategy alloc) noexcept -> crypto::symmetric::KeyPrivate*;
auto SymmetricKey(
    const api::Session& api,
    const crypto::SymmetricProvider& engine,
    const opentxs::Secret& seed,
    const std::uint64_t operations,
    const std::uint64_t difficulty,
    const std::size_t size,
    const crypto::symmetric::Source type,
    alloc::Strategy alloc) noexcept -> crypto::symmetric::KeyPrivate*;
auto SymmetricKey(
    const api::Session& api,
    const crypto::SymmetricProvider& engine,
    const opentxs::Secret& seed,
    const ReadView salt,
    const std::uint64_t operations,
    const std::uint64_t difficulty,
    const std::uint64_t parallel,
    const std::size_t size,
    const crypto::symmetric::Source type,
    alloc::Strategy alloc) noexcept -> crypto::symmetric::KeyPrivate*;
auto SymmetricKey(
    const api::Session& api,
    const crypto::SymmetricProvider& engine,
    const opentxs::Secret& raw,
    const opentxs::PasswordPrompt& reason,
    alloc::Strategy alloc) noexcept -> crypto::symmetric::KeyPrivate*;
}  // namespace opentxs::factory
