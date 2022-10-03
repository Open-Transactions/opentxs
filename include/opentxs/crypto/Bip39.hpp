// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <string_view>

#include "opentxs/Export.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

class Secret;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::crypto
{
class OPENTXS_EXPORT Bip39
{
public:
    using Suggestions = UnallocatedVector<UnallocatedCString>;

    virtual auto GetSuggestions(
        const Language lang,
        const std::string_view word) const noexcept -> Suggestions = 0;
    virtual auto LongestWord(const Language lang) const noexcept
        -> std::size_t = 0;
    virtual auto SeedToWords(
        const Secret& seed,
        Secret& words,
        const Language lang) const noexcept -> bool = 0;
    virtual auto WordsToSeed(
        const api::Session& api,
        const SeedStyle type,
        const Language lang,
        const Secret& words,
        Secret& seed,
        const Secret& passphrase) const noexcept -> bool = 0;

    Bip39(const Bip39&) = delete;
    Bip39(Bip39&&) = delete;
    auto operator=(const Bip39&) -> Bip39& = delete;
    auto operator=(Bip39&&) -> Bip39& = delete;

    virtual ~Bip39() = default;

protected:
    Bip39() = default;
};
}  // namespace opentxs::crypto
