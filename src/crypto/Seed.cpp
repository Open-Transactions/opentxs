// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::crypto::SeedStrength

#include "0_stdafx.hpp"                 // IWYU pragma: associated
#include "crypto/Seed.hpp"              // IWYU pragma: associated
#include "internal/crypto/Factory.hpp"  // IWYU pragma: associated

#include <Ciphertext.pb.h>
#include <Enums.pb.h>
#include <Seed.pb.h>
#include <robin_hood.h>
#include <algorithm>
#include <compare>
#include <cstddef>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>

#include "internal/api/crypto/Symmetric.hpp"
#include "internal/crypto/key/Key.hpp"
#include "internal/crypto/symmetric/Key.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/OT.hpp"
#include "opentxs/api/Context.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/crypto/Symmetric.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Storage.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Secret.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/crypto/Bip32.hpp"
#include "opentxs/crypto/Bip39.hpp"
#include "opentxs/crypto/Language.hpp"
#include "opentxs/crypto/SeedStyle.hpp"
#include "opentxs/crypto/symmetric/Algorithm.hpp"
#include "opentxs/crypto/symmetric/Key.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Writer.hpp"
#include "util/Container.hpp"

namespace opentxs::factory
{
auto Seed(
    const api::Session& api,
    const crypto::Bip32& bip32,
    const crypto::Bip39& bip39,
    const api::crypto::Symmetric& symmetric,
    const api::session::Factory& factory,
    const api::session::Storage& storage,
    const crypto::Language lang,
    const crypto::SeedStrength strength,
    const opentxs::PasswordPrompt& reason) noexcept(false) -> crypto::Seed
{
    using ReturnType = opentxs::crypto::Seed::Imp;

    return std::make_unique<ReturnType>(
               api,
               bip32,
               bip39,
               symmetric,
               factory,
               storage,
               lang,
               strength,
               reason)
        .release();
}

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
    const opentxs::PasswordPrompt& reason) noexcept(false) -> crypto::Seed
{
    using ReturnType = opentxs::crypto::Seed::Imp;

    return std::make_unique<ReturnType>(
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
               reason)
        .release();
}

auto Seed(
    const crypto::Bip32& bip32,
    const crypto::Bip39& bip39,
    const api::crypto::Symmetric& symmetric,
    const api::session::Factory& factory,
    const api::session::Storage& storage,
    const opentxs::Secret& entropy,
    const opentxs::PasswordPrompt& reason) noexcept(false) -> crypto::Seed
{
    using ReturnType = opentxs::crypto::Seed::Imp;

    return std::make_unique<ReturnType>(
               bip32, bip39, symmetric, factory, storage, entropy, reason)
        .release();
}

auto Seed(
    const api::Session& api,
    const crypto::Bip39& bip39,
    const api::crypto::Symmetric& symmetric,
    const api::session::Factory& factory,
    const api::session::Storage& storage,
    const proto::Seed& proto,
    const opentxs::PasswordPrompt& reason) noexcept(false) -> crypto::Seed
{
    using ReturnType = opentxs::crypto::Seed::Imp;

    return std::make_unique<ReturnType>(
               api, bip39, symmetric, factory, storage, proto, reason)
        .release();
}
}  // namespace opentxs::factory

namespace opentxs::crypto::internal
{
using SeedTypeMap = robin_hood::unordered_flat_map<SeedStyle, proto::SeedType>;
using SeedTypeReverseMap =
    robin_hood::unordered_flat_map<proto::SeedType, SeedStyle>;
using SeedLangMap = robin_hood::unordered_flat_map<Language, proto::SeedLang>;
using SeedLangReverseMap =
    robin_hood::unordered_flat_map<proto::SeedLang, Language>;

static auto seed_lang_map() noexcept -> const SeedLangMap&
{
    static const auto map = SeedLangMap{
        {Language::none, proto::SEEDLANG_NONE},
        {Language::en, proto::SEEDLANG_EN},
    };

    return map;
}
static auto seed_type_map() noexcept -> const SeedTypeMap&
{
    static const auto map = SeedTypeMap{
        {SeedStyle::BIP32, proto::SEEDTYPE_RAW},
        {SeedStyle::BIP39, proto::SEEDTYPE_BIP39},
        {SeedStyle::PKT, proto::SEEDTYPE_PKT},
    };

    return map;
}
static auto translate(const SeedStyle in) noexcept -> proto::SeedType
{
    try {

        return seed_type_map().at(in);
    } catch (...) {

        return proto::SEEDTYPE_ERROR;
    }
}
static auto translate(const Language in) noexcept -> proto::SeedLang
{
    try {

        return seed_lang_map().at(in);
    } catch (...) {

        return proto::SEEDLANG_NONE;
    }
}
static auto translate(const proto::SeedLang in) noexcept -> Language
{
    static const auto map =
        reverse_arbitrary_map<Language, proto::SeedLang, SeedLangReverseMap>(
            seed_lang_map());

    try {

        return map.at(in);
    } catch (...) {

        return Language::none;
    }
}

static auto translate(const proto::SeedType in) noexcept -> SeedStyle
{
    static const auto map =
        reverse_arbitrary_map<SeedStyle, proto::SeedType, SeedTypeReverseMap>(
            seed_type_map());

    try {

        return map.at(in);
    } catch (...) {

        return SeedStyle::Error;
    }
}

auto Seed::Translate(const int proto) noexcept -> SeedStyle
{
    return translate(static_cast<proto::SeedType>(proto));
}
}  // namespace opentxs::crypto::internal

