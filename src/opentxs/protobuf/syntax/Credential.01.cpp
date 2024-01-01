// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/protobuf/syntax/Credential.hpp"  // IWYU pragma: associated

#include <opentxs/protobuf/ChildCredentialParameters.pb.h>
#include <opentxs/protobuf/Credential.pb.h>
#include <opentxs/protobuf/Enums.pb.h>
#include <cstdint>
#include <sstream>
#include <string>

#include "opentxs/protobuf/syntax/ChildCredentialParameters.hpp"  // IWYU pragma: keep
#include "opentxs/protobuf/syntax/ContactData.hpp"    // IWYU pragma: keep
#include "opentxs/protobuf/syntax/Identifier.hpp"     // IWYU pragma: keep
#include "opentxs/protobuf/syntax/KeyCredential.hpp"  // IWYU pragma: keep
#include "opentxs/protobuf/syntax/Macros.hpp"
#include "opentxs/protobuf/syntax/MasterCredentialParameters.hpp"  // IWYU pragma: keep
#include "opentxs/protobuf/syntax/Signature.hpp"        // IWYU pragma: keep
#include "opentxs/protobuf/syntax/VerificationSet.hpp"  // IWYU pragma: keep
#include "opentxs/protobuf/syntax/VerifyContacts.hpp"
#include "opentxs/protobuf/syntax/VerifyCredentials.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::protobuf::inline syntax
{
auto version_1(
    const Credential& input,
    const Log& log,
    const KeyMode& mode,
    const CredentialRole role,
    const bool withSigs) -> bool
{
    bool isPrivate = false;
    bool isPublic = false;
    bool expectMasterSignature = false;
    bool expectSourceSignature = false;
    std::int32_t expectedSigCount = 1;  // public self-signature
    const bool checkRole = (CREDROLE_ERROR != role);

    CHECK_SUBOBJECT(id, CredentialAllowedIdentifier());
    CHECK_EXISTS(type);

    switch (input.type()) {
        case CREDTYPE_LEGACY:
        case CREDTYPE_HD: {
        } break;
        case CREDTYPE_ERROR:
        default: {
            FAIL_2("invalid type", input.type());
        }
    }

    CHECK_EXISTS(role);

    const CredentialRole actualRole = input.role();

    if (checkRole && (role != actualRole)) {
        FAIL_2("incorrect role", input.role());
    }

    const bool masterCredential = (CREDROLE_MASTERKEY == actualRole);
    const bool childKeyCredential = (CREDROLE_CHILDKEY == actualRole);
    const bool keyCredential = (masterCredential || childKeyCredential);
    const bool contactCredential = (CREDROLE_CONTACT == actualRole);
    const bool verifyCredential = (CREDROLE_VERIFY == actualRole);
    const bool metadataContainer = (contactCredential || verifyCredential);
    const bool knownRole = (keyCredential || metadataContainer);

    if (childKeyCredential) {
        expectedSigCount++;  // master signature
        expectMasterSignature = true;
    }

    if (checkRole && !knownRole) { FAIL_2("invalid role", role); }

    if (!input.has_mode()) { FAIL_1("missing mode"); }

    KeyMode requiredMode = KEYMODE_ERROR;

    switch (actualRole) {
        case CREDROLE_MASTERKEY:
        case CREDROLE_CHILDKEY: {
            requiredMode = mode;
        } break;
        case CREDROLE_CONTACT:
        case CREDROLE_VERIFY: {
            requiredMode = KEYMODE_NULL;
        } break;
        case CREDROLE_ERROR:
        default: {
            FAIL_2("incorrect role", input.role());
        }
    }

    const auto actualMode = input.mode();

    if (KEYMODE_ERROR != requiredMode) {
        if (actualMode != requiredMode) {
            FAIL_4(
                "incorrect mode",
                actualMode,
                ". Required mode: ",
                requiredMode);
        }
    }

    switch (actualMode) {
        case KEYMODE_PUBLIC: {
            isPublic = true;
        } break;
        case KEYMODE_PRIVATE: {
            isPrivate = true;

            if (keyCredential) {
                expectedSigCount++;  // private self-signature
            }
        } break;
        case KEYMODE_NULL: {
        } break;
        case KEYMODE_ERROR:
        default: {
            FAIL_2("invalid mode", actualMode);
        }
    }

    CHECK_SUBOBJECT(nymid, CredentialAllowedIdentifier());

    if (!masterCredential) {
        CHECK_SUBOBJECT(childdata, CredentialAllowedChildParams());
    }

    if (masterCredential) {
        CHECK_SUBOBJECT_VA(
            masterdata, CredentialAllowedMasterParams(), expectSourceSignature);

        if (expectSourceSignature) {
            expectedSigCount++;  // source signature
        }
    }

    if ((!masterCredential) && (input.has_masterdata())) {
        FAIL_1("child credential contains master data");
    }

    if (isPublic && input.has_privatecredential()) {
        FAIL_1(" public credential contains private data");
    }

    if (keyCredential) {
        if (input.has_contactdata()) {
            FAIL_1("key credential contains contact data");
        }

        if (input.has_verification()) {
            FAIL_1("key credential contains verification data");
        }

        if (!input.has_publiccredential()) { FAIL_1("missing public data"); }

        if (isPrivate && (!input.has_privatecredential())) {
            FAIL_1("missing private data");
        }
    }

    if (metadataContainer) {
        if (input.has_privatecredential()) {
            FAIL_1("metadata credential contains private key data");
        }

        if (input.has_publiccredential()) {
            FAIL_1("metadata credential contains public key data");
        }
    }

    if (contactCredential) {
        CHECK_EXCLUDED(verification);
        CHECK_SUBOBJECT_VA(
            contactdata, CredentialAllowedContactData(), ClaimType::Normal);
    }

    if (verifyCredential) {
        CHECK_EXCLUDED(contactdata);
        CHECK_SUBOBJECT_VA(
            verification,
            CredentialAllowedVerificationItem(),
            VerificationType::Normal);
    }

    if (keyCredential) {
        CHECK_SUBOBJECT_VA(
            publiccredential,
            CredentialAllowedKeyCredential(),
            input.type(),
            KEYMODE_PUBLIC);

        if (isPrivate) {
            CHECK_SUBOBJECT_VA(
                privatecredential,
                CredentialAllowedKeyCredential(),
                input.type(),
                KEYMODE_PRIVATE);
        }
    }

    if (withSigs) {
        if (expectedSigCount != input.signature_size()) {
            std::stringstream ss;
            ss << input.signature_size() << " of " << expectedSigCount
               << " found";

            FAIL_2("incorrect number of signatures", ss.str());
        }

        std::uint32_t selfPublicCount = 0;
        std::uint32_t selfPrivateCount = 0;
        std::uint32_t masterPublicCount = 0;
        std::uint32_t sourcePublicCount = 0;

        CHECK_SUBOBJECTS_VA(
            signature,
            CredentialAllowedSignatures(),
            input.id(),
            input.childdata().masterid(),
            selfPublicCount,
            selfPrivateCount,
            masterPublicCount,
            sourcePublicCount);

        if (keyCredential) {
            if ((1 != selfPrivateCount) && (isPrivate)) {
                std::stringstream ss;
                ss << selfPrivateCount << " of 1 found";

                FAIL_2("incorrect number of private self-signatures", ss.str());
            }

            if (1 != selfPublicCount) {
                std::stringstream ss;
                ss << selfPublicCount << " of 1 found";

                FAIL_2("incorrect number of public self-signatures", ss.str());
            }
        }

        if ((1 != masterPublicCount) && (expectMasterSignature)) {
            std::stringstream ss;
            ss << masterPublicCount << " of 1 found";

            FAIL_2("incorrect number of public master signatures", ss.str());
        }

        if ((1 != sourcePublicCount) && (expectSourceSignature)) {
            std::stringstream ss;
            ss << sourcePublicCount << " of 1 found";

            FAIL_2("incorrect number of public source signatures", ss.str());
        }
    }

    return true;
}
}  // namespace opentxs::protobuf::inline syntax

#include "opentxs/protobuf/syntax/Macros.undefine.inc"  // IWYU pragma: keep
