// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/serialization/protobuf/verify/Credential.hpp"  // IWYU pragma: associated

#include <ChildCredentialParameters.pb.h>
#include <Credential.pb.h>
#include <Enums.pb.h>
#include <cstdint>
#include <memory>
#include <sstream>

#include "internal/serialization/protobuf/verify/ChildCredentialParameters.hpp"  // IWYU pragma: keep
#include "internal/serialization/protobuf/verify/ContactData.hpp"  // IWYU pragma: keep
#include "internal/serialization/protobuf/verify/Identifier.hpp"  // IWYU pragma: keep
#include "internal/serialization/protobuf/verify/KeyCredential.hpp"  // IWYU pragma: keep
#include "internal/serialization/protobuf/verify/MasterCredentialParameters.hpp"  // IWYU pragma: keep
#include "internal/serialization/protobuf/verify/Signature.hpp"  // IWYU pragma: keep
#include "internal/serialization/protobuf/verify/VerificationSet.hpp"  // IWYU pragma: keep
#include "internal/serialization/protobuf/verify/VerifyContacts.hpp"
#include "internal/serialization/protobuf/verify/VerifyCredentials.hpp"
#include "serialization/protobuf/verify/Check.hpp"

namespace opentxs::proto
{

auto CheckProto_1(
    const Credential& input,
    const bool silent,
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

auto CheckProto_2(
    const Credential& input,
    const bool silent,
    const KeyMode& mode,
    const CredentialRole role,
    const bool withSigs) -> bool
{
    return CheckProto_1(input, silent, mode, role, withSigs);
}

auto CheckProto_3(
    const Credential& input,
    const bool silent,
    const KeyMode& mode,
    const CredentialRole role,
    const bool withSigs) -> bool
{
    return CheckProto_1(input, silent, mode, role, withSigs);
}

auto CheckProto_4(
    const Credential& input,
    const bool silent,
    const KeyMode& mode,
    const CredentialRole role,
    const bool withSigs) -> bool
{
    return CheckProto_1(input, silent, mode, role, withSigs);
}

auto CheckProto_5(
    const Credential& input,
    const bool silent,
    const KeyMode& mode,
    const CredentialRole role,
    const bool withSigs) -> bool
{
    return CheckProto_1(input, silent, mode, role, withSigs);
}

auto CheckProto_6(
    const Credential& input,
    const bool silent,
    const KeyMode& mode,
    const CredentialRole role,
    const bool withSigs) -> bool
{
    return CheckProto_1(input, silent, mode, role, withSigs);
}

auto CheckProto_7(
    const Credential& input,
    const bool silent,
    const KeyMode&,
    const CredentialRole,
    const bool) -> bool
{
    UNDEFINED_VERSION(7);
}

auto CheckProto_8(
    const Credential& input,
    const bool silent,
    const KeyMode&,
    const CredentialRole,
    const bool) -> bool
{
    UNDEFINED_VERSION(8);
}

auto CheckProto_9(
    const Credential& input,
    const bool silent,
    const KeyMode&,
    const CredentialRole,
    const bool) -> bool
{
    UNDEFINED_VERSION(9);
}

auto CheckProto_10(
    const Credential& input,
    const bool silent,
    const KeyMode&,
    const CredentialRole,
    const bool) -> bool
{
    UNDEFINED_VERSION(10);
}

auto CheckProto_11(
    const Credential& input,
    const bool silent,
    const KeyMode&,
    const CredentialRole,
    const bool) -> bool
{
    UNDEFINED_VERSION(11);
}

auto CheckProto_12(
    const Credential& input,
    const bool silent,
    const KeyMode&,
    const CredentialRole,
    const bool) -> bool
{
    UNDEFINED_VERSION(12);
}

auto CheckProto_13(
    const Credential& input,
    const bool silent,
    const KeyMode&,
    const CredentialRole,
    const bool) -> bool
{
    UNDEFINED_VERSION(13);
}

auto CheckProto_14(
    const Credential& input,
    const bool silent,
    const KeyMode&,
    const CredentialRole,
    const bool) -> bool
{
    UNDEFINED_VERSION(14);
}

auto CheckProto_15(
    const Credential& input,
    const bool silent,
    const KeyMode&,
    const CredentialRole,
    const bool) -> bool
{
    UNDEFINED_VERSION(15);
}

auto CheckProto_16(
    const Credential& input,
    const bool silent,
    const KeyMode&,
    const CredentialRole,
    const bool) -> bool
{
    UNDEFINED_VERSION(16);
}

auto CheckProto_17(
    const Credential& input,
    const bool silent,
    const KeyMode&,
    const CredentialRole,
    const bool) -> bool
{
    UNDEFINED_VERSION(17);
}

auto CheckProto_18(
    const Credential& input,
    const bool silent,
    const KeyMode&,
    const CredentialRole,
    const bool) -> bool
{
    UNDEFINED_VERSION(18);
}

auto CheckProto_19(
    const Credential& input,
    const bool silent,
    const KeyMode&,
    const CredentialRole,
    const bool) -> bool
{
    UNDEFINED_VERSION(19);
}

auto CheckProto_20(
    const Credential& input,
    const bool silent,
    const KeyMode&,
    const CredentialRole,
    const bool) -> bool
{
    UNDEFINED_VERSION(20);
}
}  // namespace opentxs::proto
