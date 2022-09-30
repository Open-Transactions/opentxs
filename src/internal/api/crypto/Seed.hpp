// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/api/crypto/Seed.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace proto
{
class HDPath;
}  // namespace proto
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::api::crypto::internal
{
class Seed : virtual public crypto::Seed
{
public:
    using api::crypto::Seed::AccountChildKey;
    virtual auto AccountChildKey(
        const proto::HDPath& path,
        const BIP44Chain internal,
        const Bip32Index index,
        const PasswordPrompt& reason) const
        -> std::unique_ptr<opentxs::crypto::key::HD> = 0;
    virtual auto AccountKey(
        const proto::HDPath& path,
        const BIP44Chain internal,
        const PasswordPrompt& reason) const
        -> std::unique_ptr<opentxs::crypto::key::HD> = 0;
    virtual auto GetOrCreateDefaultSeed(
        UnallocatedCString& seedID,
        opentxs::crypto::SeedStyle& type,
        opentxs::crypto::Language& lang,
        Bip32Index& index,
        const opentxs::crypto::SeedStrength strength,
        const PasswordPrompt& reason) const -> OTSecret = 0;
    auto Internal() const noexcept -> const Seed& final { return *this; }
    virtual auto UpdateIndex(
        const UnallocatedCString& seedID,
        const Bip32Index index,
        const PasswordPrompt& reason) const -> bool = 0;

    auto Internal() noexcept -> Seed& final { return *this; }

    ~Seed() override = default;
};
}  // namespace opentxs::api::crypto::internal
