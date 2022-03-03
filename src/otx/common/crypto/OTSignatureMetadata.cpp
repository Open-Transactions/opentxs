// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated
#include "internal/otx/common/crypto/OTSignatureMetadata.hpp"  // IWYU pragma: associated

#include "internal/util/LogMacros.hpp"
#include "opentxs/api/crypto/Encode.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs
{
OTSignatureMetadata::OTSignatureMetadata(const api::Session& api)
    : api_(api)
    , hasMetadata_(false)
    , metaKeyType_(0)
    , metaNymID_(0)
    , metaMasterCredID_(0)
    , metaChildCredID_(0)
{
}

auto OTSignatureMetadata::operator=(const OTSignatureMetadata& rhs)
    -> OTSignatureMetadata&
{
    if (this != &rhs) {
        hasMetadata_ = rhs.hasMetadata_;
        metaKeyType_ = rhs.metaKeyType_;
        metaNymID_ = rhs.metaNymID_;
        metaMasterCredID_ = rhs.metaMasterCredID_;
        metaChildCredID_ = rhs.metaChildCredID_;
    }

    return *this;
}

auto OTSignatureMetadata::SetMetadata(
    char metaKeyType,
    char metaNymID,
    char metaMasterCredID,
    char metaChildCredID) -> bool
{
    switch (metaKeyType) {
        // authentication (used for signing transmissions and stored files.)
        case 'A':
        // encryption (unusual BTW, to see this in a signature. Should
        // never actually happen, or at least should be rare and strange
        // when it does.)
        case 'E':
        // signing (a "legal signature.")
        case 'S':
            break;
        default:
            LogError()(OT_PRETTY_CLASS())(
                "Expected key type of A, E, or S, but instead found: ")(
                metaKeyType)(" (bad data or error).")
                .Flush();
            return false;
    }

    // Todo: really should verify base58 here now, instead of base62.
    UnallocatedCString str_verify_base62;

    str_verify_base62 += metaNymID;
    str_verify_base62 += metaMasterCredID;
    str_verify_base62 += metaChildCredID;

    if (false == api_.Crypto().Encode().IsBase62(str_verify_base62)) {
        LogError()(OT_PRETTY_CLASS())(
            "Metadata for signature failed base62 validation: ")(
            str_verify_base62)(".")
            .Flush();
        return false;
    }

    metaKeyType_ = metaKeyType;
    metaNymID_ = metaNymID;
    metaMasterCredID_ = metaMasterCredID;
    metaChildCredID_ = metaChildCredID;
    hasMetadata_ = true;

    return true;
}

auto OTSignatureMetadata::operator==(const OTSignatureMetadata& rhs) const
    -> bool
{
    return (
        (HasMetadata() == rhs.HasMetadata()) &&
        (GetKeyType() == rhs.GetKeyType()) &&
        (FirstCharNymID() == rhs.FirstCharNymID()) &&
        (FirstCharMasterCredID() == rhs.FirstCharMasterCredID()) &&
        (FirstCharChildCredID() == rhs.FirstCharChildCredID()));
}
}  // namespace opentxs
