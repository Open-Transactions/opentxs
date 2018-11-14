// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/api/Native.hpp"
#include "opentxs/core/crypto/OTPassword.hpp"
#include "opentxs/core/crypto/OTPasswordData.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/crypto/key/EllipticCurve.hpp"
#include "opentxs/crypto/key/Symmetric.hpp"
#include "opentxs/crypto/library/LegacySymmetricProvider.hpp"
#include "opentxs/crypto/library/SymmetricProvider.hpp"
#include "opentxs/OT.hpp"
#include "opentxs/Proto.hpp"

#include "internal/api/Internal.hpp"
#include "SymmetricNull.hpp"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>

#include "Symmetric.hpp"

template class opentxs::Pimpl<opentxs::crypto::key::Symmetric>;

#define OT_SYMMETRIC_KEY_DEFAULT_OPERATIONS 3
#define OT_SYMMETRIC_KEY_DEFAULT_DIFFICULTY 8388608

#define OT_METHOD "opentxs::crypto::key::implementation::Symmetric::"

namespace opentxs::crypto::key
{
OTSymmetricKey Symmetric::Factory()
{
    return OTSymmetricKey{new implementation::SymmetricNull};
}

OTSymmetricKey Symmetric::Factory(
    const crypto::SymmetricProvider& engine,
    const OTPasswordData& password,
    const proto::SymmetricMode mode)
{
    std::unique_ptr<implementation::Symmetric> output;
    output.reset(new implementation::Symmetric(engine));

    if (!output) { return OTSymmetricKey(output.release()); }

    proto::SymmetricMode realMode = proto::SMODE_ERROR;

    if (mode == realMode) {
        realMode = engine.DefaultMode();
    } else {
        realMode = mode;
    }

    const auto size = output->engine_.KeySize(realMode);
    output->key_size_ = size;
    OTPassword key;

    if (!output->Allocate(size, key, false)) {
        return OTSymmetricKey(output.release());
    }

    output->EncryptKey(key, password);

    return OTSymmetricKey(output.release());
}

OTSymmetricKey Symmetric::Factory(
    const crypto::SymmetricProvider& engine,
    const proto::SymmetricKey serialized)
{
    std::unique_ptr<implementation::Symmetric> output;
    const bool valid = proto::Validate(serialized, VERBOSE);

    if (valid) {
        output.reset(new implementation::Symmetric(engine, serialized));
    }

    return OTSymmetricKey(output.release());
}

OTSymmetricKey Symmetric::Factory(
    const crypto::SymmetricProvider& engine,
    const OTPassword& seed,
    const std::uint64_t operations,
    const std::uint64_t difficulty,
    const std::size_t size,
    const proto::SymmetricKeyType type)
{
    std::unique_ptr<implementation::Symmetric> output;
    std::string salt{};
    implementation::Symmetric::Allocate(engine.SaltSize(type), salt, false);

    const std::uint64_t ops =
        (0 == operations) ? OT_SYMMETRIC_KEY_DEFAULT_OPERATIONS : operations;
    const std::uint64_t mem =
        (0 == difficulty) ? OT_SYMMETRIC_KEY_DEFAULT_DIFFICULTY : difficulty;

    output.reset(new implementation::Symmetric(
        engine, seed, salt, size, ops, mem, type));

    return OTSymmetricKey(output.release());
}

OTSymmetricKey Symmetric::Factory(
    const crypto::SymmetricProvider& engine,
    const OTPassword& raw)
{
    std::unique_ptr<implementation::Symmetric> output;

    if (!raw.isMemory()) { return OTSymmetricKey(output.release()); }

    output.reset(new implementation::Symmetric(engine));

    if (!output) { return OTSymmetricKey(output.release()); }

    OTPasswordData password("Encrypting a symmetric key.");
    output->EncryptKey(raw, password);
    output->key_size_ = raw.getMemorySize();

    return OTSymmetricKey(output.release());
}
}  // namespace opentxs::crypto::key

