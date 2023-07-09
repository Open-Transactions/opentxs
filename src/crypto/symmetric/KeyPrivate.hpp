// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/api/session/Session.hpp"

#pragma once

#include <cs_shared_guarded.h>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <new>
#include <optional>
#include <shared_mutex>

#include "internal/crypto/symmetric/Key.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/core/Secret.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/crypto/symmetric/Types.hpp"
#include "opentxs/util/Numbers.hpp"
#include "opentxs/util/Types.hpp"
#include "util/Allocated.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{

namespace crypto
{
class SymmetricProvider;
}  // namespace crypto

namespace proto
{
class Ciphertext;
class SymmetricKey;
}  // namespace proto

class PasswordPrompt;
class Writer;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::crypto::symmetric
{
class KeyPrivate : virtual public internal::Key,
                   public opentxs::implementation::Allocated
{
public:
    static auto operator delete(
        KeyPrivate* ptr,
        std::destroying_delete_t) noexcept -> void;

    auto API() const noexcept -> const api::Session& override;
    [[nodiscard]] virtual auto clone(allocator_type alloc) const noexcept
        -> KeyPrivate*;
    [[nodiscard]] auto Decrypt(
        const proto::Ciphertext& ciphertext,
        Writer&& plaintext,
        const PasswordPrompt& reason) const noexcept -> bool override;
    [[nodiscard]] virtual auto Decrypt(
        ReadView ciphertext,
        Writer&& plaintext,
        const PasswordPrompt& reason) const noexcept -> bool;
    [[nodiscard]] auto Encrypt(
        ReadView plaintext,
        proto::Ciphertext& ciphertext,
        const PasswordPrompt& reason,
        bool attachKey = true,
        ReadView iv = {}) const noexcept -> bool override;
    [[nodiscard]] auto Encrypt(
        ReadView plaintext,
        Algorithm mode,
        proto::Ciphertext& ciphertext,
        const PasswordPrompt& reason,
        bool attachKey = true,
        ReadView iv = {}) const noexcept -> bool override;
    [[nodiscard]] virtual auto Encrypt(
        ReadView plaintext,
        Writer&& ciphertext,
        Algorithm mode,
        bool attachKey,
        ReadView iv,
        const PasswordPrompt& reason) const noexcept -> bool;
    [[nodiscard]] virtual auto Encrypt(
        ReadView plaintext,
        Writer&& ciphertext,
        bool attachKey,
        ReadView iv,
        const PasswordPrompt& reason) const noexcept -> bool;
    [[nodiscard]] virtual auto get_deleter() noexcept -> std::function<void()>;
    virtual auto ID(const PasswordPrompt& reason) const noexcept
        -> const identifier::Generic&;
    [[nodiscard]] virtual auto IsValid() const noexcept -> bool;
    [[nodiscard]] auto RawKey(Secret& output, const PasswordPrompt& reason)
        const noexcept -> bool override;
    [[nodiscard]] auto Serialize(proto::SymmetricKey& output) const noexcept
        -> bool override;
    [[nodiscard]] virtual auto Unlock(
        const PasswordPrompt& reason) const noexcept -> bool;

    [[nodiscard]] virtual auto ChangePassword(
        const Secret& newPassword,
        const PasswordPrompt& reason) noexcept -> bool;

    KeyPrivate(allocator_type alloc) noexcept;
    KeyPrivate(const KeyPrivate&) = delete;
    KeyPrivate(KeyPrivate&&) = delete;
    auto operator=(const KeyPrivate&) -> KeyPrivate& = delete;
    auto operator=(KeyPrivate&&) -> KeyPrivate& = delete;

    ~KeyPrivate() override;
};
}  // namespace opentxs::crypto::symmetric

namespace opentxs::crypto::symmetric::implementation
{
class Key final : public KeyPrivate
{
public:
    static constexpr auto default_operations_ = 3u;
    static constexpr auto default_difficulty_ = 8388608u;
    static constexpr auto default_threads_ = 1u;

    auto API() const noexcept -> const api::Session& final { return api_; }
    [[nodiscard]] auto clone(allocator_type alloc) const noexcept
        -> KeyPrivate* final;
    [[nodiscard]] auto Decrypt(
        const proto::Ciphertext& ciphertext,
        Writer&& plaintext,
        const PasswordPrompt& reason) const noexcept -> bool final;
    [[nodiscard]] auto Decrypt(
        ReadView ciphertext,
        Writer&& plaintext,
        const PasswordPrompt& reason) const noexcept -> bool final;
    [[nodiscard]] auto Encrypt(
        ReadView plaintext,
        proto::Ciphertext& ciphertext,
        const PasswordPrompt& reason,
        bool attachKey = true,
        ReadView iv = {}) const noexcept -> bool final;
    [[nodiscard]] auto Encrypt(
        ReadView plaintext,
        Algorithm mode,
        proto::Ciphertext& ciphertext,
        const PasswordPrompt& reason,
        bool attachKey = true,
        ReadView iv = {}) const noexcept -> bool final;
    [[nodiscard]] auto Encrypt(
        ReadView plaintext,
        Writer&& ciphertext,
        Algorithm mode,
        bool attachKey,
        ReadView iv,
        const PasswordPrompt& reason) const noexcept -> bool final;
    [[nodiscard]] auto Encrypt(
        ReadView plaintext,
        Writer&& ciphertext,
        bool attachKey,
        ReadView iv,
        const PasswordPrompt& reason) const noexcept -> bool final;
    [[nodiscard]] auto get_deleter() noexcept -> std::function<void()> final;
    auto ID(const PasswordPrompt& reason) const noexcept
        -> const identifier::Generic& final;
    auto IsValid() const noexcept -> bool final { return true; }
    [[nodiscard]] auto RawKey(Secret& output, const PasswordPrompt& reason)
        const noexcept -> bool final;
    [[nodiscard]] auto Serialize(proto::SymmetricKey& output) const noexcept
        -> bool final;
    [[nodiscard]] auto Unlock(const PasswordPrompt& reason) const noexcept
        -> bool final;

