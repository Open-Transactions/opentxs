// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "crypto/symmetric/KeyPrivate.hpp"  // IWYU pragma: associated

#include <Ciphertext.pb.h>
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>
#include <memory>
#include <optional>
#include <stdexcept>
#include <utility>

#include "internal/api/session/Session.hpp"
#include "internal/crypto/key/Key.hpp"
#include "internal/crypto/library/SymmetricProvider.hpp"
#include "internal/serialization/protobuf/Check.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "internal/serialization/protobuf/Proto.tpp"
#include "internal/serialization/protobuf/verify/SymmetricKey.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "internal/util/PasswordPrompt.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/core/Secret.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/crypto/symmetric/Algorithm.hpp"  // IWYU pragma: keep
#include "opentxs/crypto/symmetric/Source.hpp"     // IWYU pragma: keep
#include "opentxs/crypto/symmetric/Types.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Numbers.hpp"
#include "opentxs/util/PasswordPrompt.hpp"
#include "opentxs/util/Types.hpp"
#include "opentxs/util/WriteBuffer.hpp"
#include "opentxs/util/Writer.hpp"
#include "util/ScopeGuard.hpp"

namespace opentxs::crypto::symmetric
{
KeyPrivate::KeyPrivate(allocator_type alloc) noexcept
    : Allocated(alloc)
{
}

auto KeyPrivate::API() const noexcept -> const api::Session& { OT_FAIL; }

auto KeyPrivate::ChangePassword(const Secret&, const PasswordPrompt&) noexcept
    -> bool
{
    return false;
}

auto KeyPrivate::clone(allocator_type alloc) const noexcept -> KeyPrivate*
{
    return pmr::default_construct<KeyPrivate>(alloc);
}

auto KeyPrivate::Decrypt(
    const proto::Ciphertext&,
    Writer&&,
    const PasswordPrompt&) const noexcept -> bool
{
    return false;
}

auto KeyPrivate::Decrypt(ReadView, Writer&&, const PasswordPrompt&)
    const noexcept -> bool
{
    return false;
}

auto KeyPrivate::Encrypt(
    ReadView,
    proto::Ciphertext&,
    const PasswordPrompt&,
    bool,
    ReadView) const noexcept -> bool
{
    return false;
}

auto KeyPrivate::Encrypt(
    ReadView,
    Algorithm,
    proto::Ciphertext&,
    const PasswordPrompt&,
    bool,
    ReadView) const noexcept -> bool
{
    return false;
}

auto KeyPrivate::Encrypt(
    ReadView,
    Writer&&,
    Algorithm,
    bool,
    ReadView,
    const PasswordPrompt&) const noexcept -> bool
{
    return false;
}

auto KeyPrivate::Encrypt(
    ReadView,
    Writer&&,
    bool,
    ReadView,
    const PasswordPrompt&) const noexcept -> bool
{
    return false;
}

auto KeyPrivate::ID(const PasswordPrompt&) const noexcept
    -> const identifier::Generic&
{
    static const auto blank = identifier::Generic{};

    return blank;
}

auto KeyPrivate::IsValid() const noexcept -> bool { return false; }

auto KeyPrivate::operator delete(
    KeyPrivate* ptr,
    std::destroying_delete_t) noexcept -> void
{
    std::invoke(ptr->get_deleter());
}

auto KeyPrivate::RawKey(Secret&, const PasswordPrompt&) const noexcept -> bool
{
    return false;
}

auto KeyPrivate::Serialize(proto::SymmetricKey&) const noexcept -> bool
{
    return false;
}

auto KeyPrivate::Unlock(const PasswordPrompt&) const noexcept -> bool
{
    return false;
}

KeyPrivate::~KeyPrivate() = default;
}  // namespace opentxs::crypto::symmetric