namespace opentxs::crypto::key::implementation
{
Symmetric::Symmetric(const crypto::SymmetricProvider& engine)
    : engine_(engine)
    , version_(1)
    , type_(proto::SKEYTYPE_RAW)
{
}

Symmetric::Symmetric(
    const crypto::SymmetricProvider& engine,
    const proto::SymmetricKey serialized)
    : engine_(engine)
    , version_(serialized.version())
    , type_(serialized.type())
    , key_size_(serialized.size())
    , salt_(new std::string(serialized.salt()))
    , operations_(serialized.operations())
    , difficulty_(serialized.difficulty())
    , encrypted_key_(new proto::Ciphertext(serialized.key()))
{
}

Symmetric::Symmetric(
    const crypto::SymmetricProvider& engine,
    const OTPassword& seed,
    const std::string& salt,
    const std::size_t size,
    const std::uint64_t operations,
    const std::uint64_t difficulty,
    const proto::SymmetricKeyType type)
    : engine_(engine)
    , version_(1)
    , type_(type)
    , key_size_(size)
    , salt_(new std::string(salt))
    , operations_(operations)
    , difficulty_(difficulty)
    , plaintext_key_(new OTPassword)
{
    OT_ASSERT(salt_);
    OT_ASSERT(plaintext_key_);
    OT_ASSERT(0 != operations);
    OT_ASSERT(0 != difficulty);
    OT_ASSERT(0 != size);

    Allocate(key_size_, *plaintext_key_, false);

    const std::uint8_t* input = nullptr;
    std::size_t inputSize = 0;

    if (seed.isMemory()) {
        input = static_cast<const std::uint8_t*>(seed.getMemory());
        inputSize = seed.getMemorySize();
    } else {
        input = reinterpret_cast<const std::uint8_t*>(seed.getPassword());
        inputSize = seed.getPasswordSize();
    }

    const bool derived = engine.Derive(
        input,
        inputSize,
        reinterpret_cast<const std::uint8_t*>(salt_->data()),
        salt_->size(),
        operations_,
        difficulty_,
        type_,
        static_cast<std::uint8_t*>(plaintext_key_->getMemoryWritable()),
        plaintext_key_->getMemorySize());

    OT_ASSERT(derived);
}

Symmetric::Symmetric(const Symmetric& rhs)
    : key::Symmetric()
    , engine_{rhs.engine_}
    , version_{rhs.version_}
    , type_{rhs.type_}
    , key_size_{rhs.key_size_}
    , salt_{nullptr}
    , operations_{rhs.operations_}
    , difficulty_{rhs.difficulty_}
    , plaintext_key_{nullptr}
    , encrypted_key_{nullptr}
{
    if (rhs.salt_) { salt_.reset(new std::string(*rhs.salt_)); }

    if (rhs.plaintext_key_) {
        plaintext_key_.reset(new OTPassword(*rhs.plaintext_key_));
    }

    if (rhs.encrypted_key_) {
        encrypted_key_.reset(new proto::Ciphertext(*rhs.encrypted_key_));
    }
}

bool Symmetric::Allocate(const std::size_t size, Data& container)
{
    container.SetSize(size);

    return (size == container.size());
}

bool Symmetric::Allocate(
    const std::size_t size,
    std::string& container,
    const bool random)
{
    container.resize(size, 0x0);

    if (random) {
        OTPassword::randomizeMemory(
            static_cast<void*>(const_cast<char*>(container.data())),
            container.size());
    }

    return (size == container.size());
}

bool Symmetric::Allocate(
    const std::size_t size,
    OTPassword& container,
    const bool text)
{
    std::int32_t result;

    if (text) {
        result = container.randomizePassword(size);
    } else {
        result = container.randomizeMemory(size);
    }

    if (0 > result) { return false; }

    return (size == static_cast<std::uint32_t>(result));
}

bool Symmetric::ChangePassword(
    const OTPasswordData& oldPassword,
    const OTPassword& newPassword)
{
    if (Unlock(oldPassword)) {
        OTPasswordData password("");
        password.SetOverride(newPassword);

        return EncryptKey(*plaintext_key_, password);
    }

    LogOutput(OT_METHOD)(__FUNCTION__)(": Unable to unlock master key.")
        .Flush();

    return false;
}

Symmetric* Symmetric::clone() const { return new Symmetric(*this); }

bool Symmetric::Decrypt(
    const proto::Ciphertext& input,
    const OTPasswordData& keyPassword,
    std::uint8_t* plaintext)
{
    if (false == bool(plaintext_key_)) {
        if (false == Unlock(keyPassword)) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Unable to unlock master key.")
                .Flush();

            return false;
        }
    }

    const bool output = engine_.Decrypt(
        input,
        plaintext_key_->getMemory_uint8(),
        plaintext_key_->getMemorySize(),
        plaintext);

    return output;
}

bool Symmetric::Decrypt(
    const proto::Ciphertext& ciphertext,
    const OTPasswordData& keyPassword,
    std::string& plaintext)
{
    if (false == Allocate(ciphertext.data().size(), plaintext, true)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Unable to allocate space for decryption.")
            .Flush();

        return false;
    }

    return (Decrypt(
        ciphertext,
        keyPassword,
        reinterpret_cast<std::uint8_t*>(const_cast<char*>(plaintext.data()))));
}