    [[nodiscard]] auto ChangePassword(
        const Secret& newPassword,
        const PasswordPrompt& reason) noexcept -> bool final;
    auto Derive(
        crypto::symmetric::Algorithm mode,
        const PasswordPrompt& reason) noexcept(false) -> bool;
    auto SetRawKey(
        const opentxs::Secret& raw,
        const PasswordPrompt& reason) noexcept(false) -> bool;

    Key(const api::Session& api,
        const crypto::SymmetricProvider& engine,
        allocator_type alloc) noexcept;
    Key(const api::Session& api,
        const crypto::SymmetricProvider& engine,
        const proto::SymmetricKey& serialized,
        allocator_type alloc) noexcept;
    Key(const api::Session& api,
        const crypto::SymmetricProvider& engine,
        const opentxs::Secret& seed,
        ReadView salt,
        std::size_t size,
        std::uint64_t operations,
        std::uint64_t difficulty,
        std::uint64_t parallel,
        crypto::symmetric::Source type,
        allocator_type alloc) noexcept(false);
    Key(const api::Session& api,
        const crypto::SymmetricProvider& engine,
        VersionNumber version,
        crypto::symmetric::Source type,
        std::size_t keySize,
        ReadView salt,
        std::uint64_t operations,
        std::uint64_t difficulty,
        std::uint64_t parallel,
        std::optional<Secret> plaintextKey,
        std::unique_ptr<proto::Ciphertext> encryptedKey,
        allocator_type alloc) noexcept;
    Key() = delete;
    Key(const Key& rhs, allocator_type alloc = {}) noexcept;
    Key(Key&&) = delete;
    auto operator=(const Key&) -> Key& = delete;
    auto operator=(Key&&) -> Key& = delete;

    ~Key() final = default;

private:
    static constexpr auto default_version_ = VersionNumber{1u};

    struct Data {
        std::size_t key_size_;
        ByteArray salt_;
        std::uint64_t operations_;
        std::uint64_t difficulty_;
        std::uint64_t parallel_;
        std::optional<Secret> plaintext_key_;
        std::unique_ptr<proto::Ciphertext> encrypted_key_;
        std::optional<identifier::Generic> id_;

        Data(
            std::size_t keySize,
            ReadView salt,
            std::uint64_t operations,
            std::uint64_t difficulty,
            std::uint64_t parallel,
            std::optional<Secret> plaintextKey,
            std::unique_ptr<proto::Ciphertext> encryptedKey,
            std::optional<identifier::Generic> id,
            allocator_type alloc) noexcept;
        Data() = delete;
        Data(const Data& rhs, allocator_type alloc = {}) noexcept;
        Data(Data&&) = delete;
        auto operator=(const Data&) -> Data& = delete;
        auto operator=(Data&&) -> Data& = delete;

        ~Data();
    };

    using GuardedData = libguarded::shared_guarded<Data, std::shared_mutex>;

    const api::Session& api_;
    const crypto::SymmetricProvider& engine_;
    const VersionNumber version_;
    const crypto::symmetric::Source type_;
    mutable GuardedData data_;

    auto decrypt(
        Data& data,
        const proto::Ciphertext& ciphertext,
        Writer&& plaintext,
        const PasswordPrompt& reason) const noexcept(false) -> bool;
    auto derive(
        Data& data,
        crypto::symmetric::Algorithm mode,
        const PasswordPrompt& reason) const noexcept(false) -> bool;
    auto encrypt(
        Data& data,
        ReadView plaintext,
        Algorithm mode,
        proto::Ciphertext& ciphertext,
        const PasswordPrompt& reason,
        bool attachKey,
        ReadView iv) const noexcept(false) -> void;
    auto encrypt_key(
        Data& data,
        const opentxs::Secret& plaintextKey,
        const crypto::symmetric::Source type,
        const PasswordPrompt& reason) const noexcept(false) -> bool;
    auto encrypt_key(
        Data& data,
        const opentxs::Secret& plaintextKey,
        const PasswordPrompt& reason) const noexcept(false) -> bool;
    auto encrypted_password(
        const Data& data,
        symmetric::Algorithm mode,
        const PasswordPrompt& reason) const noexcept(false) -> Secret;
    auto get_password(const PasswordPrompt& reason, Secret& out) const
        noexcept(false) -> bool;
    auto id(Data& data, const PasswordPrompt& reason) const noexcept(false)
        -> const identifier::Generic&;
    auto serialize(const Data& data, proto::SymmetricKey& output) const
        noexcept(false) -> bool;
    auto set_raw_key(
        Data& data,
        const opentxs::Secret& raw,
        const PasswordPrompt& reason) noexcept(false) -> bool;
    auto unlock(Data& data, const PasswordPrompt& reason) const noexcept(false)
        -> const Secret&;
};
}  // namespace opentxs::crypto::symmetric::implementation
