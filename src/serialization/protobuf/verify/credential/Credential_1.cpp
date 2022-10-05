// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/serialization/protobuf/verify/Credential.hpp"  // IWYU pragma: associated

#include <ChildCredentialParameters.pb.h>
#include <ContactData.pb.h>  // IWYU pragma: keep
#include <Credential.pb.h>
#include <Enums.pb.h>
#include <KeyCredential.pb.h>               // IWYU pragma: keep
#include <MasterCredentialParameters.pb.h>  // IWYU pragma: keep
#include <Signature.pb.h>
#include <VerificationSet.pb.h>  // IWYU pragma: keep
#include <cstdint>
#include <sstream>
#include <stdexcept>
#include <utility>

#include "internal/serialization/protobuf/Basic.hpp"
#include "internal/serialization/protobuf/Check.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "internal/serialization/protobuf/verify/ChildCredentialParameters.hpp"  // IWYU pragma: keep
#include "internal/serialization/protobuf/verify/ContactData.hpp"  // IWYU pragma: keep
#include "internal/serialization/protobuf/verify/KeyCredential.hpp"  // IWYU pragma: keep
#include "internal/serialization/protobuf/verify/MasterCredentialParameters.hpp"  // IWYU pragma: keep
#include "internal/serialization/protobuf/verify/Signature.hpp"  // IWYU pragma: keep
#include "internal/serialization/protobuf/verify/VerificationSet.hpp"  // IWYU pragma: keep
#include "internal/serialization/protobuf/verify/VerifyContacts.hpp"
#include "internal/serialization/protobuf/verify/VerifyCredentials.hpp"
#include "opentxs/util/Container.hpp"
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
    bool validChildData = false;
    bool validMasterData = false;
    bool validPublicData = false;
    bool validPrivateData = false;
    bool validContactData = false;
    bool expectMasterSignature = false;
    bool expectSourceSignature = false;
    int32_t expectedSigCount = 1;  // public self-signature
    bool checkRole = (CREDROLE_ERROR != role);

    if (!input.has_id()) { FAIL_1("missing identifier"); }

    if (MIN_PLAUSIBLE_IDENTIFIER > input.id().size()) {
        FAIL_2("invalid identifier", input.id());
    }

    if (!input.has_type()) { FAIL_1("missing type"); }

    switch (input.type()) {
        case CREDTYPE_LEGACY:
        case CREDTYPE_HD: {
        } break;
        case CREDTYPE_ERROR:
        default: {
            FAIL_2("invalid type", input.type());
        }
    }

    if (!input.has_role()) { FAIL_1("missing role"); }

    CredentialRole actualRole = input.role();

    if (checkRole && (role != actualRole)) {
        FAIL_2("incorrect role", input.role());
    }

    bool masterCredential = (CREDROLE_MASTERKEY == actualRole);
    bool childKeyCredential = (CREDROLE_CHILDKEY == actualRole);
    bool keyCredential = (masterCredential || childKeyCredential);
    bool contactCredential = (CREDROLE_CONTACT == actualRole);
    bool verifyCredential = (CREDROLE_VERIFY == actualRole);
    bool metadataContainer = (contactCredential || verifyCredential);
    bool knownRole = (keyCredential || metadataContainer);

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

    if (!input.has_nymid()) { FAIL_1("missing nym id"); }

    if (MIN_PLAUSIBLE_IDENTIFIER > input.nymid().size()) {
        FAIL_2("invalid nym id", input.nymid());
    }

    if (!masterCredential) {
        if (!input.has_childdata()) { FAIL_1("missing child data"); }

        try {
            validChildData = Check(
                input.childdata(),
                CredentialAllowedChildParams().at(input.version()).first,
                CredentialAllowedChildParams().at(input.version()).second,
                silent);

            if (!validChildData) { FAIL_1("invalid child data"); }
        } catch (const std::out_of_range&) {
            FAIL_2(
                "allowed child params version not defined for version",
                input.version());
        }
    }

    if (masterCredential) {
        if (!input.has_masterdata()) { FAIL_1("missing master data"); }

        try {
            validMasterData = Check(
                input.masterdata(),
                CredentialAllowedMasterParams().at(input.version()).first,
                CredentialAllowedMasterParams().at(input.version()).second,
                silent,
                expectSourceSignature);

            if (!validMasterData) { FAIL_1("invalid master data"); }
        } catch (const std::out_of_range&) {
            FAIL_2(
                "allowed master params version not defined for version",
                input.version());
        }

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
        if (input.has_verification()) {
            FAIL_1("contact credential contains verification data");
        }

        if (!input.has_contactdata()) { FAIL_1("missing contact data"); }

        try {
            validContactData = Check(
                input.contactdata(),
                CredentialAllowedContactData().at(input.version()).first,
                CredentialAllowedContactData().at(input.version()).second,
                silent,
                ClaimType::Normal);

            if (!validContactData) { FAIL_1("invalid contact data"); }
        } catch (const std::out_of_range&) {
            FAIL_2(
                "allowed contact data version not defined for version",
                input.version());
        }
    }

    if (verifyCredential) {
        if (input.has_contactdata()) {
            FAIL_1("verification credential contains contact data");
        }

        if (!input.has_verification()) { FAIL_1("missing verification data"); }

        try {
            bool validVerificationSet = Check(
                input.verification(),
                CredentialAllowedVerification().at(input.version()).first,
                CredentialAllowedVerification().at(input.version()).second,
                silent,
                VerificationType::Normal);

            if (!validVerificationSet) { FAIL_1("invalid verification data"); }
        } catch (const std::out_of_range&) {
            FAIL_2(
                "allowed verification version not defined for version",
                input.version());
        }
    }

    if (keyCredential) {
        try {
            validPublicData = Check(
                input.publiccredential(),
                CredentialAllowedKeyCredential().at(input.version()).first,
                CredentialAllowedKeyCredential().at(input.version()).second,
                silent,
                input.type(),
                KEYMODE_PUBLIC);

            if (!validPublicData) { FAIL_1("invalid public data"); }
        } catch (const std::out_of_range&) {
            FAIL_2(
                "allowed key credential version not defined for version",
                input.version());
        }

        if (isPrivate) {
            try {
                validPrivateData = Check(
                    input.privatecredential(),
                    CredentialAllowedKeyCredential().at(input.version()).first,
                    CredentialAllowedKeyCredential().at(input.version()).second,
                    silent,
                    input.type(),
                    KEYMODE_PRIVATE);

                if (!validPrivateData) { FAIL_1("invalid private data"); }
            } catch (const std::out_of_range&) {
                FAIL_2(
                    "allowed key credential version not defined for version",
                    input.version());
            }
        }
    }

    if (withSigs) {
        UnallocatedCString masterID = input.childdata().masterid();

        if (expectedSigCount != input.signature_size()) {
            std::stringstream ss;
            ss << input.signature_size() << " of " << expectedSigCount
               << " found";

            FAIL_2("incorrect number of signatures", ss.str());
        }

        uint32_t selfPublicCount = 0;
        uint32_t selfPrivateCount = 0;
        uint32_t masterPublicCount = 0;
        uint32_t sourcePublicCount = 0;

        for (const auto& it : input.signature()) {
            try {
                bool validSig = Check(
                    it,
                    CredentialAllowedSignatures().at(input.version()).first,
                    CredentialAllowedSignatures().at(input.version()).second,
                    silent,
                    input.id(),
                    masterID,
                    selfPublicCount,
                    selfPrivateCount,
                    masterPublicCount,
                    sourcePublicCount);

                if (!validSig) { FAIL_1("invalid signature"); }
            } catch (const std::out_of_range&) {
                FAIL_2(
                    "allowed signature version not defined for version",
                    input.version());
            }
        }

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