namespace opentxs::crypto
{
auto operator<(const Seed& lhs, const Seed& rhs) noexcept -> bool
{
    return lhs.ID() < rhs.ID();
}

auto operator==(const Seed& lhs, const Seed& rhs) noexcept -> bool
{
    return lhs.ID() == rhs.ID();
}

auto swap(Seed& lhs, Seed& rhs) noexcept -> void { lhs.swap(rhs); }
}  // namespace opentxs::crypto

namespace opentxs::crypto
{
Seed::Imp::Imp() noexcept
    : Imp(Context().Factory())
{
}

Seed::Imp::Imp(const api::Factory& factory) noexcept
    : type_(SeedStyle::Error)
    , lang_(Language::none)
    , words_(factory.Secret(0))
    , phrase_(factory.Secret(0))
    , entropy_(factory.Secret(0))
    , id_()
    , storage_(nullptr)
    , encrypted_words_()
    , encrypted_phrase_()
    , encrypted_entropy_()
    , data_(0, 0)
{
}

Seed::Imp::Imp(const Imp& rhs) noexcept
    : type_(rhs.type_)
    , lang_(rhs.lang_)
    , words_(rhs.words_)
    , phrase_(rhs.phrase_)
    , entropy_(rhs.entropy_)
    , id_(rhs.id_)
    , storage_(rhs.storage_)
    , encrypted_words_(rhs.encrypted_words_)
    , encrypted_phrase_(rhs.encrypted_phrase_)
    , encrypted_entropy_(rhs.encrypted_entropy_)
    , data_(*rhs.data_.lock())
{
}

Seed::Imp::Imp(
    const api::Session& api,
    const opentxs::crypto::Bip32& bip32,
    const opentxs::crypto::Bip39& bip39,
    const api::crypto::Symmetric& symmetric,
    const api::session::Factory& factory,
    const api::session::Storage& storage,
    const Language lang,
    const SeedStrength strength,
    const PasswordPrompt& reason) noexcept(false)
    : Imp(
          api,
          bip32,
          bip39,
          symmetric,
          factory,
          storage,
          SeedStyle::BIP39,
          lang,
          [&] {
              const auto random = [&] {
                  auto out = factory.Secret(0);
                  static constexpr auto bitsPerByte{8u};
                  const auto bytes =
                      static_cast<std::size_t>(strength) / bitsPerByte;

                  if ((16u > bytes) || (32u < bytes)) {
                      throw std::runtime_error{"Invalid seed strength"};
                  }

                  out.Randomize(
                      static_cast<std::size_t>(strength) / bitsPerByte);

                  return out;
              }();
              auto out = factory.Secret(0);

              if (false == bip39.SeedToWords(random, out, lang)) {
                  throw std::runtime_error{
                      "Unable to convert entropy to word list"};
              }

              return out;
          }(),
          [&] {
              auto out = factory.Secret(0);
              out.AssignText(no_passphrase_);

              return out;
          }(),
          reason)
{
}

Seed::Imp::Imp(
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
    : type_(type)
    , lang_(lang)
    , words_(words)
    , phrase_(passphrase)
    , entropy_([&] {
        auto out = factory.Secret(0);

        if (false ==
            bip39.WordsToSeed(api, type, lang_, words, out, passphrase)) {
            throw std::runtime_error{"Failed to calculate entropy"};
        }

        return out;
    }())
    , id_(bip32.SeedID(entropy_.Bytes()))
    , storage_(&storage)
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
    , data_()
{
    if (16u > entropy_.size()) {
        throw std::runtime_error{"Entropy too short"};
    }

    if (64u < entropy_.size()) { throw std::runtime_error{"Entropy too long"}; }

    if (false == save()) { throw std::runtime_error{"Failed to save seed"}; }
}

Seed::Imp::Imp(
    const opentxs::crypto::Bip32& bip32,
    const opentxs::crypto::Bip39& bip39,
    const api::crypto::Symmetric& symmetric,
    const api::session::Factory& factory,
    const api::session::Storage& storage,
    const Secret& entropy,
    const PasswordPrompt& reason) noexcept(false)
    : type_(SeedStyle::BIP32)
    , lang_(Language::none)
    , words_(factory.Secret(0))
    , phrase_(factory.Secret(0))
    , entropy_(entropy)
    , id_(bip32.SeedID(entropy_.Bytes()))
    , storage_(&storage)
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
    , data_()
{
    if (16u > entropy_.size()) {
        throw std::runtime_error{"Entropy too short"};
    }

    if (64u < entropy_.size()) { throw std::runtime_error{"Entropy too long"}; }

    if (false == save()) { throw std::runtime_error{"Failed to save seed"}; }
}

Seed::Imp::Imp(
    const api::Session& api,
    const opentxs::crypto::Bip39& bip39,
    const api::crypto::Symmetric& symmetric,
    const api::session::Factory& factory,
    const api::session::Storage& storage,
    const proto::Seed& proto,
    const PasswordPrompt& reason) noexcept(false)
    : type_(internal::translate(proto.type()))
    , lang_(internal::translate(proto.lang()))
    , words_(factory.Secret(0))
    , phrase_(factory.Secret(0))
    , entropy_(factory.Secret(0))
    , id_(factory.IdentifierFromBase58(proto.fingerprint()))
    , storage_(&storage)
    , encrypted_words_(proto.has_words() ? proto.words() : proto::Ciphertext{})
    , encrypted_phrase_(
          proto.has_passphrase() ? proto.passphrase() : proto::Ciphertext{})
    , encrypted_entropy_(proto.has_raw() ? proto.raw() : proto::Ciphertext{})
    , data_(proto.version(), proto.index())
{
    const auto& session =
        (3 > data_.lock()->version_) ? encrypted_words_ : encrypted_entropy_;
    const auto key = symmetric.InternalSymmetric().Key(
        session.key(), opentxs::translate(session.mode()));

    if (false == key) {
        throw std::runtime_error{"Failed to get decryption key"};
    }

    if (proto.has_words()) {
        auto& words = const_cast<Secret&>(words_);
        const auto rc = key.Internal().Decrypt(
            encrypted_words_, words.WriteInto(Secret::Mode::Text), reason);

        if (false == rc) {
            throw std::runtime_error{"Failed to decrypt words"};
        }
    }

    if (proto.has_passphrase()) {
        auto& phrase = const_cast<Secret&>(phrase_);
        const auto rc = key.Internal().Decrypt(
            encrypted_phrase_, phrase.WriteInto(Secret::Mode::Text), reason);

        if (false == rc) {
            throw std::runtime_error{"Failed to decrypt passphrase"};
        }
    }

    if (proto.has_raw()) {
        auto& entropy = const_cast<Secret&>(entropy_);
        const auto rc = key.Internal().Decrypt(
            encrypted_entropy_, entropy.WriteInto(Secret::Mode::Text), reason);

        if (false == rc) {
            throw std::runtime_error{"Failed to decrypt entropy"};
        }
    } else {
        OT_ASSERT(proto.has_words());

        auto& entropy = const_cast<Secret&>(entropy_);

        if (false ==
            bip39.WordsToSeed(api, type_, lang_, words_, entropy, phrase_)) {
            throw std::runtime_error{"Failed to calculate entropy"};
        }

        auto ctext = const_cast<proto::Ciphertext&>(encrypted_entropy_);
        auto cwords = const_cast<proto::Ciphertext&>(encrypted_words_);

        if (!key.Internal().Encrypt(entropy_.Bytes(), ctext, reason, true)) {
            throw std::runtime_error{"Failed to encrypt entropy"};
        }

        if (!key.Internal().Encrypt(words_.Bytes(), cwords, reason, false)) {
            throw std::runtime_error{"Failed to encrypt words"};
        }

        data_.lock()->version_ = default_version_;
    }
}

auto Seed::Imp::encrypt(
    const SeedStyle type,
    const api::crypto::Symmetric& symmetric,
    const Secret& entropy,
    const Secret& words,
    const Secret& phrase,
    proto::Ciphertext& cwords,
    proto::Ciphertext& cphrase,
    const PasswordPrompt& reason) noexcept(false) -> proto::Ciphertext
{
    auto key =
        symmetric.Key(crypto::symmetric::Algorithm::ChaCha20Poly1305, reason);

    if (false == key) {
        throw std::runtime_error{"Failed to get encryption key"};
    }

    if (0u < words.size()) {
        if (!key.Internal().Encrypt(words.Bytes(), cwords, reason, false)) {
            throw std::runtime_error{"Failed to encrypt words"};
        }
    }

    if (0u < phrase.size()) {
        if (!key.Internal().Encrypt(phrase.Bytes(), cphrase, reason, false)) {
            throw std::runtime_error{"Failed to encrypt phrase"};
        }
    }

    auto out = proto::Ciphertext{};

    if (!key.Internal().Encrypt(entropy.Bytes(), out, reason, true)) {
        throw std::runtime_error{"Failed to encrypt entropy"};
    }

    return out;
}

auto Seed::Imp::Index() const noexcept -> Bip32Index
{
    return data_.lock()->index_;
}

auto Seed::Imp::IncrementIndex(const Bip32Index index) noexcept -> bool
{
    auto handle = data_.lock();

    if (handle->index_ > index) {
        LogError()(OT_PRETTY_CLASS())("Index values must always increase.")
            .Flush();

        return false;
    }

    handle->index_ = index;
    handle->version_ = std::max(handle->version_, default_version_);

    return save(*handle);
}

auto Seed::Imp::save() const noexcept -> bool { return save(*data_.lock()); }

auto Seed::Imp::save(const MutableData& data) const noexcept -> bool
{
    if (nullptr == storage_) { return false; }

    const auto id = id_.asBase58(Context().Crypto());
    auto proto = SerializeType{};
    proto.set_version(data.version_);
    proto.set_index(data.index_);
    proto.set_fingerprint(id);
    proto.set_type(internal::translate(type_));
    proto.set_lang(internal::translate(lang_));

    if (0u < words_.size()) { *proto.mutable_words() = encrypted_words_; }

    if (0u < phrase_.size()) {
        *proto.mutable_passphrase() = encrypted_phrase_;
    }

    *proto.mutable_raw() = encrypted_entropy_;

    if (false == storage_->Store(proto)) {
        LogError()(OT_PRETTY_CLASS())("Failed to store seed.").Flush();

        return false;
    }

    return true;
}
}  // namespace opentxs::crypto

namespace opentxs::crypto
{
Seed::Seed(Imp* imp) noexcept
    : imp_(imp)
{
    OT_ASSERT(nullptr != imp_);
}

Seed::Seed() noexcept
    : Seed(std::make_unique<Imp>().release())
{
}

Seed::Seed(const Seed& rhs) noexcept
    : Seed(std::make_unique<Imp>(*rhs.imp_).release())
{
}

Seed::Seed(Seed&& rhs) noexcept
    : Seed(std::make_unique<Imp>().release())
{
    swap(rhs);
}

auto Seed::Entropy() const noexcept -> const Secret& { return imp_->entropy_; }

auto Seed::ID() const noexcept -> const identifier::Generic&
{
    return imp_->id_;
}

auto Seed::Index() const noexcept -> Bip32Index { return imp_->Index(); }

auto Seed::Internal() const noexcept -> const internal::Seed& { return *imp_; }

auto Seed::Internal() noexcept -> internal::Seed& { return *imp_; }

auto Seed::operator=(const Seed& rhs) noexcept -> Seed&
{
    auto old = std::unique_ptr<Imp>(imp_);
    imp_ = std::make_unique<Imp>(*rhs.imp_).release();

    return *this;
}

auto Seed::operator=(Seed&& rhs) noexcept -> Seed&
{
    swap(rhs);

    return *this;
}

auto Seed::Phrase() const noexcept -> const Secret& { return imp_->phrase_; }

auto Seed::swap(Seed& rhs) noexcept -> void { std::swap(imp_, rhs.imp_); }

auto Seed::Type() const noexcept -> SeedStyle { return imp_->type_; }

auto Seed::Words() const noexcept -> const Secret& { return imp_->words_; }

Seed::~Seed()
{
    if (nullptr != imp_) {
        delete imp_;
        imp_ = nullptr;
    }
}
}  // namespace opentxs::crypto