bool Symmetric::Decrypt(
    const proto::Ciphertext& ciphertext,
    const OTPasswordData& keyPassword,
    Data& plaintext)
{
    if (false == Allocate(ciphertext.data().size(), plaintext)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Unable to allocate space for decryption.")
            .Flush();

        return false;
    }

    return (Decrypt(
        ciphertext,
        keyPassword,
        static_cast<std::uint8_t*>(const_cast<void*>(plaintext.data()))));
}

bool Symmetric::Decrypt(
    const proto::Ciphertext& ciphertext,
    const OTPasswordData& keyPassword,
    OTPassword& plaintext)
{
    if (!Allocate(ciphertext.data().size(), plaintext, ciphertext.text())) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Unable to allocate space for decryption.")
            .Flush();

        return false;
    }

    std::uint8_t* output = nullptr;

    if (ciphertext.text()) {
        output = plaintext.getPasswordWritable();
    } else {
        output = static_cast<std::uint8_t*>(plaintext.getMemoryWritable());
    }

    return (Decrypt(ciphertext, keyPassword, output));
}

bool Symmetric::Encrypt(
    const std::uint8_t* input,
    const std::size_t inputSize,
    const std::uint8_t* iv,
    const std::size_t ivSize,
    const proto::SymmetricMode mode,
    const OTPasswordData& keyPassword,
    proto::Ciphertext& ciphertext,
    const bool text)
{
    if (nullptr == input) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Null input.").Flush();

        return false;
    }

    if (false == bool(plaintext_key_)) {
        if (false == Unlock(keyPassword)) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Unable to unlock master key.")
                .Flush();

            return false;
        }
    }

    ciphertext.set_version(1);

    if (proto::SMODE_ERROR == mode) {
        ciphertext.set_mode(engine_.DefaultMode());
    } else {
        ciphertext.set_mode(mode);
    }

    if ((0 == ivSize) || (nullptr == iv)) {
        OTPassword blankIV;
        blankIV.randomizeMemory(engine_.IvSize(ciphertext.mode()));
        ciphertext.set_iv(blankIV.getMemory(), blankIV.getMemorySize());
    } else {
        ciphertext.set_iv(iv, ivSize);
    }

    ciphertext.set_text(text);

    OT_ASSERT(nullptr != plaintext_key_->getMemory_uint8());

    return engine_.Encrypt(
        input,
        inputSize,
        plaintext_key_->getMemory_uint8(),
        plaintext_key_->getMemorySize(),
        ciphertext);
}

bool Symmetric::Encrypt(
    const OTPassword& plaintext,
    const Data& iv,
    const OTPasswordData& keyPassword,
    proto::Ciphertext& ciphertext,
    const bool attachKey,
    const proto::SymmetricMode mode)
{
    const std::uint8_t* input = nullptr;
    std::size_t inputSize = 0;

    if (plaintext.isPassword()) {
        input = reinterpret_cast<const std::uint8_t*>(plaintext.getPassword());
        inputSize = plaintext.getPasswordSize();
    } else {
        input = static_cast<const std::uint8_t*>(plaintext.getMemory());
        inputSize = plaintext.getMemorySize();
    }

    const bool success = Encrypt(
        input,
        inputSize,
        static_cast<const std::uint8_t*>(iv.data()),
        iv.size(),
        mode,
        keyPassword,
        ciphertext,
        plaintext.isPassword());

    if (success && attachKey) { Serialize(*ciphertext.mutable_key()); }

    return success;
}

bool Symmetric::Encrypt(
    const String& plaintext,
    const Data& iv,
    const OTPasswordData& keyPassword,
    proto::Ciphertext& ciphertext,
    const bool attachKey,
    const proto::SymmetricMode mode)
{
    const bool success = Encrypt(
        reinterpret_cast<const std::uint8_t*>(plaintext.Get()),
        plaintext.GetLength() + 1,
        static_cast<const std::uint8_t*>(iv.data()),
        iv.size(),
        mode,
        keyPassword,
        ciphertext,
        true);

    if (success && attachKey) { Serialize(*ciphertext.mutable_key()); }

    return success;
}

bool Symmetric::Encrypt(
    const std::string& plaintext,
    const Data& iv,
    const OTPasswordData& keyPassword,
    proto::Ciphertext& ciphertext,
    const bool attachKey,
    const proto::SymmetricMode mode)
{
    const bool success = Encrypt(
        reinterpret_cast<const std::uint8_t*>(plaintext.c_str()),
        plaintext.size(),
        static_cast<const std::uint8_t*>(iv.data()),
        iv.size(),
        mode,
        keyPassword,
        ciphertext,
        true);

    if (success && attachKey) { Serialize(*ciphertext.mutable_key()); }

    return success;
}

