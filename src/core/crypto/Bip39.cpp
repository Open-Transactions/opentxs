/************************************************************
 *
 *                 OPEN TRANSACTIONS
 *
 *       Financial Cryptography and Digital Cash
 *       Library, Protocol, API, Server, CLI, GUI
 *
 *       -- Anonymous Numbered Accounts.
 *       -- Untraceable Digital Cash.
 *       -- Triple-Signed Receipts.
 *       -- Cheques, Vouchers, Transfers, Inboxes.
 *       -- Basket Currencies, Markets, Payment Plans.
 *       -- Signed, XML, Ricardian-style Contracts.
 *       -- Scripted smart contracts.
 *
 *  EMAIL:
 *  fellowtraveler@opentransactions.org
 *
 *  WEBSITE:
 *  http://www.opentransactions.org/
 *
 *  -----------------------------------------------------
 *
 *   LICENSE:
 *   This Source Code Form is subject to the terms of the
 *   Mozilla Public License, v. 2.0. If a copy of the MPL
 *   was not distributed with this file, You can obtain one
 *   at http://mozilla.org/MPL/2.0/.
 *
 *   DISCLAIMER:
 *   This program is distributed in the hope that it will
 *   be useful, but WITHOUT ANY WARRANTY; without even the
 *   implied warranty of MERCHANTABILITY or FITNESS FOR A
 *   PARTICULAR PURPOSE.  See the Mozilla Public License
 *   for more details.
 *
 ************************************************************/
#include "stdafx.hpp"

#include "opentxs/core/crypto/Bip39.hpp"

#if OT_CRYPTO_WITH_BIP39
#include "opentxs/api/crypto/Crypto.hpp"
#include "opentxs/api/crypto/Hash.hpp"
#include "opentxs/api/crypto/Symmetric.hpp"
#include "opentxs/api/storage/Storage.hpp"
#include "opentxs/api/Native.hpp"
#include "opentxs/core/crypto/Bip32.hpp"
#include "opentxs/core/crypto/CryptoSymmetric.hpp"
#include "opentxs/core/crypto/OTPasswordData.hpp"
#include "opentxs/core/crypto/SymmetricKey.hpp"
#include "opentxs/core/util/Assert.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Log.hpp"

#include <memory>
#include <string>

#define OT_METHOD "opentxs::Bip39::"

