// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"     // IWYU pragma: associated
#include "1_Internal.hpp"   // IWYU pragma: associated
#include "crypto/Seed.hpp"  // IWYU pragma: associated

#include <robin_hood.h>
#include <algorithm>
#include <cstddef>
#include <mutex>
#include <stdexcept>
#include <utility>

#include "internal/api/crypto/Symmetric.hpp"
#include "internal/crypto/key/Key.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/crypto/Symmetric.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Storage.hpp"
#include "opentxs/core/Secret.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/crypto/Bip32.hpp"
#include "opentxs/crypto/Bip39.hpp"
#include "opentxs/crypto/Language.hpp"
#include "opentxs/crypto/SeedStrength.hpp"
#include "opentxs/crypto/SeedStyle.hpp"
#include "opentxs/crypto/key/Symmetric.hpp"
#include "opentxs/crypto/key/symmetric/Algorithm.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Numbers.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "serialization/protobuf/Ciphertext.pb.h"
#include "serialization/protobuf/Enums.pb.h"
#include "serialization/protobuf/Seed.pb.h"
#include "util/Container.hpp"

namespace opentxs::crypto
{
struct Seed::Imp {
    const SeedStyle type_;
    const Language lang_;
    const OTSecret entropy_;
    const OTSecret words_;
    const OTSecret phrase_;
    const OTIdentifier id_;

    static auto translate(const proto::SeedType in) noexcept -> SeedStyle
    {
        static const auto map =
            reverse_arbitrary_map<SeedStyle, proto::SeedType, TypeReverseMap>(
                type_map());

        try {

            return map.at(in);
        } catch (...) {

            return SeedStyle::Error;
        }
    }

    auto Index() const noexcept -> Bip32Index
    {
        auto lock = Lock{lock_};

        return index_;
    }

    auto IncrementIndex(const Bip32Index index) noexcept -> bool
    {
        auto lock = Lock{lock_};

        if (index_ > index) {
            LogError()(OT_PRETTY_CLASS())("Index values must always increase.")
                .Flush();

            return false;
        }

        index_ = index;
        version_ = std::max(version_, default_version_);

        return save(lock);
    }

