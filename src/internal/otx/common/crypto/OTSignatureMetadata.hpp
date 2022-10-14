// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

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

    inline auto HasMetadata() const -> bool { return has_metadata_; }

    inline auto GetKeyType() const -> char { return meta_key_type_; }

    inline auto FirstCharNymID() const -> char { return meta_nym_id_; }

    inline auto FirstCharMasterCredID() const -> char
    {
        return meta_master_cred_id_;
    }

    inline auto FirstCharChildCredID() const -> char
    {
        return meta_child_cred_id_;
    }

    OTSignatureMetadata(const api::Session& api);
    auto operator=(const OTSignatureMetadata& rhs) -> OTSignatureMetadata&;

private:
    const api::Session& api_;
    // Defaults to false. Is set true by calling SetMetadata
    bool has_metadata_{false};
    // Can be A, E, or S (authentication, encryption, or signing.
    // Also, E would be unusual.)
    char meta_key_type_{0x0};
    // Can be any letter from base64 alphabet. Represents
    // first letter of a Nym's ID.
    char meta_nym_id_{0x0};
    // Can be any letter from base64 alphabet.
    // Represents first letter of a Master Credential
    // ID (for that Nym.)
    char meta_master_cred_id_{0x0};
    // Can be any letter from base64 alphabet. Represents
    // first letter of a Credential ID (signed by that Master.)
    char meta_child_cred_id_{0x0};
};
}  // namespace opentxs
