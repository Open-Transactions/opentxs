// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"              // IWYU pragma: associated
#include "1_Internal.hpp"            // IWYU pragma: associated
#include "crypto/key/Symmetric.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <cstdint>
#include <functional>
#include <limits>
#include <memory>
#include <string_view>
#include <utility>

#include "Proto.tpp"
#include "internal/api/session/Session.hpp"
#include "internal/crypto/key/Factory.hpp"
#include "internal/crypto/key/Key.hpp"
#include "internal/crypto/key/Null.hpp"
#include "internal/serialization/protobuf/Check.hpp"
#include "internal/serialization/protobuf/verify/SymmetricKey.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Secret.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/crypto/key/Symmetric.hpp"
#include "opentxs/crypto/library/SymmetricProvider.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/PasswordPrompt.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "serialization/protobuf/Ciphertext.pb.h"
#include "util/ScopeGuard.hpp"

template class opentxs::Pimpl<opentxs::crypto::key::Symmetric>;

#define OT_SYMMETRIC_KEY_DEFAULT_OPERATIONS 3
#define OT_SYMMETRIC_KEY_DEFAULT_DIFFICULTY 8388608
#define OT_SYMMETRIC_KEY_DEFAULT_THREADS 1

namespace opentxs::factory
{
auto SymmetricKey() noexcept -> std::unique_ptr<crypto::key::Symmetric>
{
    return std::make_unique<crypto::key::blank::Symmetric>();
}

auto SymmetricKey(
    const api::Session& api,
    const crypto::SymmetricProvider& engine,
    const opentxs::PasswordPrompt& reason,
    const opentxs::crypto::key::symmetric::Algorithm mode) noexcept
    -> std::unique_ptr<crypto::key::Symmetric>
{
    using ReturnType = crypto::key::implementation::Symmetric;

    auto output = std::make_unique<ReturnType>(api, engine);

    if (false == bool(output)) { return nullptr; }

    const auto realMode{
        mode == opentxs::crypto::key::symmetric::Algorithm::Error
            ? engine.DefaultMode()
            : mode};
    Lock lock(output->lock_);
    const auto size = output->engine_.KeySize(realMode);
    output->key_size_ = size;
    output->plaintext_key_ = api.Factory().Secret(0);

    OT_ASSERT(output->plaintext_key_.has_value());

    auto& key = output->plaintext_key_.value();

    if (false == output->allocate(lock, size, key)) { return nullptr; }

    output->encrypt_key(lock, key, reason);

    return std::move(output);
}

auto SymmetricKey(
    const api::Session& api,
    const crypto::SymmetricProvider& engine,
    const proto::SymmetricKey serialized) noexcept
    -> std::unique_ptr<crypto::key::Symmetric>
{
    using ReturnType = crypto::key::implementation::Symmetric;

    if (proto::Validate(serialized, VERBOSE)) {

        return std::make_unique<ReturnType>(api, engine, serialized);
    }

    return {};
}

auto SymmetricKey(
    const api::Session& api,
    const crypto::SymmetricProvider& engine,
    const opentxs::Secret& seed,
    const std::uint64_t operations,
    const std::uint64_t difficulty,
    const std::size_t size,
    const crypto::key::symmetric::Source type) noexcept
    -> std::unique_ptr<crypto::key::Symmetric>
{
    using ReturnType = crypto::key::implementation::Symmetric;
    auto salt = Space{};
    ReturnType::Allocate(api, engine.SaltSize(type), salt, false);

    return SymmetricKey(
        api, engine, seed, reader(salt), size, operations, difficulty, 0, type);
}

auto SymmetricKey(
    const api::Session& api,
    const crypto::SymmetricProvider& engine,
    const opentxs::Secret& seed,
    const ReadView salt,
    const std::uint64_t operations,
    const std::uint64_t difficulty,
    const std::uint64_t parallel,
    const std::size_t size,
    const crypto::key::symmetric::Source type) noexcept
    -> std::unique_ptr<crypto::key::Symmetric>
{
    using ReturnType = crypto::key::implementation::Symmetric;
    const std::uint64_t ops =
        (0 == operations) ? OT_SYMMETRIC_KEY_DEFAULT_OPERATIONS : operations;
    const std::uint64_t mem =
        (0 == difficulty) ? OT_SYMMETRIC_KEY_DEFAULT_DIFFICULTY : difficulty;
    const std::uint64_t par =
        (0 == parallel) ? OT_SYMMETRIC_KEY_DEFAULT_THREADS : parallel;

    return std::make_unique<ReturnType>(
        api, engine, seed, salt, size, ops, mem, par, type);
}

auto SymmetricKey(
    const api::Session& api,
    const crypto::SymmetricProvider& engine,
    const opentxs::Secret& raw,
    const opentxs::PasswordPrompt& reason) noexcept
    -> std::unique_ptr<crypto::key::Symmetric>
{
    using ReturnType = crypto::key::implementation::Symmetric;
    auto output = std::make_unique<ReturnType>(api, engine);

    if (!output) { return {}; }

    Lock lock(output->lock_);
    output->encrypt_key(lock, raw, reason);
    output->key_size_ = raw.size();

    return std::move(output);
}
}  // namespace opentxs::factory