    Imp(const opentxs::crypto::Bip32& bip32,
        const opentxs::crypto::Bip39& bip39,
        const api::crypto::Symmetric& symmetric,
        const api::session::Factory& factory,
        const api::session::Storage& storage,
        const Language lang,
        const SeedStrength strength,
        const PasswordPrompt& reason) noexcept(false)
        : type_(SeedStyle::BIP39)
        , lang_(lang)
        , entropy_([&] {
            auto out = factory.Secret(0);
            static constexpr auto bitsPerByte{8u};
            const auto bytes = static_cast<std::size_t>(strength) / bitsPerByte;

            if ((16u > bytes) || (32u < bytes)) {
                throw std::runtime_error{"Invalid seed strength"};
            }

            out->Randomize(static_cast<std::size_t>(strength) / bitsPerByte);

            return out;
        }())
        , words_([&] {
            auto out = factory.Secret(0);

            if (false == bip39.SeedToWords(entropy_, out, lang_)) {
                throw std::runtime_error{
                    "Unable to convert entropy to word list"};
            }

            return out;
        }())
        , phrase_([&] {
            auto out = factory.Secret(0);
            out->AssignText(no_passphrase_);

            return out;
        }())
        , id_(bip32.SeedID(entropy_->Bytes()))
        , storage_(storage)
        , lock_()
        , encrypted_words_()
        , encrypted_phrase_()
        , encrypted_entropy_(encrypt(
              type_,
              symmetric,
              entropy_,
              words_,
              phrase_,
              const_cast<proto::Ciphertext&>(encrypted_words_),
              const_cast<proto::Ciphertext&>(encrypted_phrase_),
              reason))
        , version_(default_version_)
        , index_(0)
    {
        if (16u > entropy_->size()) {
            throw std::runtime_error{"Entropy too short"};
        }

        if (64u < entropy_->size()) {
            throw std::runtime_error{"Entropy too long"};
        }

        auto lock = Lock{lock_};

        if (false == save(lock)) {
            throw std::runtime_error{"Failed to save seed"};
        }
    }
    Imp(const api::Session& api,
        const opentxs::crypto::Bip32& bip32,
        const opentxs::crypto::Bip39& bip39,
        const api::crypto::Symmetric& symmetric,
        const api::session::Factory& factory,
        const api::session::Storage& storage,
        const SeedStyle type,
        const Language lang,
        const Secret& words,
        const Secret& passphrase,
        const PasswordPrompt& reason) noexcept(false)
        : type_(type)
        , lang_(lang)
        , entropy_([&] {
            auto out = factory.Secret(0);

            if (false ==
                bip39.WordsToSeed(api, type, lang_, words, out, passphrase)) {
                throw std::runtime_error{"Failed to calculate entropy"};
            }

            return out;
        }())
        , words_(words)
        , phrase_(passphrase)
        , id_(bip32.SeedID(entropy_->Bytes()))
        , storage_(storage)
        , lock_()
        , encrypted_words_()
        , encrypted_phrase_()
        , encrypted_entropy_(encrypt(
              type_,
              symmetric,
              entropy_,
              words_,
              phrase_,
              const_cast<proto::Ciphertext&>(encrypted_words_),
              const_cast<proto::Ciphertext&>(encrypted_phrase_),
              reason))
        , version_(default_version_)
        , index_(0)
    {
        if (16u > entropy_->size()) {
            throw std::runtime_error{"Entropy too short"};
        }

        if (64u < entropy_->size()) {
            throw std::runtime_error{"Entropy too long"};
        }

        auto lock = Lock{lock_};

        if (false == save(lock)) {
            throw std::runtime_error{"Failed to save seed"};
        }
    }
    Imp(const opentxs::crypto::Bip32& bip32,
        const opentxs::crypto::Bip39& bip39,
        const api::crypto::Symmetric& symmetric,
        const api::session::Factory& factory,
        const api::session::Storage& storage,
        const Secret& entropy,
        const PasswordPrompt& reason) noexcept(false)
        : type_(SeedStyle::BIP32)
        , lang_(Language::none)
        , entropy_(entropy)
        , words_(factory.Secret(0))
        , phrase_(factory.Secret(0))
        , id_(bip32.SeedID(entropy_->Bytes()))
        , storage_(storage)
        , lock_()
        , encrypted_words_()
        , encrypted_phrase_()
        , encrypted_entropy_(encrypt(
              type_,
              symmetric,
              entropy_,
              words_,
              phrase_,
              const_cast<proto::Ciphertext&>(encrypted_words_),
              const_cast<proto::Ciphertext&>(encrypted_phrase_),
              reason))
        , version_(default_version_)
        , index_(0)
    {
        if (16u > entropy_->size()) {
            throw std::runtime_error{"Entropy too short"};
        }

        if (64u < entropy_->size()) {
            throw std::runtime_error{"Entropy too long"};
        }

        auto lock = Lock{lock_};

        if (false == save(lock)) {
            throw std::runtime_error{"Failed to save seed"};
        }
    }
    Imp(const api::Session& api,
        const opentxs::crypto::Bip39& bip39,
        const api::crypto::Symmetric& symmetric,
        const api::session::Factory& factory,
        const api::session::Storage& storage,
        const proto::Seed& proto,
        const PasswordPrompt& reason) noexcept(false)
        : type_(translate(proto.type()))
        , lang_(translate(proto.lang()))
        , entropy_(factory.Secret(0))
        , words_(factory.Secret(0))
        , phrase_(factory.Secret(0))
        , id_(factory.IdentifierFromBase58(proto.fingerprint()))
        , storage_(storage)
        , lock_()
        , encrypted_words_(
              proto.has_words() ? proto.words() : proto::Ciphertext{})
        , encrypted_phrase_(
              proto.has_passphrase() ? proto.passphrase() : proto::Ciphertext{})
        , encrypted_entropy_(
              proto.has_raw() ? proto.raw() : proto::Ciphertext{})
        , version_(proto.version())
        , index_(proto.index())
    {
        const auto& session =
            (3 > version_) ? encrypted_words_ : encrypted_entropy_;
        const auto key = symmetric.InternalSymmetric().Key(
            session.key(), opentxs::translate(session.mode()));

        if (false == key.get()) {
            throw std::runtime_error{"Failed to get decryption key"};
        }

        if (proto.has_words()) {
            auto& words = const_cast<Secret&>(words_.get());
            const auto rc = key->Decrypt(
                encrypted_words_, reason, words.WriteInto(Secret::Mode::Text));

            if (false == rc) {
                throw std::runtime_error{"Failed to decrypt words"};
            }
        }

        if (proto.has_passphrase()) {
            auto& phrase = const_cast<Secret&>(phrase_.get());
            const auto rc = key->Decrypt(
                encrypted_phrase_,
                reason,
                phrase.WriteInto(Secret::Mode::Text));

            if (false == rc) {
                throw std::runtime_error{"Failed to decrypt passphrase"};
            }
        }

        if (proto.has_raw()) {
            auto& entropy = const_cast<Secret&>(entropy_.get());
            const auto rc = key->Decrypt(
                encrypted_entropy_,
                reason,
                entropy.WriteInto(Secret::Mode::Text));

            if (false == rc) {
                throw std::runtime_error{"Failed to decrypt entropy"};
            }
        } else {
            OT_ASSERT(proto.has_words());

            auto& entropy = const_cast<Secret&>(entropy_.get());

            if (false == bip39.WordsToSeed(
                             api, type_, lang_, words_, entropy, phrase_)) {
                throw std::runtime_error{"Failed to calculate entropy"};
            }

            auto ctext = const_cast<proto::Ciphertext&>(encrypted_entropy_);
            auto cwords = const_cast<proto::Ciphertext&>(encrypted_words_);

            if (!key->Encrypt(entropy_->Bytes(), reason, ctext, true)) {
                throw std::runtime_error{"Failed to encrypt entropy"};
            }

            if (!key->Encrypt(words_->Bytes(), reason, cwords, false)) {
                throw std::runtime_error{"Failed to encrypt words"};
            }

            version_ = default_version_;
        }
    }

private:
    using SerializeType = proto::Seed;
    using TypeMap = robin_hood::unordered_flat_map<SeedStyle, proto::SeedType>;
    using TypeReverseMap =
        robin_hood::unordered_flat_map<proto::SeedType, SeedStyle>;
    using LangMap = robin_hood::unordered_flat_map<Language, proto::SeedLang>;
    using LangReverseMap =
        robin_hood::unordered_flat_map<proto::SeedLang, Language>;

