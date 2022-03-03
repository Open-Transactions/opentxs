// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"

namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

class OTSignatureMetadata
{
public:
    auto operator==(const OTSignatureMetadata& rhs) const -> bool;

    auto operator!=(const OTSignatureMetadata& rhs) const -> bool
    {
        return !(operator==(rhs));
    }

    auto SetMetadata(
        char metaKeyType,
        char metaNymID,
        char metaMasterCredID,
        char metaChildCredID) -> bool;

    inline auto HasMetadata() const -> bool { return hasMetadata_; }

    inline auto GetKeyType() const -> char { return metaKeyType_; }

    inline auto FirstCharNymID() const -> char { return metaNymID_; }

    inline auto FirstCharMasterCredID() const -> char
    {
        return metaMasterCredID_;
    }

    inline auto FirstCharChildCredID() const -> char
    {
        return metaChildCredID_;
    }

    OTSignatureMetadata(const api::Session& api);
    auto operator=(const OTSignatureMetadata& rhs) -> OTSignatureMetadata&;

private:
    const api::Session& api_;
    // Defaults to false. Is set true by calling SetMetadata
    bool hasMetadata_{false};
    // Can be A, E, or S (authentication, encryption, or signing.
    // Also, E would be unusual.)
    char metaKeyType_{0x0};
    // Can be any letter from base62 alphabet. Represents
    // first letter of a Nym's ID.
    char metaNymID_{0x0};
    // Can be any letter from base62 alphabet.
    // Represents first letter of a Master Credential
    // ID (for that Nym.)
    char metaMasterCredID_{0x0};
    // Can be any letter from base62 alphabet. Represents
    // first letter of a Credential ID (signed by that Master.)
    char metaChildCredID_{0x0};
};
}  // namespace opentxs