namespace opentxs::crypto::key
{
auto Symmetric::Factory() -> OTSymmetricKey
{
    return OTSymmetricKey{new blank::Symmetric};
}
}  // namespace opentxs::crypto::key

namespace opentxs::crypto::key::implementation
{
Symmetric::Symmetric(
    const api::Session& api,
    const crypto::SymmetricProvider& engine,
    const VersionNumber version,
    const crypto::key::symmetric::Source type,
    const std::size_t keySize,
    const ReadView salt,
    const std::uint64_t operations,
    const std::uint64_t difficulty,
    const std::uint64_t parallel,
    std::optional<OTSecret> plaintextKey,
    proto::Ciphertext* encryptedKey)
    : key::Symmetric()
    , api_(api)
    , engine_(engine)
    , version_(version)
    , type_(type)
    , key_size_(keySize)
    , operations_(operations)
    , difficulty_(difficulty)
    , parallel_(parallel)
    , salt_([&] {
        auto out = Space{};

        if (0u < salt.size()) { copy(salt, writer(out)); }

        return out;
    }())
    , plaintext_key_(plaintextKey)
    , encrypted_key_(encryptedKey)
    , lock_()
{
}

Symmetric::Symmetric(
    const api::Session& api,
    const crypto::SymmetricProvider& engine)
    : Symmetric(
          api,
          engine,
          default_version_,
          crypto::key::symmetric::Source::Raw,
          0,
          {},
          0,
          0,
          0,
          {},
          nullptr)
{
}

Symmetric::Symmetric(
    const api::Session& api,
    const crypto::SymmetricProvider& engine,
    const proto::SymmetricKey serialized)
    : Symmetric(
          api,
          engine,
          std::max(serialized.version(), default_version_),
          translate(serialized.type()),
          serialized.size(),
          serialized.salt(),
          serialized.operations(),
          serialized.difficulty(),
          serialized.parallel(),
          {},
          new proto::Ciphertext(serialized.key()))
{
}

Symmetric::Symmetric(
    const api::Session& api,
    const crypto::SymmetricProvider& engine,
    const opentxs::Secret& seed,
    const ReadView salt,
    const std::size_t size,
    const std::uint64_t operations,
    const std::uint64_t difficulty,
    const std::uint64_t parallel,
    const crypto::key::symmetric::Source type)
    : Symmetric(
          api,
          engine,
          default_version_,
          type,
          size,
          salt,
          operations,
          difficulty,
          parallel,
          api.Factory().Secret(0),
          nullptr)
{
    OT_ASSERT(0 != salt_.size());
    OT_ASSERT(plaintext_key_);
    OT_ASSERT(0 != operations);
    OT_ASSERT(0 != difficulty);
    OT_ASSERT(0 != size);

    auto lock = Lock{lock_};
    auto& plain = get_plaintext(lock);

    OT_ASSERT(plain.has_value());

    allocate(lock, key_size_, *plain);
    const auto bytes = seed.Bytes();
    const bool derived = engine.Derive(
        reinterpret_cast<const std::uint8_t*>(bytes.data()),
        bytes.size(),
        reinterpret_cast<const std::uint8_t*>(salt_.data()),
        salt_.size(),
        operations_,
        difficulty_,
        parallel,
        type_,
        reinterpret_cast<std::uint8_t*>(plain.value()->data()),
        plain.value()->size());

    OT_ASSERT(derived);
}

Symmetric::Symmetric(const Symmetric& rhs)
    : Symmetric(
          rhs.api_,
          rhs.engine_,
          rhs.version_,
          rhs.type_,
          rhs.key_size_,
          reader(rhs.salt_),
          rhs.operations_,
          rhs.difficulty_,
          rhs.parallel_,
          {},
          nullptr)
{
    if (rhs.plaintext_key_.has_value()) {
        plaintext_key_ = rhs.plaintext_key_.value();
    }

    if (rhs.encrypted_key_) {
        encrypted_key_ =
            std::make_unique<proto::Ciphertext>(*rhs.encrypted_key_);
    }
}

auto Symmetric::Allocate(const std::size_t size, Data& container) -> bool
{
    container.SetSize(size);

    return (size == container.size());
}

auto Symmetric::Allocate(const std::size_t size, String& container) -> bool
{
    if (std::numeric_limits<std::uint32_t>::max() < size) { return false; }

    auto blank = UnallocatedVector<char>{};
    blank.assign(size, 0x7f);

    OT_ASSERT(blank.size() == size);

    container.Set(blank.data(), static_cast<std::uint32_t>(blank.size()));

    return (size == container.GetLength());
}

auto Symmetric::Allocate(
    const api::Session& api,
    const std::size_t size,
    Space& container,
    const bool random) -> bool
{
    container.resize(size, std::byte{0x0});

    if (random) {
        auto secret = api.Factory().Secret(0);
        secret->Randomize(size);
        copy(secret->Bytes(), writer(container));
    }

    return (size == container.size());
}

auto Symmetric::allocate(
    const Lock& lock,
    const std::size_t size,
    Secret& container) const -> bool
{
    return size == container.Randomize(size);
}

auto Symmetric::ChangePassword(
    const opentxs::PasswordPrompt& reason,
    const opentxs::Secret& newPassword) -> bool
{
    auto lock = Lock{lock_};
    auto& plain = get_plaintext(lock);

    OT_ASSERT(plain.has_value());

    if (unlock(lock, reason)) {
        OTPasswordPrompt copy{reason};
        copy->SetPassword(newPassword);

        return encrypt_key(lock, plain.value(), copy);
    }

    LogError()(OT_PRETTY_CLASS())("Unable to unlock master key.").Flush();

    return false;
}

auto Symmetric::clone() const -> Symmetric* { return new Symmetric(*this); }

auto Symmetric::decrypt(
    const Lock& lock,
    const proto::Ciphertext& input,
    const opentxs::PasswordPrompt& reason,
    std::uint8_t* plaintext) const -> bool
{
    auto& plain = get_plaintext(lock);

    if (false == plain.has_value()) {
        if (false == unlock(lock, reason)) {
            LogError()(OT_PRETTY_CLASS())("Unable to unlock master key.")
                .Flush();

            return false;
        }
    }

    OT_ASSERT(plain.has_value());

    const bool output = engine_.Decrypt(
        input,
        reinterpret_cast<const std::uint8_t*>(plain.value()->data()),
        plain.value()->size(),
        plaintext);

    if (false == output) {
        LogError()(OT_PRETTY_CLASS())("Unable to decrypt key.").Flush();

        return false;
    }

    return output;
}

auto Symmetric::Decrypt(
    const proto::Ciphertext& ciphertext,
    const opentxs::PasswordPrompt& reason,
    const AllocateOutput plaintext) const -> bool
{
    auto lock = Lock{lock_};

    if (false == bool(plaintext)) {
        LogError()(OT_PRETTY_CLASS())("Missing output allocator").Flush();

        return false;
    }

    auto output = plaintext(ciphertext.data().size());

    if (false == output.valid(ciphertext.data().size())) {
        LogError()(OT_PRETTY_CLASS())(
            "Unable to allocate space for decryption.")
            .Flush();

        return false;
    }

    return decrypt(lock, ciphertext, reason, output.as<std::uint8_t>());
}

auto Symmetric::Decrypt(
    const ReadView& ciphertext,
    const opentxs::PasswordPrompt& reason,
    const AllocateOutput plaintext) const -> bool
{
    return Decrypt(
        proto::Factory<proto::Ciphertext>(ciphertext), reason, plaintext);
}

auto Symmetric::encrypt(
    const Lock& lock,
    const std::uint8_t* input,
    const std::size_t inputSize,
    const std::uint8_t* iv,
    const std::size_t ivSize,
    const opentxs::crypto::key::symmetric::Algorithm mode,
    const opentxs::PasswordPrompt& reason,
    proto::Ciphertext& ciphertext,
    const bool text) const -> bool
{
    if (nullptr == input) {
        LogError()(OT_PRETTY_CLASS())("Null input.").Flush();

        return false;
    }

    auto& plain = get_plaintext(lock);

    if (false == plain.has_value()) {
        if (false == unlock(lock, reason)) {
            LogError()(OT_PRETTY_CLASS())("Unable to unlock master key.")
                .Flush();

            return false;
        }
    }

    OT_ASSERT(plain.has_value());

    ciphertext.set_version(1);

    if (opentxs::crypto::key::symmetric::Algorithm::Error == mode) {
        ciphertext.set_mode(translate(engine_.DefaultMode()));
    } else {
        ciphertext.set_mode(translate(mode));
    }

    if ((0u == ivSize) || (nullptr == iv)) {
        const auto random = [&] {
            auto out = api_.Factory().Secret(0);
            const auto size = engine_.IvSize(translate(ciphertext.mode()));
            out->Randomize(size);

            OT_ASSERT(out->size() == size);

            return out;
        }();

        ciphertext.set_iv(random->data(), random->size());
    } else {
        ciphertext.set_iv(iv, ivSize);
    }

    ciphertext.set_text(text);

    OT_ASSERT(nullptr != plain.value()->data());

    return engine_.Encrypt(
        input,
        inputSize,
        reinterpret_cast<const std::uint8_t*>(plain.value()->data()),
        plain.value()->size(),
        ciphertext);
}

auto Symmetric::Encrypt(
    const ReadView plaintext,
    const opentxs::PasswordPrompt& reason,
    proto::Ciphertext& ciphertext,
    const bool attachKey,
    const opentxs::crypto::key::symmetric::Algorithm mode,
    const ReadView iv) const -> bool
{
    auto lock = Lock{lock_};
    auto success = encrypt(
        lock,
        reinterpret_cast<const std::uint8_t*>(plaintext.data()),
        plaintext.size(),
        reinterpret_cast<const std::uint8_t*>(iv.data()),
        iv.size(),
        mode,
        reason,
        ciphertext,
        true);

    if (success && attachKey) {
        success &= serialize(lock, *ciphertext.mutable_key());
    }

    return success;
}

auto Symmetric::Encrypt(
    const ReadView plaintext,
    const opentxs::PasswordPrompt& reason,
    AllocateOutput ciphertext,
    const bool attachKey,
    const opentxs::crypto::key::symmetric::Algorithm mode,
    const ReadView iv) const -> bool
{
    auto serialized = proto::Ciphertext{};

    if (false == Encrypt(plaintext, reason, serialized, attachKey, mode, iv)) {
        LogError()(OT_PRETTY_CLASS())("Failed to encrypt data.").Flush();

        return false;
    }

    auto view = ciphertext(serialized.ByteSizeLong());
    if (false == serialized.SerializeToArray(
                     view.data(), static_cast<int>(view.size()))) {
        LogError()(OT_PRETTY_CLASS())("Failed to serialize encrypted data.")
            .Flush();

        return false;
    }

    return true;
}

auto Symmetric::encrypt_key(
    const Lock& lock,
    const opentxs::Secret& plaintextKey,
    const opentxs::PasswordPrompt& reason,
    const crypto::key::symmetric::Source type) const -> bool
{
    auto& encrypted = get_encrypted(lock);
    encrypted = std::make_unique<proto::Ciphertext>();

    OT_ASSERT(encrypted);

    encrypted->set_mode(translate(engine_.DefaultMode()));
    auto blankIV = api_.Factory().Secret(0);
    blankIV->Randomize(engine_.IvSize(translate(encrypted->mode())));
    encrypted->set_iv(blankIV->data(), blankIV->size());
    encrypted->set_text(false);
    auto key = api_.Factory().Secret(0);
    get_password(lock, reason, key);
    const auto saltSize = engine_.SaltSize(type);

    if (salt_.size() != saltSize) {
        if (!Allocate(api_, saltSize, salt_, true)) { return false; }
    }

    auto secondaryKey = Symmetric{
        api_,
        engine_,
        key,
        reader(salt_),
        engine_.KeySize(translate(encrypted->mode())),
        OT_SYMMETRIC_KEY_DEFAULT_OPERATIONS,
        OT_SYMMETRIC_KEY_DEFAULT_DIFFICULTY,
        OT_SYMMETRIC_KEY_DEFAULT_THREADS};

    OT_ASSERT(secondaryKey.plaintext_key_.has_value());

    return engine_.Encrypt(
        reinterpret_cast<const std::uint8_t*>(plaintextKey.data()),
        plaintextKey.size(),
        reinterpret_cast<const std::uint8_t*>(
            secondaryKey.plaintext_key_.value()->data()),
        secondaryKey.plaintext_key_.value()->size(),
        *encrypted);
}

auto Symmetric::get_encrypted(const Lock& lock) const
    -> std::unique_ptr<proto::Ciphertext>&
{
    return encrypted_key_;
}

auto Symmetric::get_password(
    const Lock& lock,
    const opentxs::PasswordPrompt& reason,
    Secret& password) const -> bool
{
    if (false == reason.Password().empty()) {
        password.Assign(reason.Password());

        return true;
    } else {
        auto buffer = api_.Factory().Secret(0);
        buffer->Randomize(1024);
        auto* callback = api_.Internal().GetInternalPasswordCallback();

        OT_ASSERT(nullptr != callback);

        auto bytes = buffer->Bytes();

        OT_ASSERT(std::numeric_limits<int>::max() >= bytes.size());

        const auto length = (*callback)(
            const_cast<char*>(bytes.data()),
            static_cast<int>(bytes.size()),
            0,
            const_cast<PasswordPrompt*>(&reason));
        bool result = false;

        if (0 < length) {
            password.Assign(bytes.data(), static_cast<std::size_t>(length));
            result = true;
        } else {
            LogError()(OT_PRETTY_CLASS())("Failed to obtain master password")
                .Flush();
        }

        return result;
    }
}

auto Symmetric::get_plaintext(const Lock& lock) const
    -> std::optional<OTSecret>&
{
    return plaintext_key_;
}

auto Symmetric::ID(const opentxs::PasswordPrompt& reason) const -> OTIdentifier
{
    auto lock = Lock{lock_};
    auto& plain = get_plaintext(lock);

    if (false == plain.has_value()) {
        if (false == unlock(lock, reason)) {
            LogError()(OT_PRETTY_CLASS())("Unable to unlock master key.")
                .Flush();

            return api_.Factory().Identifier();
        }
    }

    OT_ASSERT(plain.has_value());

    return api_.Factory().Identifier(plain.value()->Bytes());
}

auto Symmetric::RawKey(const opentxs::PasswordPrompt& reason, Secret& output)
    const -> bool
{
    auto lock = Lock{lock_};
    auto& plain = get_plaintext(lock);

    if (false == plain.has_value()) {
        if (false == unlock(lock, reason)) {
            LogError()(OT_PRETTY_CLASS())("Unable to unlock master key.")
                .Flush();

            return false;
        }
    }

    OT_ASSERT(plain.has_value());

    output.Assign(plain.value());

    return true;
}

auto Symmetric::serialize(const Lock& lock, proto::SymmetricKey& output) const
    -> bool
{
    auto& encrypted = get_encrypted(lock);

    if (!encrypted) { return false; }

    OT_ASSERT(std::numeric_limits<std::uint32_t>::max() >= key_size_);

    output.set_version(version_);
    output.set_type(translate(type_));
    output.set_size(static_cast<std::uint32_t>(key_size_));
    *output.mutable_key() = *encrypted;

    if (0u < salt_.size()) {
        const auto view = reader(salt_);

        output.set_salt(view.data(), view.size());
    }

    output.set_operations(operations_);
    output.set_difficulty(difficulty_);
    output.set_parallel(parallel_);

    return proto::Validate(output, VERBOSE);
}

auto Symmetric::Serialize(proto::SymmetricKey& output) const -> bool
{
    auto lock = Lock{lock_};

    return serialize(lock, output);
}

auto Symmetric::unlock(const Lock& lock, const opentxs::PasswordPrompt& reason)
    const -> bool
{
    auto output{false};
    auto& encrypted = get_encrypted(lock);
    auto& plain = get_plaintext(lock);
    auto post = ScopeGuard{[&] {
        if (false == output) { plain = std::nullopt; }
    }};

    if (false == bool(encrypted)) {
        LogError()(OT_PRETTY_CLASS())("Master key not loaded.").Flush();

        return output;
    }

    if (plain.has_value()) {
        if (0 < plain.value()->Bytes().size()) {
            LogDetail()(OT_PRETTY_CLASS())("Already unlocked").Flush();
            output = true;

            return output;
        }
    } else {
        plain = api_.Factory().Secret(0);

        OT_ASSERT(plain.has_value());

        // Allocate space for plaintext (same size as ciphertext)
        if (!allocate(lock, encrypted->data().size(), plain.value())) {
            LogError()(OT_PRETTY_CLASS())(
                "Unable to allocate space for plaintext master key.")
                .Flush();

            return output;
        }
    }

    OT_ASSERT(plain.has_value());

    auto key = api_.Factory().Secret(0);

    if (false == get_password(lock, reason, key)) {
        LogError()(OT_PRETTY_CLASS())("Unable to obtain master password.")
            .Flush();

        return output;
    }

    auto secondaryKey = Symmetric{
        api_,
        engine_,
        key,
        reader(salt_),
        engine_.KeySize(translate(encrypted->mode())),
        OT_SYMMETRIC_KEY_DEFAULT_OPERATIONS,
        OT_SYMMETRIC_KEY_DEFAULT_DIFFICULTY,
        OT_SYMMETRIC_KEY_DEFAULT_THREADS};

    OT_ASSERT(secondaryKey.plaintext_key_.has_value());

    output = engine_.Decrypt(
        *encrypted,
        reinterpret_cast<const std::uint8_t*>(
            secondaryKey.plaintext_key_.value()->data()),
        secondaryKey.plaintext_key_.value()->size(),
        reinterpret_cast<std::uint8_t*>(plain.value()->data()));

    if (output) {
        LogDetail()(OT_PRETTY_CLASS())("Key unlocked").Flush();
    } else {
        LogDetail()(OT_PRETTY_CLASS())("Failed to unlock key").Flush();
    }

    return output;
}

auto Symmetric::Unlock(const opentxs::PasswordPrompt& reason) const -> bool
{
    auto lock = Lock{lock_};

    return unlock(lock, reason);
}
}  // namespace opentxs::crypto::key::implementation