namespace opentxs
{

Bip39::Bip39(api::Native& native)
    : native_(native)
{
}

const proto::SymmetricMode Bip39::DEFAULT_ENCRYPTION_MODE =
    proto::SMODE_CHACHA20POLY1305;
const std::string Bip39::DEFAULT_PASSPHRASE = "";

bool Bip39::DecryptSeed(
    const proto::Seed& seed,
    OTPassword& words,
    OTPassword& phrase) const
{
    if (!proto::Validate(seed, VERBOSE)) {
        return false;
    }

    const auto& cwords = seed.words();
    const auto& cphrase = seed.passphrase();
    const OTPasswordData reason("Decrypting a new BIP39 seed");

    auto key = native_.Crypto().Symmetric().Key(cwords.key(), cwords.mode());

    OT_ASSERT(key);
    OT_ASSERT(words.isPassword());

    const bool haveWords = key->Decrypt(seed.words(), reason, words);

    if (!haveWords) {
        otErr << OT_METHOD << __FUNCTION__ << ": Failed to decrypt words."
              << std::endl;

        return false;
    }

    OT_ASSERT(phrase.isPassword());

    if (seed.has_passphrase()) {
        const bool havePassphrase = key->Decrypt(cphrase, reason, phrase);

        if (!havePassphrase) {
            otErr << OT_METHOD << __FUNCTION__
                  << ": Failed to decrypt passphrase." << std::endl;

            return false;
        }
    }

    return true;
}

std::string Bip39::DefaultSeed() const { return native_.DB().DefaultSeed(); }

std::string Bip39::SaveSeed(
    const OTPassword& words,
    const OTPassword& passphrase) const
{
    OT_ASSERT(words.isPassword() && passphrase.isPassword());

    auto seed = native_.Crypto().AES().InstantiateBinarySecretSP();
    WordsToSeed(words, *seed, passphrase);

    OT_ASSERT(1 < seed->getMemorySize());

    // the fingerprint is used as the identifier of the seed for indexing
    // purposes. Always use the secp256k1 version for this.
    auto fingerprint = native_.Crypto().BIP32().SeedToFingerprint(
        EcdsaCurve::SECP256K1, *seed);
    const OTPasswordData reason("Encrypting a new BIP39 seed");
    auto key =
        native_.Crypto().Symmetric().Key(reason, DEFAULT_ENCRYPTION_MODE);

    OT_ASSERT(key);

    proto::Seed serialized;
    serialized.set_version(2);
    serialized.set_index(0);
    auto& encryptedWords = *serialized.mutable_words();
    auto& encryptedPassphrase = *serialized.mutable_passphrase();
    serialized.set_fingerprint(fingerprint);
    auto empty = Data::Factory();
    const bool haveWords = key->Encrypt(words, empty, reason, encryptedWords);

    if (false == haveWords) {
        otErr << OT_METHOD << __FUNCTION__ << ": Failed to encrypt seed."
              << std::endl;

        return "";
    }

    bool havePassphrase =
        key->Encrypt(passphrase, empty, reason, encryptedPassphrase, false);

    if (!havePassphrase) {
        otErr << OT_METHOD << __FUNCTION__ << ": Failed to encrypt passphrase."
              << std::endl;

        return "";
    }

    const bool stored = native_.DB().Store(serialized, fingerprint);

    if (!stored) {
        otErr << OT_METHOD << __FUNCTION__ << ": Failed to store seed."
              << std::endl;

        return "";
    }

    return fingerprint;
}

bool Bip39::SeedToData(
    const OTPassword& words,
    const OTPassword& passphrase,
    OTPassword& output) const
{
    OT_ASSERT(words.isPassword());
    OT_ASSERT(passphrase.isPassword());

    WordsToSeed(words, output, passphrase);

    return true;
}

std::string Bip39::ImportSeed(
    const OTPassword& words,
    const OTPassword& passphrase) const
{
    return SaveSeed(words, passphrase);
}

std::string Bip39::NewSeed() const
{
    auto entropy = native_.Crypto().AES().InstantiateBinarySecretSP();

    if (entropy) {
        entropy->randomizeMemory(256 / 8);
        OTPassword words, passphrase;
        passphrase.setPassword(DEFAULT_PASSPHRASE);

        if (false == toWords(*entropy, words)) {
            otErr << OT_METHOD << __FUNCTION__
                  << ": Unable to convert entropy to word list." << std::endl;

            return {};
        }

        return SaveSeed(words, passphrase);
    } else {
        OT_FAIL;
    }

    return "";
}

std::string Bip39::Passphrase(const std::string& fingerprint) const
{
    // TODO: make fingerprint non-const
    std::string input(fingerprint);
    std::int32_t notUsed = 0;
    auto seed = SerializedSeed(input, notUsed);

    if (!seed) {
        return "";
    }

    OTPassword words, phrase;

    if (!DecryptSeed(*seed, words, phrase)) {
        return "";
    }

    return phrase.getPassword();
}

std::shared_ptr<OTPassword> Bip39::Seed(
    std::string& fingerprint,
    std::int32_t& index) const
{
    auto output = native_.Crypto().AES().InstantiateBinarySecretSP();

    OT_ASSERT(output);

    auto serialized = SerializedSeed(fingerprint, index);

    if (serialized) {
        std::unique_ptr<OTPassword> seed(
            native_.Crypto().AES().InstantiateBinarySecret());

        OT_ASSERT(seed);

        OTPassword words, phrase;

        if (false == DecryptSeed(*serialized, words, phrase)) {
            return {};
        }

        bool extracted = SeedToData(words, phrase, *seed);

        if (extracted) {
            output.reset(seed.release());
        } else {
            OT_FAIL;
        }
    }

    return output;
}

std::shared_ptr<proto::Seed> Bip39::SerializedSeed(
    std::string& fingerprint,
    std::int32_t& index) const
{
    const bool wantDefaultSeed = fingerprint.empty();
    std::shared_ptr<proto::Seed> serialized;
    index = 0;

    if (wantDefaultSeed) {
        std::string defaultFingerprint = native_.DB().DefaultSeed();
        bool haveDefaultSeed = !defaultFingerprint.empty();

        if (false == haveDefaultSeed) {
            defaultFingerprint = NewSeed();
        }

        if (!defaultFingerprint.empty()) {
            serialized = SerializedSeed(defaultFingerprint, index);
        } else {
            OT_FAIL;
        }

        // Update to correct value
        fingerprint = defaultFingerprint;
    } else {  // want an explicitly identified seed
        native_.DB().Load(fingerprint, serialized);
    }

    index = serialized->index();

    return serialized;
}

bool Bip39::UpdateIndex(std::string& seed, const std::int32_t index) const
{
    std::int32_t oldIndex = 0;
    auto serialized = SerializedSeed(seed, oldIndex);

    if (oldIndex > index) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Index values must always increase." << std::endl;

        return false;
    }

    serialized->set_index(index);

    if (serialized->version() < 2) {
        serialized->set_version(2);
    }

    return native_.DB().Store(*serialized, seed);
}

std::string Bip39::Words(const std::string& fingerprint) const
{
    // TODO: make fingerprint non-const
    std::string input(fingerprint);
    std::int32_t notUsed;
    auto seed = SerializedSeed(input, notUsed);

    if (!seed) {
        return "";
    }

    OTPassword words, phrase;

    if (!DecryptSeed(*seed, words, phrase)) {
        return "";
    }

    return words.getPassword();
}

}  // namespace opentxs
#endif