namespace opentxs::crypto::symmetric::implementation
{
constexpr auto default_source_{crypto::symmetric::Source::Argon2i};
constexpr auto default_algorithm_{
    crypto::symmetric::Algorithm::ChaCha20Poly1305};

Key::Data::Data(
    std::size_t keySize,
    ReadView salt,
    std::uint64_t operations,
    std::uint64_t difficulty,
    std::uint64_t parallel,
    std::optional<Secret> plaintextKey,
    std::unique_ptr<proto::Ciphertext> encryptedKey,
    std::optional<identifier::Generic> id,
    allocator_type alloc) noexcept
    : key_size_(keySize)
    , salt_(salt, alloc)
    , operations_(operations)
    , difficulty_(difficulty)
    , parallel_(parallel)
    , plaintext_key_(std::move(plaintextKey))
    , encrypted_key_(std::move(encryptedKey))
    , id_(std::move(id))
{
}

Key::Data::Data(const Data& rhs, allocator_type alloc) noexcept
    : Data(
          rhs.key_size_,
          rhs.salt_.Bytes(),
          rhs.operations_,
          rhs.difficulty_,
          rhs.parallel_,
          rhs.plaintext_key_,
          [&]() -> decltype(encrypted_key_) {
              if (rhs.encrypted_key_) {

                  return std::make_unique<proto::Ciphertext>(
                      *rhs.encrypted_key_);
              } else {

                  return {};
              }
          }(),
          rhs.id_,
          alloc)
{
}

Key::Data::~Data() = default;
}  // namespace opentxs::crypto::symmetric::implementation

