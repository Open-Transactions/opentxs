// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <cstdint>
#include <string_view>

#include "opentxs/crypto/Bip39.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Crypto;
class Session;
}  // namespace api

class Secret;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::crypto::implementation
{
class Bip39 final : public opentxs::crypto::Bip39
{
public:
    using WordList = UnallocatedVector<const char*>;
    using Words = UnallocatedMap<Language, WordList>;

    static const Words words_;

    auto GetSuggestions(const Language lang, const std::string_view word)
        const noexcept -> Suggestions final;
    auto LongestWord(const Language lang) const noexcept -> std::size_t final;
    auto SeedToWords(const Secret& seed, Secret& words, const Language lang)
        const noexcept -> bool final;
    auto WordsToSeed(
        const api::Session& api,
        const SeedStyle type,
        const Language lang,
        const Secret& words,
        Secret& seed,
        const Secret& passphrase) const noexcept -> bool final;

    Bip39(const api::Crypto& crypto) noexcept;
    Bip39() = delete;
    Bip39(const Bip39&) = delete;
    Bip39(Bip39&&) = delete;
    auto operator=(const Bip39&) -> Bip39& = delete;
    auto operator=(Bip39&&) -> Bip39& = delete;

    ~Bip39() final = default;

private:
    using LongestWords = UnallocatedMap<Language, std::size_t>;
    using MnemonicWords = UnallocatedVector<UnallocatedCString>;

    static const LongestWords longest_words_;
    static const std::size_t BitsPerWord;
    static const std::uint8_t ByteBits;
    static const std::size_t DictionarySize;
    static const std::size_t EntropyBitDivisor;
    static const std::size_t HmacIterationCount;
    static const std::size_t HmacOutputSizeBytes;
    static const UnallocatedCString PassphrasePrefix;
    static const std::size_t ValidMnemonicWordMultiple;

    const api::Crypto& crypto_;

    static auto bitShift(std::size_t theBit) noexcept -> std::byte;
    static auto find_longest_words(const Words& words) noexcept -> LongestWords;
    static auto tokenize(const Language lang, const ReadView words) noexcept(
        false) -> UnallocatedVector<std::size_t>;

    auto entropy_to_words(
        const Secret& entropy,
        Secret& words,
        const Language lang) const noexcept -> bool;
    auto words_to_root_bip39(
        const Secret& words,
        Secret& bip32RootNode,
        const Secret& passphrase) const noexcept -> bool;
    auto words_to_root_pkt(
        const api::Session& api,
        const Language lang,
        const Secret& words,
        Secret& bip32RootNode,
        const Secret& passphrase) const noexcept -> bool;
};
}  // namespace opentxs::crypto::implementation