    static constexpr auto default_version_ = VersionNumber{4u};
    static constexpr auto no_passphrase_{""};

    const api::session::Storage& storage_;
    mutable std::mutex lock_;
    const proto::Ciphertext encrypted_words_;
    const proto::Ciphertext encrypted_phrase_;
    const proto::Ciphertext encrypted_entropy_;
    VersionNumber version_;
    Bip32Index index_;

    static auto encrypt(
        const SeedStyle type,
        const api::crypto::Symmetric& symmetric,
        const Secret& entropy,
        const Secret& words,
        const Secret& phrase,
        proto::Ciphertext& cwords,
        proto::Ciphertext& cphrase,
        const PasswordPrompt& reason) noexcept(false) -> proto::Ciphertext
    {
        auto key = symmetric.Key(
            reason, crypto::key::symmetric::Algorithm::ChaCha20Poly1305);

        if (false == key.get()) {
            throw std::runtime_error{"Failed to get encryption key"};
        }

        if (0u < words.size()) {
            if (!key->Encrypt(words.Bytes(), reason, cwords, false)) {
                throw std::runtime_error{"Failed to encrypt words"};
            }
        }

        if (0u < phrase.size()) {
            if (!key->Encrypt(phrase.Bytes(), reason, cphrase, false)) {
                throw std::runtime_error{"Failed to encrypt phrase"};
            }
        }

        auto out = proto::Ciphertext{};

        if (!key->Encrypt(entropy.Bytes(), reason, out, true)) {
            throw std::runtime_error{"Failed to encrypt entropy"};
        }

        return out;
    }
    static auto lang_map() noexcept -> const LangMap&
    {
        static const auto map = LangMap{
            {Language::none, proto::SEEDLANG_NONE},
            {Language::en, proto::SEEDLANG_EN},
        };

        return map;
    }
    static auto translate(const SeedStyle in) noexcept -> proto::SeedType
    {
        try {

            return type_map().at(in);
        } catch (...) {

            return proto::SEEDTYPE_ERROR;
        }
    }
    static auto translate(const Language in) noexcept -> proto::SeedLang
    {
        try {

            return lang_map().at(in);
        } catch (...) {

            return proto::SEEDLANG_NONE;
        }
    }
    static auto translate(const proto::SeedLang in) noexcept -> Language
    {
        static const auto map =
            reverse_arbitrary_map<Language, proto::SeedLang, LangReverseMap>(
                lang_map());

        try {

            return map.at(in);
        } catch (...) {

            return Language::none;
        }
    }
    static auto type_map() noexcept -> const TypeMap&
    {
        static const auto map = TypeMap{
            {SeedStyle::BIP32, proto::SEEDTYPE_RAW},
            {SeedStyle::BIP39, proto::SEEDTYPE_BIP39},
            {SeedStyle::PKT, proto::SEEDTYPE_PKT},
        };

        return map;
    }