namespace opentxs::crypto::symmetric::implementation
{
Key::Key(
    const api::Session& api,
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
    allocator_type alloc) noexcept
    : KeyPrivate(std::move(alloc))
    , api_(api)
    , engine_(engine)
    , version_(version)
    , type_(type)
    , data_(
          keySize,
          salt,
          operations,
          difficulty,
          parallel,
          std::move(plaintextKey),
          std::move(encryptedKey),
          std::nullopt,
          get_allocator())
{
}

Key::Key(
    const api::Session& api,
    const crypto::SymmetricProvider& engine,
    allocator_type alloc) noexcept
    : Key(api,
          engine,
          default_version_,
          crypto::symmetric::Source::Raw,
          0u,
          {},
          0u,
          0u,
          0u,
          std::nullopt,
          {},
          std::move(alloc))
{
}

Key::Key(
    const api::Session& api,
    const crypto::SymmetricProvider& engine,
    const proto::SymmetricKey& serialized,
    allocator_type alloc) noexcept
    : Key(api,
          engine,
          std::max(serialized.version(), default_version_),
          translate(serialized.type()),
          serialized.size(),
          serialized.salt(),
          serialized.operations(),
          serialized.difficulty(),
          serialized.parallel(),
          std::nullopt,
          std::make_unique<proto::Ciphertext>(serialized.key()),
          std::move(alloc))
{
}

Key::Key(
    const api::Session& api,
    const crypto::SymmetricProvider& engine,
    const opentxs::Secret& seed,
    ReadView salt,
    std::size_t size,
    std::uint64_t operations,
    std::uint64_t difficulty,
    std::uint64_t parallel,
    crypto::symmetric::Source type,
    allocator_type alloc) noexcept(false)
    : Key(api,
          engine,
          default_version_,
          type,
          size,
          salt,
          operations,
          difficulty,
          parallel,
          std::nullopt,
          {},
          std::move(alloc))
{
    auto handle = data_.lock();
    auto& data = *handle;

    if (0_uz == data.key_size_) {
        throw std::runtime_error{"invalid key size"};
    }

    if (0u == data.salt_.size()) { throw std::runtime_error{"invalid salt"}; }

    if (0u == data.operations_) {
        throw std::runtime_error{"invalid operations"};
    }

    if (0u == data.difficulty_) {
        throw std::runtime_error{"invalid difficulty"};
    }

    auto& plain = data.plaintext_key_.emplace(api_.Factory().Secret(0_uz));

    if (false == plain.resize(data.key_size_)) {
        throw std::runtime_error{"failed to allocate space for plaintext key"};
    }

    const auto bytes = seed.Bytes();
    const bool derived = engine.Derive(
        reinterpret_cast<const std::uint8_t*>(bytes.data()),
        bytes.size(),
        reinterpret_cast<const std::uint8_t*>(data.salt_.data()),
        data.salt_.size(),
        data.operations_,
        data.difficulty_,
        data.parallel_,
        type_,
        reinterpret_cast<std::uint8_t*>(plain.data()),
        plain.size());

    if (false == derived) { throw std::runtime_error{"failed to derive key"}; }
}

Key::Key(const Key& rhs, allocator_type alloc) noexcept
    : KeyPrivate(alloc)
    , api_(rhs.api_)
    , engine_(rhs.engine_)
    , version_(rhs.version_)
    , type_(rhs.type_)
    , data_(*rhs.data_.lock_shared())
{
}

auto Key::ChangePassword(
    const Secret& newPassword,
    const PasswordPrompt& reason) noexcept -> bool
{
    try {
        auto handle = data_.lock();
        auto& data = *handle;
        const auto& plain = unlock(data, reason);
        auto changed = api_.Factory().PasswordPrompt(reason);
        changed.Internal().SetPassword(newPassword);

        return encrypt_key(data, plain, changed);
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return false;
    }
}

auto Key::clone(allocator_type alloc) const noexcept -> KeyPrivate*
{
    return pmr::clone(this, {alloc});
}

auto Key::Decrypt(
    const proto::Ciphertext& ciphertext,
    Writer&& plaintext,
    const PasswordPrompt& reason) const noexcept -> bool
{
    try {

        return decrypt(*data_.lock(), ciphertext, std::move(plaintext), reason);
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return false;
    }
}

auto Key::Decrypt(
    ReadView ciphertext,
    Writer&& plaintext,
    const PasswordPrompt& reason) const noexcept -> bool
{
    try {

        return Decrypt(
            proto::Factory<proto::Ciphertext>(ciphertext),
            std::move(plaintext),
            reason);
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return false;
    }
}

auto Key::decrypt(
    Data& data,
    const proto::Ciphertext& ciphertext,
    Writer&& plaintext,
    const PasswordPrompt& reason) const noexcept(false) -> bool
{
    const auto& input = ciphertext.data();
    const auto bytes = input.size();
    auto buffer = plaintext.Reserve(bytes);

    if (false == buffer.IsValid(bytes)) {

        throw std::runtime_error{"failed to allocate output buffer"};
    }

    const auto& key = unlock(data, reason);

    return engine_.Decrypt(
        ciphertext,
        static_cast<const std::uint8_t*>(key.data()),
        key.size(),
        buffer.as<std::uint8_t>());
}

auto Key::Derive(
    crypto::symmetric::Algorithm mode,
    const PasswordPrompt& reason) noexcept(false) -> bool
{
    return derive(*data_.lock(), mode, reason);
}

auto Key::derive(
    Data& data,
    crypto::symmetric::Algorithm mode,
    const PasswordPrompt& reason) const noexcept(false) -> bool
{
    const auto& size = [&]() -> auto& {
        auto& out = data.key_size_;
        out = engine_.KeySize(mode);

        return out;
    }();
    auto& plain = data.plaintext_key_.emplace(api_.Factory().Secret(0_uz));

    if (false == plain.Randomize(size)) {
        LogError()(OT_PRETTY_CLASS())("failed to generate key").Flush();

        return false;
    }

    return encrypt_key(data, plain, reason);
}

auto Key::Encrypt(
    ReadView plaintext,
    proto::Ciphertext& ciphertext,
    const PasswordPrompt& reason,
    bool attachKey,
    ReadView iv) const noexcept -> bool
{
    try {

        return Encrypt(
            plaintext, default_algorithm_, ciphertext, reason, attachKey, iv);
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return false;
    }
}

auto Key::Encrypt(
    ReadView plaintext,
    Algorithm mode,
    proto::Ciphertext& ciphertext,
    const PasswordPrompt& reason,
    bool attachKey,
    ReadView iv) const noexcept -> bool
{
    try {
        encrypt(
            *data_.lock(), plaintext, mode, ciphertext, reason, attachKey, iv);

        return true;
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return false;
    }
}

auto Key::Encrypt(
    ReadView plaintext,
    Writer&& ciphertext,
    Algorithm mode,
    bool attachKey,
    ReadView iv,
    const PasswordPrompt& reason) const noexcept -> bool
{
    try {
        auto proto = proto::Ciphertext{};
        encrypt(*data_.lock(), plaintext, mode, proto, reason, attachKey, iv);

        return proto::write(proto, std::move(ciphertext));
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return false;
    }
}

auto Key::Encrypt(
    ReadView plaintext,
    Writer&& ciphertext,
    bool attachKey,
    ReadView iv,
    const PasswordPrompt& reason) const noexcept -> bool
{
    try {

        return Encrypt(
            plaintext,
            std::move(ciphertext),
            default_algorithm_,
            attachKey,
            iv,
            reason);
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return false;
    }
}

auto Key::encrypt(
    Data& data,
    ReadView plaintext,
    Algorithm mode,
    proto::Ciphertext& ciphertext,
    const PasswordPrompt& reason,
    bool attachKey,
    ReadView iv) const noexcept(false) -> void
{
    if (false == valid(plaintext)) {

        throw std::runtime_error{"invalid input"};
    }

    if (mode == opentxs::crypto::symmetric::Algorithm::Error) {
        mode = engine_.DefaultMode();
    }

    ciphertext.set_version(default_version_);
    ciphertext.set_mode(translate(mode));

    if (valid(iv)) {
        ciphertext.set_iv(iv.data(), iv.size());
    } else {
        const auto random = [&] {
            auto out = api_.Factory().Secret(0);
            const auto size = engine_.IvSize(translate(ciphertext.mode()));

            if (false == out.Randomize(size)) {

                throw std::runtime_error{"failed to calculate IV"};
            }

            return out;
        }();
        ciphertext.set_iv(random.data(), random.size());
    }

    ciphertext.set_is_payload(true);
    const auto& key = unlock(data, reason);
    const auto encrypted = engine_.Encrypt(
        reinterpret_cast<const std::uint8_t*>(plaintext.data()),
        plaintext.size(),
        reinterpret_cast<const std::uint8_t*>(key.data()),
        key.size(),
        ciphertext);

    if (false == encrypted) { throw std::runtime_error{"encryption failed"}; }

    if (attachKey) { serialize(data, *ciphertext.mutable_key()); }
}

auto Key::encrypt_key(
    Data& data,
    const opentxs::Secret& plaintextKey,
    const crypto::symmetric::Source type,
    const PasswordPrompt& reason) const noexcept(false) -> bool
{
    auto& encrypted = data.encrypted_key_;
    encrypted = std::make_unique<proto::Ciphertext>();

    OT_ASSERT(encrypted);

    encrypted->set_mode(translate(engine_.DefaultMode()));
    auto blankIV = api_.Factory().Secret(0);
    blankIV.Randomize(engine_.IvSize(translate(encrypted->mode())));
    encrypted->set_iv(blankIV.data(), blankIV.size());
    encrypted->set_is_payload(false);
    {
        const auto saltSize = engine_.SaltSize(type);
        auto& salt = data.salt_;

        if (salt.size() != saltSize) {
            if (false == salt.Randomize(saltSize)) {

                throw std::runtime_error{"failed to generate salt"};
            }
        }
    }
    const auto password =
        encrypted_password(data, translate(encrypted->mode()), reason);

    return engine_.Encrypt(
        reinterpret_cast<const std::uint8_t*>(plaintextKey.data()),
        plaintextKey.size(),
        reinterpret_cast<const std::uint8_t*>(password.data()),
        password.size(),
        *encrypted);
}

auto Key::encrypt_key(
    Data& data,
    const opentxs::Secret& plaintextKey,
    const PasswordPrompt& reason) const noexcept(false) -> bool
{
    return encrypt_key(data, plaintextKey, default_source_, reason);
}

auto Key::encrypted_password(
    const Data& data,
    symmetric::Algorithm mode,
    const PasswordPrompt& reason) const noexcept(false) -> Secret
{
    const auto password = [&] {
        auto out = api_.Factory().Secret(0_uz);
        get_password(reason, out);

        return out;
    }();
    const auto key =
        Key{api_,
            engine_,
            password,
            data.salt_.Bytes(),
            engine_.KeySize(mode),
            default_operations_,
            default_difficulty_,
            default_threads_,
            default_source_,
            get_allocator()};
    auto handle = key.data_.lock_shared();
    const auto& secondary = *handle;

    OT_ASSERT(secondary.plaintext_key_.has_value());

    return *secondary.plaintext_key_;
}

auto Key::get_password(const PasswordPrompt& reason, Secret& out) const
    noexcept(false) -> bool
{
    if (const auto& pw = reason.Internal().Password(); false == pw.empty()) {
        out.Assign(pw);

        return true;
    } else {
        auto buffer = api_.Factory().Secret(0);
        constexpr auto bufferBytes = 1024_uz;
        static_assert(std::numeric_limits<int>::max() >= bufferBytes);
        buffer.Randomize(bufferBytes);
        auto* callback = api_.Internal().GetInternalPasswordCallback();

        if (nullptr == callback) {
            throw std::runtime_error{"invalid password callback"};
        }

        auto bytes = buffer.Bytes();
        const auto length = (*callback)(
            const_cast<char*>(bytes.data()),
            static_cast<int>(bytes.size()),
            0,
            const_cast<PasswordPrompt*>(&reason));
        bool result = false;
        static_assert(sizeof(int) <= sizeof(std::size_t));

        if (0 < length) {
            out.Assign(bytes.data(), static_cast<std::size_t>(length));
            result = true;
        } else {
            LogError()(OT_PRETTY_CLASS())("Failed to obtain master password")
                .Flush();
        }

        return result;
    }
}

auto Key::ID(const PasswordPrompt& reason) const noexcept
    -> const identifier::Generic&
{
    try {

        return id(*data_.lock(), reason);
    } catch (const std::exception& e) {
        static const auto blank = identifier::Generic{};
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return blank;
    }
}

auto Key::id(Data& data, const PasswordPrompt& reason) const noexcept(false)
    -> const identifier::Generic&
{
    if (data.id_.has_value()) {

        return *data.id_;
    } else {
        const auto& plain = unlock(data, reason);

        return data.id_.emplace(
            api_.Factory().IdentifierFromPreimage(plain.Bytes()));
    }
}

auto Key::RawKey(Secret& output, const PasswordPrompt& reason) const noexcept
    -> bool
{
    try {

        return output.Assign(unlock(*data_.lock(), reason).Bytes());
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return false;
    }
}

auto Key::SetRawKey(
    const opentxs::Secret& raw,
    const PasswordPrompt& reason) noexcept(false) -> bool
{
    return set_raw_key(*data_.lock(), raw, reason);
}

auto Key::Serialize(proto::SymmetricKey& output) const noexcept -> bool
{
    try {

        return serialize(*data_.lock_shared(), output);
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return false;
    }
}

auto Key::serialize(const Data& data, proto::SymmetricKey& output) const
    noexcept(false) -> bool
{
    const auto& encrypted = data.encrypted_key_;

    if (false == encrypted.operator bool()) {

        throw std::runtime_error{"encrypted key missing"};
    }

    if constexpr (constexpr auto max =
                      std::numeric_limits<std::uint32_t>::max();
                  max < std::numeric_limits<std::size_t>::max()) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtautological-type-limit-compare"
        if (max < data.key_size_) { throw std::runtime_error{"key too large"}; }
#pragma GCC diagnostic pop
    }

