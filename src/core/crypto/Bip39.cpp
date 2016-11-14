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
#if OT_CRYPTO_WITH_BIP39
#include "opentxs/core/crypto/Bip39.hpp"

#include "opentxs/core/app/App.hpp"
#include "opentxs/core/crypto/Bip32.hpp"
#include "opentxs/core/crypto/CryptoEngine.hpp"
#include "opentxs/core/crypto/CryptoHashEngine.hpp"
#include "opentxs/core/crypto/CryptoSymmetric.hpp"
#include "opentxs/core/crypto/CryptoSymmetricEngine.hpp"
#include "opentxs/core/crypto/OTPasswordData.hpp"
#include "opentxs/core/crypto/SymmetricKey.hpp"
#include "opentxs/core/util/Assert.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/OTData.hpp"
#include "opentxs/storage/Storage.hpp"

#include <memory>
#include <string>

namespace opentxs
{

const proto::SymmetricMode Bip39::DEFAULT_ENCRYPTION_MODE =
    proto::SMODE_CHACHA20POLY1305;
const std::string Bip39::DEFAULT_PASSPHRASE = "opentxs";

bool Bip39::DecryptSeed(
    const proto::Seed& seed,
    OTPassword& words,
    OTPassword& phrase) const
{
    if (!proto::Check(seed, 0, 0xFFFFFFFF)) { return false; }

    const auto& cwords = seed.words();
    const auto& cphrase = seed.passphrase();
    const OTPasswordData reason("Decrypting a new BIP39 seed");

    auto key = App::Me().Crypto().Symmetric().Key(cwords.key(), cwords.mode());

    OT_ASSERT(key);
    OT_ASSERT(words.isPassword());

    const bool haveWords =
        key->Decrypt(
            seed.words(),
            reason,
            words);

    if (!haveWords) {
        otErr << __FUNCTION__ << ": Failed to decrypt words." << std::endl;

        return false;
    }

    OT_ASSERT(phrase.isPassword());

    if (seed.has_passphrase()) {
        const bool havePassphrase = key->Decrypt(
            cphrase,
            reason,
            phrase);

        if (!havePassphrase) {
            otErr << __FUNCTION__ << ": Failed to decrypt passphrase."
                  << std::endl;

            return false;
        }
    }

    return true;
}

std::string Bip39::SaveSeed(
    const OTPassword& words,
    const OTPassword& passphrase) const
{
    OT_ASSERT(words.isPassword() && passphrase.isPassword());

    auto seed = App::Me().Crypto().AES().InstantiateBinarySecretSP();
    WordsToSeed(words, *seed, passphrase);

    OT_ASSERT(1 < seed->getMemorySize());

    // the fingerprint is used as the identifier of the seed for indexing
    // purposes. Always use the secp256k1 version for this.
    auto fingerprint = App::Me().Crypto().BIP32().SeedToFingerprint(
        EcdsaCurve::SECP256K1, *seed);
    const OTPasswordData reason("Encrypting a new BIP39 seed");
    auto key = App::Me().Crypto().Symmetric().Key(
        reason,
        DEFAULT_ENCRYPTION_MODE);

    OT_ASSERT(key);

    proto::Seed serialized;
    serialized.set_version(1);
    auto& encryptedWords = *serialized.mutable_words();
    auto& encryptedPassphrase = *serialized.mutable_passphrase();
    serialized.set_fingerprint(fingerprint);

    OTData empty;

    const bool haveWords = key->Encrypt(
        words,
        empty,
        reason,
        encryptedWords);

    if (!haveWords) {
        otErr << __FUNCTION__ << ": Failed to encrypt seed." << std::endl;

        return "";
    }

    bool havePassphrase = key->Encrypt(
        passphrase,
        empty,
        reason,
        encryptedPassphrase,
        false);


    if (!havePassphrase) {
        otErr << __FUNCTION__ << ": Failed to encrypt passphrase." << std::endl;

        return "";
    }

    const bool stored = App::Me().DB().Store(serialized, fingerprint);

    if (!stored) {
        otErr << __FUNCTION__ << ": Failed to store seed." << std::endl;

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

    WordsToSeed(
        words,
        output,
        passphrase);

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
    auto entropy = App::Me().Crypto().AES().InstantiateBinarySecretSP();

    if (entropy) {
        entropy->randomizeMemory(256/8);
        OTPassword words, passphrase;
        passphrase.setPassword(DEFAULT_PASSPHRASE);

        if (!toWords(*entropy, words)) { return ""; }

        return SaveSeed(words, passphrase);
    }

    return "";
}

std::string Bip39::Passphrase(const std::string& fingerprint) const
{
    //TODO: make fingerprint non-const
    std::string input (fingerprint);
    auto seed = SerializedSeed(input);

    if (!seed) { return ""; }

    OTPassword words, phrase;

    if (!DecryptSeed(*seed, words, phrase)) { return ""; }

    return phrase.getPassword();
}

std::shared_ptr<OTPassword> Bip39::Seed(std::string& fingerprint) const
{
    auto output = App::Me().Crypto().AES().InstantiateBinarySecretSP();

    OT_ASSERT(output);

        auto serialized = SerializedSeed(fingerprint);

        if (serialized) {
            std::unique_ptr<OTPassword>
                seed(App::Me().Crypto().AES().InstantiateBinarySecret());

            OT_ASSERT(seed);

            OTPassword words, passphrase;
            const bool decrypted = DecryptSeed(*serialized, words, passphrase);

            OT_ASSERT(decrypted);

            bool extracted = SeedToData(words, passphrase, *seed);

            if (extracted) {
                output.reset(seed.release());
            }
        }

    return output;
}

std::shared_ptr<proto::Seed> Bip39::SerializedSeed(
    std::string& fingerprint) const
{
    const bool wantDefaultSeed = fingerprint.empty();
    std::shared_ptr<proto::Seed> serialized;

    if (wantDefaultSeed) {
        std::string defaultFingerprint = App::Me().DB().DefaultSeed();
        bool haveDefaultSeed = !defaultFingerprint.empty();

        if (!haveDefaultSeed) {
            defaultFingerprint = NewSeed();
        }

        if (!defaultFingerprint.empty()) {
            serialized = SerializedSeed(defaultFingerprint);
        } else {
            OT_FAIL;
        }

        // Update to correct value
        fingerprint = defaultFingerprint;
    } else { // want an explicitly identified seed
        App::Me().DB().Load(fingerprint, serialized);
    }

    return serialized;
}

std::string Bip39::Words(const std::string& fingerprint) const
{
    //TODO: make fingerprint non-const
    std::string input (fingerprint);
    auto seed = SerializedSeed(input);

    if (!seed) { return ""; }

    OTPassword words, phrase;

    if (!DecryptSeed(*seed, words, phrase)) { return ""; }

    return words.getPassword();
}

} // namespace opentxs
#endif
