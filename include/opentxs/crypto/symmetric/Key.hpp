// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Export.hpp"
#include "opentxs/crypto/symmetric/Types.hpp"
#include "opentxs/util/Allocated.hpp"
#include "opentxs/util/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace crypto
{
namespace symmetric
{
namespace internal
{
class Key;
}  // namespace internal

class KeyPrivate;
}  // namespace symmetric
}  // namespace crypto

namespace identifier
{
class Generic;
}  // namespace identifier

class PasswordPrompt;
class Secret;
class Writer;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::crypto::symmetric
{
class OPENTXS_EXPORT Key : virtual public opentxs::Allocated
{
public:
    operator bool() const noexcept { return IsValid(); }

    [[nodiscard]] auto Decrypt(
        ReadView ciphertext,
        Writer&& plaintext,
        const PasswordPrompt& reason) const noexcept -> bool;
    [[nodiscard]] auto Encrypt(
        ReadView plaintext,
        Writer&& ciphertext,
        Algorithm mode,
        const PasswordPrompt& reason,
        bool attachKey = true,
        ReadView iv = {}) const noexcept -> bool;
    [[nodiscard]] auto Encrypt(
        ReadView plaintext,
        Writer&& ciphertext,
        const PasswordPrompt& reason,
        bool attachKey = true,
        ReadView iv = {}) const noexcept -> bool;
    auto get_allocator() const noexcept -> allocator_type final;
    auto ID(const PasswordPrompt& reason) const noexcept
        -> const identifier::Generic&;
    auto Internal() const noexcept -> const internal::Key&;
    [[nodiscard]] auto IsValid() const noexcept -> bool;
    [[nodiscard]] auto Unlock(const PasswordPrompt& reason) const noexcept
        -> bool;

    [[nodiscard]] auto ChangePassword(
        const Secret& newPassword,
        const PasswordPrompt& reason) noexcept -> bool;
    auto Internal() noexcept -> internal::Key&;
    auto swap(Key& rhs) noexcept -> void;

    OPENTXS_NO_EXPORT Key(KeyPrivate* imp) noexcept;
    Key(allocator_type alloc = {}) noexcept;
    Key(const Key& rhs, allocator_type alloc = {}) noexcept;
    Key(Key&& rhs) noexcept;
    Key(Key&& rhs, allocator_type alloc) noexcept;
    auto operator=(const Key& rhs) noexcept -> Key&;
    auto operator=(Key&& rhs) noexcept -> Key&;

    ~Key() override;

private:
    KeyPrivate* imp_;
};
}  // namespace opentxs::crypto::symmetric