    output.set_version(version_);
    output.set_type(translate(type_));
    output.set_size(static_cast<std::uint32_t>(data.key_size_));
    *output.mutable_key() = *encrypted;

    if (false == data.salt_.empty()) {
        const auto view = data.salt_.Bytes();
        output.set_salt(view.data(), view.size());
    }

    output.set_operations(data.operations_);
    output.set_difficulty(data.difficulty_);
    output.set_parallel(data.parallel_);

    return proto::Validate(output, VERBOSE);
}

auto Key::set_raw_key(
    Data& data,
    const opentxs::Secret& raw,
    const PasswordPrompt& reason) noexcept(false) -> bool
{
    const auto& plain = [&]() -> const auto& {
        if (auto& key = data.plaintext_key_; key.has_value()) {
            key->Assign(raw);

            return *key;
        } else {

            return key.emplace(raw);
        }
    }();
    data.key_size_ = plain.size();

    return encrypt_key(data, plain, reason);
}

auto Key::Unlock(const PasswordPrompt& reason) const noexcept -> bool
{
    try {
        unlock(*data_.lock(), reason);

        return true;
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return false;
    }
}

auto Key::unlock(Data& data, const PasswordPrompt& reason) const noexcept(false)
    -> const Secret&
{
    auto success{false};
    const auto post = ScopeGuard{[&] {
        if (false == success) { data.plaintext_key_.reset(); }
    }};
    auto& key = [&]() -> auto& {
        if (auto& plain = data.plaintext_key_; plain) {

            return *plain;
        } else {

            return plain.emplace(api_.Factory().Secret(0_uz));
        }
    }();

    if (0_uz == key.size()) {
        if (false == data.encrypted_key_.operator bool()) {

            throw std::runtime_error{"encrypted key missing"};
        }

        const auto& encrypted = *data.encrypted_key_;
        const auto& ciphertext = encrypted.data();

        if (0_uz == ciphertext.size()) {

            throw std::runtime_error{"empty key"};
        }

        if (false == key.resize(ciphertext.size())) {

            throw std::runtime_error{"failed to allocate space for plaintext"};
        }

        const auto password =
            encrypted_password(data, translate(encrypted.mode()), reason);
        success = engine_.Decrypt(
            encrypted,
            reinterpret_cast<const std::uint8_t*>(password.data()),
            password.size(),
            reinterpret_cast<std::uint8_t*>(key.data()));

        if (false == success) {

            throw std::runtime_error{"unable to decrypt key"};
        }
    } else {
        success = true;
    }

    return key;
}
}  // namespace opentxs::crypto::symmetric::implementation
