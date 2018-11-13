// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/api/Identity.hpp"
#include "opentxs/api/Core.hpp"
#include "opentxs/api/Wallet.hpp"
#include "opentxs/client/NymData.hpp"
#include "opentxs/core/crypto/ContactCredential.hpp"
#include "opentxs/core/crypto/VerificationCredential.hpp"
#include "opentxs/core/util/Assert.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/Nym.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/Types.hpp"

#include <list>
#include <memory>
#include <ostream>
#include <set>
#include <string>
#include <tuple>

#include "Identity.hpp"

// error from being unused.
#define OT_METHOD "opentxs::Identity::"

namespace opentxs
{
api::Identity* Factory::Identity(const api::Core& api)
{
    return new api::implementation::Identity(api);
}
}  // namespace opentxs

namespace opentxs::api::implementation
{
Identity::Identity(const api::Core& api)
    : api_(api)
{
    // WARNING: do not access api_.Wallet() during construction
}

bool Identity::AddInternalVerification(
    bool& changed,
    proto::VerificationSet& verifications,
    const Nym& onNym,
    const std::string& claimantNymID,
    const std::string& claimID,
    const ClaimPolarity polarity,
    const std::int64_t start,
    const std::int64_t end,
    const OTPasswordData* pPWData) const
{
    auto& internal =
        GetOrCreateInternalGroup(verifications, verifications.version());
    auto& identity = GetOrCreateVerificationIdentity(
        internal, claimantNymID, verifications.version());

    // check for an exact match
    const bool exists =
        HaveVerification(identity, claimID, polarity, start, end);

    if (!exists) {
        // removes all conflicting verifications
        DeleteVerification(changed, identity, claimID, start, end);

        auto& newVerification = *identity.add_verification();
        newVerification.set_version(identity.version());
        newVerification.set_claim(claimID);

        if (ClaimPolarity::POSITIVE == polarity) {
            newVerification.set_valid(true);
        } else {
            newVerification.set_valid(false);
        }

        newVerification.set_start(start);
        newVerification.set_end(end);

        changed = true;

        return Sign(newVerification, onNym, pPWData);
    }

    return true;
}

// Because we're building with protobuf-lite, we don't have library
// support for deleting items from protobuf repeated fields.
// Thus we delete by making a copy which excludes the item to be
// deleted.
void Identity::DeleteVerification(
    bool& changed,
    proto::VerificationIdentity& identity,
    const std::string& claimID,
    const std::int64_t start,
    const std::int64_t end) const
{
    proto::VerificationIdentity newData;
    newData.set_version(identity.version());
    newData.set_nym(identity.nym());

    for (auto& verification : identity.verification()) {
        // TODO:: Handle all of these cases correctly:
        // https://en.wikipedia.org/wiki/Allen's_interval_algebra#Relations
        if (!MatchVerification(verification, claimID, start, end)) {
            auto& it = *newData.add_verification();
            it = verification;
        } else {
            changed = true;
        }
    }

    if (changed) { identity = newData; }
}

proto::VerificationGroup& Identity::GetOrCreateInternalGroup(
    proto::VerificationSet& verificationSet,
    const std::uint32_t version) const
{
    const bool existing = verificationSet.has_internal();
    auto& output = *verificationSet.mutable_internal();

    if (!existing) { output.set_version(version); }

    return output;
}

proto::VerificationIdentity& Identity::GetOrCreateVerificationIdentity(
    proto::VerificationGroup& verificationGroup,
    const std::string& nym,
    const std::uint32_t version) const
{
    for (auto& identity : *verificationGroup.mutable_identity()) {
        if (identity.nym() == nym) { return identity; }
    }

    // We didn't find an existing item, so create a new one
    auto& identity = *verificationGroup.add_identity();
    identity.set_version(version);
    identity.set_nym(nym);

    return identity;
}

bool Identity::HaveVerification(
    proto::VerificationIdentity& identity,
    const std::string& claimID,
    const ClaimPolarity polarity,
    const std::int64_t start,
    const std::int64_t end) const
{
    bool match = false;

    if (ClaimPolarity::NEUTRAL != polarity) {
        bool valid = false;

        if (ClaimPolarity::POSITIVE == polarity) { valid = true; }

        for (auto& verification : identity.verification()) {
            if (verification.claim() != claimID) { break; }
            if (verification.valid() != valid) { break; }
            if (verification.start() != start) { break; }
            if (verification.end() != end) { break; }

            match = true;
        }
    }

    return match;
}

std::unique_ptr<proto::VerificationSet> Identity::InitializeVerificationSet(
    const std::uint32_t version) const
{
    std::unique_ptr<proto::VerificationSet> output(new proto::VerificationSet);

    if (output) {
        output->set_version(version);
    } else {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failed to instantiate verification set.")
            .Flush();
    }

    return output;
}

bool Identity::MatchVerification(
    const proto::Verification& item,
    const std::string& claimID,
    const std::int64_t start,
    const std::int64_t end) const
{
    // different claim
    if (item.claim() != claimID) { return false; }

    if ((item.start() == start) && (item.end() == end)) { return true; }

    // time range for claim falls outside given interval
    if (item.start() >= end) { return false; }
    if (item.end() <= start) { return false; }

    return true;
}

void Identity::PopulateVerificationIDs(proto::VerificationGroup& group) const
{
    for (auto& identity : *group.mutable_identity()) {
        for (auto& item : *identity.mutable_verification()) {
            const auto id = VerificationCredential::VerificationID(item);
            item.set_id(id);
        }
    }
}

bool Identity::RemoveInternalVerification(
    bool& changed,
    proto::VerificationSet& verifications,
    const std::string& claimantNymID,
    const std::string& claimID,
    const std::int64_t start,
    const std::int64_t end) const
{
    if (verifications.has_internal()) {
        auto& internalGroup = *verifications.mutable_internal();

        for (auto& identity : *internalGroup.mutable_identity()) {
            if (claimantNymID == identity.nym()) {
                DeleteVerification(changed, identity, claimID, start, end);
            }
        }
        // no internal verifications for the claimant nym means nothing to
        // delete
    }  // else: no internal verifications to delete means nothing to delete

    return true;
}

bool Identity::Sign(
    proto::Verification& plaintext,
    const Nym& nym,
    const OTPasswordData* pPWData) const
{
    plaintext.clear_sig();
    auto& signature = *plaintext.mutable_sig();
    signature.set_role(proto::SIGROLE_CLAIM);

    return nym.SignProto(plaintext, signature, pPWData);
}

std::unique_ptr<proto::VerificationSet> Identity::Verifications(
    const Nym& onNym) const
{
    auto output = onNym.VerificationSet();

    if (output) {
        if (output->has_internal()) {
            auto& group = *output->mutable_internal();
            PopulateVerificationIDs(group);
        }

        if (output->has_external()) {
            auto& group = *output->mutable_external();
            PopulateVerificationIDs(group);
        }
    }

    return output;
}

std::unique_ptr<proto::VerificationSet> Identity::Verify(
    NymData& onNym,
    bool& changed,
    const std::string& claimantNymID,
    const std::string& claimID,
    const ClaimPolarity polarity,
    const std::int64_t start,
    const std::int64_t end,
    const OTPasswordData* pPWData) const
{
    changed = false;
    std::unique_ptr<proto::VerificationSet> revised;
    auto existing = onNym.VerificationSet();

    if (existing) {
        revised.reset(new proto::VerificationSet(*existing));
    } else {
        changed = true;
        revised = InitializeVerificationSet();
    }

    if (!revised) { return revised; }

    bool finished = false;

    if (ClaimPolarity::NEUTRAL == polarity) {
        finished = RemoveInternalVerification(
            changed, *revised, claimantNymID, claimID, start, end);
    } else {
        finished = AddInternalVerification(
            changed,
            *revised,
            onNym.Nym(),
            claimantNymID,
            claimID,
            polarity,
            start,
            end,
            pPWData);
    }

    if (finished) {
        if (changed) {
            const bool updated = onNym.SetVerificationSet(*revised);

            if (updated) {
                api_.Wallet().Nym(onNym.asPublicNym());

                return revised;
            } else {
                LogOutput(OT_METHOD)(__FUNCTION__)(
                    ": Failed to update verification set.")
                    .Flush();
            }
        } else {
            return revised;
        }
    } else {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failed to add internal verification.")
            .Flush();
    }

    return nullptr;
}
}  // namespace opentxs::api::implementation
