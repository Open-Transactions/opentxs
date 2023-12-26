// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Types.hpp"
#include "opentxs/crypto/symmetric/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

namespace identifier
{
class Generic;
}  // namespace identifier

namespace proto
{
class Ciphertext;
class SymmetricKey;
}  // namespace proto

class PasswordPrompt;
class Secret;
class Writer;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::crypto::symmetric::internal
{
class Key
{
public:
    virtual auto API() const noexcept -> const api::Session& = 0;
    [[nodiscard]] virtual auto Decrypt(
        const proto::Ciphertext& ciphertext,
        Writer&& plaintext,
        const PasswordPrompt& reason) const noexcept -> bool = 0;
    [[nodiscard]] virtual auto Encrypt(
        ReadView plaintext,
        proto::Ciphertext& ciphertext,
        const PasswordPrompt& reason,
        bool attachKey = true,
        ReadView iv = {}) const noexcept -> bool = 0;
    [[nodiscard]] virtual auto Encrypt(
        ReadView plaintext,
        Algorithm mode,
        proto::Ciphertext& ciphertext,
        const PasswordPrompt& reason,
        bool attachKey = true,
        ReadView iv = {}) const noexcept -> bool = 0;
    [[nodiscard]] virtual auto RawKey(
        Secret& output,
        const PasswordPrompt& reason) const noexcept -> bool = 0;
    [[nodiscard]] virtual auto Serialize(
        proto::SymmetricKey& output) const noexcept -> bool = 0;

    Key(const Key&) = delete;
    Key(Key&&) = delete;
    auto operator=(const Key&) -> Key& = delete;
    auto operator=(Key&&) -> Key& = delete;

    virtual ~Key() = default;

protected:
    Key() = default;
};
}  // namespace opentxs::crypto::symmetric::internal
