/************************************************************
 *
 *                 OPEN TRANSACTIONS
 *
 *       Financial Cryptography and Digital Cash
 *       Library, Protocol, API, Server, CLI, GUI
 *
 *       -- Anonymous Numbered Accounts.
 *       -- Untraceable Digital Cash.
 *       -- Triple-Signed Receipts.
 *       -- Cheques, Vouchers, Transfers, Inboxes.
 *       -- Basket Currencies, Markets, Payment Plans.
 *       -- Signed, XML, Ricardian-style Contracts.
 *       -- Scripted smart contracts.
 *
 *  EMAIL:
 *  fellowtraveler@opentransactions.org
 *
 *  WEBSITE:
 *  http://www.opentransactions.org/
 *
 *  -----------------------------------------------------
 *
 *   LICENSE:
 *   This Source Code Form is subject to the terms of the
 *   Mozilla Public License, v. 2.0. If a copy of the MPL
 *   was not distributed with this file, You can obtain one
 *   at http://mozilla.org/MPL/2.0/.
 *
 *   DISCLAIMER:
 *   This program is distributed in the hope that it will
 *   be useful, but WITHOUT ANY WARRANTY; without even the
 *   implied warranty of MERCHANTABILITY or FITNESS FOR A
 *   PARTICULAR PURPOSE.  See the Mozilla Public License
 *   for more details.
 *
 ************************************************************/

#include "opentxs/stdafx.hpp"

#include "opentxs/client/NymData.hpp"

#include "opentxs/contact/Contact.hpp"
#include "opentxs/contact/ContactData.hpp"
#include "opentxs/contact/ContactItem.hpp"
#include "opentxs/core/crypto/PaymentCode.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/Nym.hpp"
#include "opentxs/core/String.hpp"

#define OT_METHOD "opentxs::NymData::"

namespace opentxs
{
NymData::NymData(const std::shared_ptr<class Nym>& nym)
    : nym_(nym)
{
}

std::string NymData::AddChildKeyCredential(
    const Identifier& strMasterID,
    const NymParameters& nymParameters)
{
    return nym().AddChildKeyCredential(strMasterID, nymParameters);
}

bool NymData::AddClaim(const Claim& claim) { return nym().AddClaim(claim); }

bool NymData::DeleteClaim(const Identifier& id)
{
    return nym().DeleteClaim(id);
}

bool NymData::AddContract(
    const std::string& instrumentDefinitionID,
    const proto::ContactItemType currency,
    const bool primary,
    const bool active)
{
    auto id = Identifier::Factory(instrumentDefinitionID);

    if (id->empty()) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Invalid instrument definition id." << std::endl;

        return false;
    }

    return nym().AddContract(id, currency, primary, active);
}

bool NymData::AddEmail(
    const std::string& value,
    const bool primary,
    const bool active)
{
    return nym().AddEmail(value, primary, active);
}

bool NymData::AddPaymentCode(
    const std::string& code,
    const proto::ContactItemType currency,
    const bool primary,
    const bool active)
{
    auto paymentCode = PaymentCode::Factory(code);

    if (false == paymentCode->VerifyInternally()) {
        otErr << OT_METHOD << __FUNCTION__ << ": Invalid payment code."
              << std::endl;

        return false;
    }

    return nym().AddPaymentCode(paymentCode, currency, primary, active);
}

bool NymData::AddPhoneNumber(
    const std::string& value,
    const bool primary,
    const bool active)
{
    return nym().AddPhoneNumber(value, primary, active);
}

bool NymData::AddPreferredOTServer(const std::string& id, const bool primary)
{
    if (id.empty()) {
        otErr << OT_METHOD << __FUNCTION__ << ": Invalid server id."
              << std::endl;

        return false;
    }

    return nym().AddPreferredOTServer(Identifier::Factory(id), primary);
}

bool NymData::AddSocialMediaProfile(
    const std::string& value,
    const proto::ContactItemType type,
    const bool primary,
    const bool active)
{
    return nym().AddSocialMediaProfile(value, type, primary, active);
}

const serializedCredentialIndex NymData::asPublicNym() const
{
    return nym_->asPublicNym();
}

std::string NymData::BestEmail() const { return Nym().BestEmail(); }

std::string NymData::BestPhoneNumber() const { return Nym().BestPhoneNumber(); }

std::string NymData::BestSocialMediaProfile(
    const proto::ContactItemType type) const
{
    return Nym().BestSocialMediaProfile(type);
}

const class ContactData& NymData::Claims() const { return nym_->Claims(); }

const ContactData& NymData::data() const
{
    OT_ASSERT(nym_);

    return nym_->Claims();
}

std::string NymData::EmailAddresses(bool active) const
{
    return Nym().EmailAddresses(active);
}

bool NymData::HaveContract(
    const Identifier& instrumentDefinitionID,
    const proto::ContactItemType currency,
    const bool primary,
    const bool active) const
{
    OT_ASSERT(nym_);

    const auto contracts = nym_->Contracts(currency, active);

    if (0 == contracts.size()) {

        return false;
    }

    const auto& data = nym_->Claims();

    for (const auto& id : contracts) {
        const auto& claim = data.Claim(id);

        OT_ASSERT(claim);

        const auto value = Identifier::Factory(claim->Value());

        if (false == (instrumentDefinitionID == value)) {

            continue;
        }

        if ((false == primary) || claim->isPrimary()) {

            return true;
        }
    }

    return false;
}

std::string NymData::Name() const
{
    OT_ASSERT(nym_);

    return nym_->Name();
}

const class Nym& NymData::Nym() const
{
    OT_ASSERT(nym_);

    return *nym_;
}

Nym& NymData::nym()
{
    OT_ASSERT(nym_);

    return *nym_;
}

std::string NymData::PaymentCode(const proto::ContactItemType currency) const
{
    return Contact::PaymentCode(data(), currency);
}

std::string NymData::PhoneNumbers(bool active) const
{
    return Nym().PhoneNumbers(active);
}

std::string NymData::PreferredOTServer() const
{
    return data().PreferredOTServer()->str();
}

std::string NymData::PrintContactData() const
{
    return ContactData::PrintContactData(data().Serialize(true));
}

bool NymData::SetAlias(const std::string& alias)
{
    return nym().SetAlias(alias);
}

bool NymData::SetContactData(const proto::ContactData& data)
{
    return nym().SetContactData(data);
}

bool NymData::SetScope(
    const proto::ContactItemType type,
    const std::string& name,
    const bool primary)
{
    return nym().SetScope(type, name, primary);
}

bool NymData::SetVerificationSet(const proto::VerificationSet& data)
{
    return nym().SetVerificationSet(data);
}

std::string NymData::SocialMediaProfiles(
    const proto::ContactItemType type,
    bool active) const
{
    return Nym().SocialMediaProfiles(type, active);
}

std::set<proto::ContactItemType> NymData::SocialMediaProfileTypes() const
{
    return Nym().SocialMediaProfileTypes();
}

proto::ContactItemType NymData::Type() const { return data().Type(); }

bool NymData::Valid() const { return bool(nym_); }

std::unique_ptr<proto::VerificationSet> NymData::VerificationSet() const
{
    return nym_->VerificationSet();
}
}  // namespace opentxs
