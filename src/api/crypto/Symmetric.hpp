// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <cstdint>
#include <iosfwd>

#include "internal/api/crypto/Symmetric.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "opentxs/api/crypto/Symmetric.hpp"
#include "opentxs/crypto/key/Symmetric.hpp"
#include "opentxs/crypto/key/symmetric/Algorithm.hpp"
#include "opentxs/crypto/key/symmetric/Source.hpp"
#include "opentxs/util/Bytes.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace crypto
{
class Symmetric;
}  // namespace crypto

class Session;
}  // namespace api

namespace crypto
{
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

namespace opentxs::api::crypto::imp
{
class Symmetric final : public internal::Symmetric
{
public:
    auto IvSize(const opentxs::crypto::key::symmetric::Algorithm mode) const
        -> std::size_t final;
    auto Key(
        const PasswordPrompt& password,
        const opentxs::crypto::key::symmetric::Algorithm mode =
            opentxs::crypto::key::symmetric::Algorithm::ChaCha20Poly1305) const
        -> OTSymmetricKey final;
    auto Key(
        const proto::SymmetricKey& serialized,
        const opentxs::crypto::key::symmetric::Algorithm mode) const
        -> OTSymmetricKey final;
    auto Key(
        const ReadView& serializedCiphertext,
        const opentxs::crypto::key::symmetric::Algorithm mode) const
        -> OTSymmetricKey final;
    auto Key(
        const Secret& seed,
        const std::uint64_t operations = 0,
        const std::uint64_t difficulty = 0,
        const opentxs::crypto::key::symmetric::Algorithm mode =
            opentxs::crypto::key::symmetric::Algorithm::ChaCha20Poly1305,
        const opentxs::crypto::key::symmetric::Source type =
            opentxs::crypto::key::symmetric::Source::Argon2i) const
        -> OTSymmetricKey final;
    auto Key(
        const Secret& seed,
        const ReadView salt,
        const std::uint64_t operations,
        const std::uint64_t difficulty,
        const std::uint64_t parallel,
        const std::size_t bytes,
        const opentxs::crypto::key::symmetric::Source type) const
        -> OTSymmetricKey final;

    Symmetric(const api::Session& api) noexcept;
    Symmetric() = delete;
    Symmetric(const Symmetric&) = delete;
    Symmetric(Symmetric&&) = delete;
    auto operator=(const Symmetric&) -> Symmetric& = delete;
    auto operator=(Symmetric&&) -> Symmetric& = delete;

    ~Symmetric() final = default;

private:
    const api::Session& api_;
};
}  // namespace opentxs::api::crypto::imp