    auto save(const Lock& lock) const noexcept -> bool
    {
        const auto id = id_->str();
        auto proto = SerializeType{};
        proto.set_version(version_);
        proto.set_index(index_);
        proto.set_fingerprint(id);
        proto.set_type(translate(type_));
        proto.set_lang(translate(lang_));

        if (0u < words_->size()) { *proto.mutable_words() = encrypted_words_; }

        if (0u < phrase_->size()) {
            *proto.mutable_passphrase() = encrypted_phrase_;
        }

        *proto.mutable_raw() = encrypted_entropy_;

        if (false == storage_.Store(proto, id)) {
            LogError()(OT_PRETTY_CLASS())("Failed to store seed.").Flush();

            return false;
        }

        return true;
    }
};

Seed::Seed(
    const opentxs::crypto::Bip32& bip32,
    const opentxs::crypto::Bip39& bip39,
    const api::crypto::Symmetric& symmetric,
    const api::session::Factory& factory,
    const api::session::Storage& storage,
    const Language lang,
    const SeedStrength strength,
    const PasswordPrompt& reason) noexcept(false)
    : imp_(std::make_unique<Imp>(
          bip32,
          bip39,
          symmetric,
          factory,
          storage,
          lang,
          strength,
          reason))
{
    OT_ASSERT(imp_);
}

Seed::Seed(
    const api::Session& api,
    const opentxs::crypto::Bip32& bip32,
    const opentxs::crypto::Bip39& bip39,
    const api::crypto::Symmetric& symmetric,
    const api::session::Factory& factory,
    const api::session::Storage& storage,
    const SeedStyle type,
    const Language lang,
    const Secret& words,
    const Secret& passphrase,
    const PasswordPrompt& reason) noexcept(false)
    : imp_(std::make_unique<Imp>(
          api,
          bip32,
          bip39,
          symmetric,
          factory,
          storage,
          type,
          lang,
          words,
          passphrase,
          reason))
{
    OT_ASSERT(imp_);
}

Seed::Seed(
    const opentxs::crypto::Bip32& bip32,
    const opentxs::crypto::Bip39& bip39,
    const api::crypto::Symmetric& symmetric,
    const api::session::Factory& factory,
    const api::session::Storage& storage,
    const Secret& entropy,
    const PasswordPrompt& reason) noexcept(false)
    : imp_(std::make_unique<
           Imp>(bip32, bip39, symmetric, factory, storage, entropy, reason))
{
    OT_ASSERT(imp_);
}

Seed::Seed(
    const api::Session& api,
    const opentxs::crypto::Bip39& bip39,
    const api::crypto::Symmetric& symmetric,
    const api::session::Factory& factory,
    const api::session::Storage& storage,
    const proto::Seed& proto,
    const PasswordPrompt& reason) noexcept(false)
    : imp_(std::make_unique<
           Imp>(api, bip39, symmetric, factory, storage, proto, reason))
{
    OT_ASSERT(imp_);
}

Seed::Seed(Seed&& rhs) noexcept
    : imp_(std::move(rhs.imp_))
{
    OT_ASSERT(imp_);
}

auto Seed::Entropy() const noexcept -> const Secret& { return imp_->entropy_; }

auto Seed::ID() const noexcept -> const Identifier& { return imp_->id_; }

auto Seed::IncrementIndex(const Bip32Index index) noexcept -> bool
{
    return imp_->IncrementIndex(index);
}

auto Seed::Index() const noexcept -> Bip32Index { return imp_->Index(); }

auto Seed::Phrase() const noexcept -> const Secret& { return imp_->phrase_; }

auto Seed::Translate(const int proto) noexcept -> SeedStyle
{
    return Imp::translate(static_cast<proto::SeedType>(proto));
}

auto Seed::Type() const noexcept -> SeedStyle { return imp_->type_; }

auto Seed::Words() const noexcept -> const Secret& { return imp_->words_; }

Seed::~Seed() = default;
}  // namespace opentxs::crypto