bool Symmetric::EncryptKey(
    const OTPassword& plaintextKey,
    const OTPasswordData& keyPassword,
    const proto::SymmetricKeyType type)
{
    encrypted_key_.reset(new proto::Ciphertext);

    OT_ASSERT(encrypted_key_);

    encrypted_key_->set_mode(engine_.DefaultMode());
    OTPassword blankIV;
    blankIV.randomizeMemory(engine_.IvSize(encrypted_key_->mode()));
    encrypted_key_->set_iv(blankIV.getMemory(), blankIV.getMemorySize());
    encrypted_key_->set_text(false);
    OTPassword key;
    GetPassword(keyPassword, key);
    const auto saltSize = engine_.SaltSize(type);

    if (!salt_) { salt_.reset(new std::string); }

    OT_ASSERT(salt_);

    if (salt_->size() != saltSize) {
        if (!Allocate(saltSize, *salt_, true)) { return false; }
    }

    Symmetric secondaryKey(
        engine_,
        key,
        *salt_,
        engine_.KeySize(encrypted_key_->mode()),
        OT_SYMMETRIC_KEY_DEFAULT_OPERATIONS,
        OT_SYMMETRIC_KEY_DEFAULT_DIFFICULTY);

    return engine_.Encrypt(
        plaintextKey.getMemory_uint8(),
        plaintextKey.getMemorySize(),
        secondaryKey.plaintext_key_->getMemory_uint8(),
        secondaryKey.plaintext_key_->getMemorySize(),
        *encrypted_key_);
}

bool Symmetric::GetPassword(
    const OTPasswordData& keyPassword,
    OTPassword& password)
{
    if (keyPassword.Override()) {
        password = *keyPassword.Override();

        return true;
    } else {
        std::unique_ptr<OTPassword> master(new OTPassword);

        OT_ASSERT(master);

        master->randomizeMemory(master->getBlockSize());
        const auto& native =
            dynamic_cast<const api::internal::Native&>(OT::App());
        auto* callback = native.GetInternalPasswordCallback();
        const auto length = (*callback)(
            static_cast<char*>(master->getMemoryWritable()),
            master->getBlockSize(),
            0,
            reinterpret_cast<void*>(const_cast<OTPasswordData*>(&keyPassword)));
        bool result = false;

        if (0 < length) {
            password.setMemory(master->getMemory(), length);
            result = true;
        }

        return result;
    }
}

OTIdentifier Symmetric::ID()
{
    OTPasswordData keyPassword("");

    if (false == bool(plaintext_key_)) {
        if (false == Unlock(keyPassword)) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Unable to unlock master key.")
                .Flush();

            return Identifier::Factory();
        }
    }

    auto output = Identifier::Factory();
    output->CalculateDigest(Data::Factory(
        plaintext_key_->getMemory(), plaintext_key_->getMemorySize()));

    return output;
}

bool Symmetric::Serialize(proto::SymmetricKey& output) const
{
    output.set_version(version_);
    output.set_type(type_);
    output.set_size(key_size_);
    *output.mutable_key() = *encrypted_key_;

    if (salt_) { output.set_salt(*salt_); }

    output.set_operations(operations_);
    output.set_difficulty(difficulty_);

    if (!encrypted_key_) { return false; }

    return proto::Validate(output, VERBOSE);
}

bool Symmetric::Unlock(const OTPasswordData& keyPassword)
{
    if (false == bool(encrypted_key_)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Master key not loaded.").Flush();

        return false;
    }

    if (false == bool(plaintext_key_)) {
        plaintext_key_.reset(new OTPassword);

        OT_ASSERT(plaintext_key_);

        // Allocate space for plaintext (same size as ciphertext)
        if (!Allocate(encrypted_key_->data().size(), *plaintext_key_)) {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Unable to allocate space for plaintext master key.")
                .Flush();

            return false;
        }
    }

    OTPassword key;

    if (false == GetPassword(keyPassword, key)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Unable to obtain master password.")
            .Flush();

        return false;
    }

    Symmetric secondaryKey(
        engine_,
        key,
        *salt_,
        engine_.KeySize(encrypted_key_->mode()),
        OT_SYMMETRIC_KEY_DEFAULT_OPERATIONS,
        OT_SYMMETRIC_KEY_DEFAULT_DIFFICULTY);

    return engine_.Decrypt(
        *encrypted_key_,
        secondaryKey.plaintext_key_->getMemory_uint8(),
        secondaryKey.plaintext_key_->getMemorySize(),
        static_cast<std::uint8_t*>(plaintext_key_->getMemoryWritable()));
}
}  // namespace opentxs::crypto::key::implementation
