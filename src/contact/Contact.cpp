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

#include "opentxs/contact/Contact.hpp"

#include "opentxs/api/client/Wallet.hpp"
#include "opentxs/api/crypto/Crypto.hpp"
#include "opentxs/api/crypto/Encode.hpp"
#include "opentxs/api/Native.hpp"
#include "opentxs/contact/ContactData.hpp"
#include "opentxs/contact/ContactGroup.hpp"
#include "opentxs/contact/ContactItem.hpp"
#include "opentxs/contact/ContactSection.hpp"
#include "opentxs/core/crypto/PaymentCode.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/OT.hpp"
#include "opentxs/Types.hpp"

#include <sstream>
#include <stdexcept>

#define CURRENT_VERSION 2
#define ID_BYTES 32

#define OT_METHOD "opentxs::Contact::"

namespace opentxs
{
Contact::Contact(
    const api::client::Wallet& wallet,
    const proto::Contact& serialized)
    : wallet_(wallet)
    , version_(check_version(serialized.version(), CURRENT_VERSION))
    , label_(serialized.label())
    , lock_()
    , id_(Identifier::Factory(Identifier::Factory(serialized.id())))
    , parent_(Identifier::Factory(serialized.mergedto()))
    , primary_nym_(Identifier::Factory())
    , nyms_()
    , merged_children_()
    , contact_data_(new ContactData(
          serialized.id(),
          CONTACT_CONTACT_DATA_VERSION,
          CONTACT_CONTACT_DATA_VERSION,
          ContactData::SectionMap{}))
    , cached_contact_data_()
    , revision_(serialized.revision())
{
    if (serialized.has_contactdata()) {
        contact_data_.reset(new ContactData(
            serialized.id(),
            CONTACT_CONTACT_DATA_VERSION,
            serialized.contactdata()));
    }

    OT_ASSERT(contact_data_);

    for (const auto& child : serialized.merged()) {
        merged_children_.emplace(Identifier::Factory(child));
    }

    init_nyms();
}

Contact::Contact(const api::client::Wallet& wallet, const std::string& label)
    : wallet_(wallet)
    , version_(CURRENT_VERSION)
    , label_(label)
    , lock_()
    , id_(Identifier::Factory(generate_id()))
    , parent_(Identifier::Factory())
    , primary_nym_(Identifier::Factory())
    , nyms_()
    , merged_children_()
    , contact_data_(nullptr)
    , cached_contact_data_()
    , revision_(1)
{
    contact_data_.reset(new ContactData(
        String(id_).Get(),
        CONTACT_CONTACT_DATA_VERSION,
        CONTACT_CONTACT_DATA_VERSION,
        ContactData::SectionMap{}));

    OT_ASSERT(contact_data_);
}

Contact::operator proto::Contact() const
{
    Lock lock(lock_);
    proto::Contact output{};
    output.set_version(version_);
    output.set_id(String(id_).Get());
    output.set_revision(revision_);
    output.set_label(label_);

    if (contact_data_) {
        auto& data = *output.mutable_contactdata();
        data = contact_data_->Serialize();
    }

    output.set_mergedto(String(parent_).Get());

    for (const auto& child : merged_children_) {
        output.add_merged(String(child).Get());
    }

    return output;
}

Contact& Contact::operator+=(Contact& rhs)
{
    Lock rLock(rhs.lock_, std::defer_lock);
    Lock lock(lock_, std::defer_lock);
    std::lock(rLock, lock);

    if (label_.empty()) {
        label_ = rhs.label_;
    }

    rhs.parent_ = id_;

    if (primary_nym_->empty()) {
        primary_nym_ = rhs.primary_nym_;
    }

    for (const auto& it : rhs.nyms_) {
        const auto& id = it.first;
        const auto& nym = it.second;

        if (0 == nyms_.count(id)) {
            nyms_[id] = nym;
        }
    }

    rhs.nyms_.clear();

    for (const auto& it : rhs.merged_children_) {
        merged_children_.insert(it);
    }

    merged_children_.insert(rhs.id_);
    rhs.merged_children_.clear();

    if (contact_data_) {
        if (rhs.contact_data_) {
            contact_data_.reset(
                new ContactData(*contact_data_ + *rhs.contact_data_));
        }
    } else {
        if (rhs.contact_data_) {
            contact_data_.reset(new ContactData(*rhs.contact_data_));
        }
    }

    rhs.contact_data_.reset();
    cached_contact_data_.reset();
    rhs.cached_contact_data_.reset();

    return *this;
}

bool Contact::add_claim(const std::shared_ptr<ContactItem>& item)
{
    Lock lock(lock_);

    return add_claim(lock, item);
}

bool Contact::add_claim(
    const Lock& lock,
    const std::shared_ptr<ContactItem>& item)
{
    OT_ASSERT(verify_write_lock(lock));

    if (false == bool(item)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Null claim." << std::endl;

        return false;
    }

    const auto version = std::make_pair(item->Version(), item->Section());
    const proto::ContactItem serialized(*item);

    if (false == proto::Validate<proto::ContactItem>(
                     serialized, VERBOSE, true, version)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Invalid claim." << std::endl;

        return false;
    }

    add_verified_claim(lock, item);

    return true;
}

bool Contact::add_nym(
    const Lock& lock,
    const std::shared_ptr<const Nym>& nym,
    const bool primary)
{
    OT_ASSERT(verify_write_lock(lock));

    if (false == bool(nym)) {

        return false;
    }

    const auto contactType = type(lock);
    const auto nymType = ExtractType(*nym);
    const bool haveType = (proto::CITEMTYPE_ERROR != contactType) &&
                          (proto::CITEMTYPE_UNKNOWN != contactType);
    const bool typeMismatch = (contactType != nymType);

    if (haveType && typeMismatch) {
        otErr << OT_METHOD << __FUNCTION__ << ": Wrong nym type." << std::endl;

        return false;
    }

    const auto& id = nym->ID();
    const bool needPrimary = (0 == nyms_.size());
    const bool isPrimary = needPrimary || primary;
    nyms_[id] = nym;

    if (isPrimary) {
        primary_nym_ = id;
    }

    add_nym_claim(lock, id, isPrimary);

    return true;
}

void Contact::add_nym_claim(
    const Lock& lock,
    const Identifier& nymID,
    const bool primary)
{
    OT_ASSERT(verify_write_lock(lock));

    std::set<proto::ContactItemAttribute> attr{proto::CITEMATTR_LOCAL,
                                               proto::CITEMATTR_ACTIVE};

    if (primary) {
        attr.emplace(proto::CITEMATTR_PRIMARY);
    }

    std::shared_ptr<ContactItem> claim{nullptr};
    claim.reset(new ContactItem(
        String(id_).Get(),
        CONTACT_CONTACT_DATA_VERSION,
        CONTACT_CONTACT_DATA_VERSION,
        proto::CONTACTSECTION_RELATIONSHIP,
        proto::CITEMTYPE_CONTACT,
        String(nymID).Get(),
        attr,
        NULL_START,
        NULL_END));

    add_claim(lock, claim);
}

void Contact::add_verified_claim(
    const Lock& lock,
    const std::shared_ptr<ContactItem>& item)
{
    OT_ASSERT(verify_write_lock(lock));
    OT_ASSERT(contact_data_);

    contact_data_.reset(new ContactData(contact_data_->AddItem(item)));

    OT_ASSERT(contact_data_);

    revision_++;
    cached_contact_data_.reset();
}

bool Contact::AddBlockchainAddress(
    const std::string& address,
    const proto::ContactItemType currency)
{
    Lock lock(lock_);

    std::shared_ptr<ContactItem> claim{nullptr};
    claim.reset(new ContactItem(
        String(id_).Get(),
        CONTACT_CONTACT_DATA_VERSION,
        CONTACT_CONTACT_DATA_VERSION,
        proto::CONTACTSECTION_ADDRESS,
        currency,
        address,
        {proto::CITEMATTR_LOCAL, proto::CITEMATTR_ACTIVE},
        NULL_START,
        NULL_END));

    return add_claim(lock, claim);
}

bool Contact::AddEmail(
    const std::string& value,
    const bool primary,
    const bool active)
{
    if (value.empty()) {
        return false;
    }

    Lock lock(lock_);

    contact_data_.reset(
        new ContactData(contact_data_->AddEmail(value, primary, active)));

    OT_ASSERT(contact_data_);

    revision_++;
    cached_contact_data_.reset();

    return true;
}

bool Contact::AddNym(const std::shared_ptr<const Nym>& nym, const bool primary)
{
    Lock lock(lock_);

    return add_nym(lock, nym, primary);
}

bool Contact::AddNym(const Identifier& nymID, const bool primary)
{
    Lock lock(lock_);

    const bool needPrimary = (0 == nyms_.size());
    const bool isPrimary = needPrimary || primary;

    if (isPrimary) {
        primary_nym_ = nymID;
    }

    add_nym_claim(lock, nymID, isPrimary);

    revision_++;

    return true;
}

bool Contact::AddPaymentCode(
    const class PaymentCode& code,
    const bool primary,
    const proto::ContactItemType currency,
    const bool active)
{
    std::set<proto::ContactItemAttribute> attr{proto::CITEMATTR_LOCAL};

    if (active) {
        attr.emplace(proto::CITEMATTR_ACTIVE);
    }

    if (primary) {
        attr.emplace(proto::CITEMATTR_PRIMARY);
    }

    const std::string value = code.asBase58();
    std::shared_ptr<ContactItem> claim{nullptr};
    claim.reset(new ContactItem(
        String(id_).Get(),
        CONTACT_CONTACT_DATA_VERSION,
        CONTACT_CONTACT_DATA_VERSION,
        proto::CONTACTSECTION_PROCEDURE,
        currency,
        value,
        attr,
        NULL_START,
        NULL_END));

    if (false == add_claim(claim)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Unable to add claim."
              << std::endl;

        return false;
    }

    return true;
}

bool Contact::AddPhoneNumber(
    const std::string& value,
    const bool primary,
    const bool active)
{
    if (value.empty()) {
        return false;
    }

    Lock lock(lock_);

    contact_data_.reset(
        new ContactData(contact_data_->AddPhoneNumber(value, primary, active)));

    OT_ASSERT(contact_data_);

    revision_++;
    cached_contact_data_.reset();

    return true;
}

bool Contact::AddSocialMediaProfile(
    const std::string& value,
    const proto::ContactItemType type,
    const bool primary,
    const bool active)
{
    if (value.empty()) {
        return false;
    }

    Lock lock(lock_);

    contact_data_.reset(new ContactData(
        contact_data_->AddSocialMediaProfile(value, type, primary, active)));

    OT_ASSERT(contact_data_);

    revision_++;
    cached_contact_data_.reset();

    return true;
}

std::shared_ptr<ContactItem> Contact::Best(const ContactGroup& group)
{
    if (0 == group.Size()) {

        return {};
    }

    const auto primary = group.PrimaryClaim();

    if (primary) {

        return primary;
    }

    for (const auto& it : group) {
        const auto& claim = it.second;

        if (claim->isActive()) {

            return claim;
        }
    }

    return group.begin()->second;
}

std::string Contact::BestEmail() const
{
    Lock lock(lock_);
    const auto data = merged_data(lock);
    lock.unlock();

    if (false == bool(data)) {

        return {};
    }

    return data->BestEmail();
}

std::string Contact::BestPhoneNumber() const
{
    Lock lock(lock_);
    const auto data = merged_data(lock);
    lock.unlock();

    if (false == bool(data)) {

        return {};
    }

    return data->BestPhoneNumber();
}

std::string Contact::BestSocialMediaProfile(
    const proto::ContactItemType type) const
{
    Lock lock(lock_);
    const auto data = merged_data(lock);
    lock.unlock();

    if (false == bool(data)) {

        return {};
    }

    return data->BestSocialMediaProfile(type);
}

std::vector<Contact::BlockchainAddress> Contact::BlockchainAddresses() const
{
    std::vector<BlockchainAddress> output;
    Lock lock(lock_);
    auto data = merged_data(lock);
    lock.unlock();

    if (false == bool(data)) {

        return {};
    }

    const auto& version = data->Version();
    const auto section = data->Section(proto::CONTACTSECTION_ADDRESS);

    if (false == bool(section)) {

        return {};
    }

    for (const auto& it : *section) {
        const auto& type = it.first;
        const auto& group = it.second;

        OT_ASSERT(group);

        const bool currency = proto::ValidContactItemType(
            {version, proto::CONTACTSECTION_CONTRACT}, type);

        if (false == currency) {

            continue;
        }

        for (const auto& it : *group) {
            const auto& item = it.second;

            OT_ASSERT(item);

            output.push_back({type, item->Value()});
        }
    }

    return output;
}

std::uint32_t Contact::check_version(
    const std::uint32_t in,
    const std::uint32_t targetVersion)
{
    // Upgrade version
    if (targetVersion > in) {

        return targetVersion;
    }

    return in;
}

std::shared_ptr<ContactData> Contact::Data() const
{
    Lock lock(lock_);

    return merged_data(lock);
}

OTIdentifier Contact::generate_id() const
{
    auto& crypto = OT::App().Crypto().Encode();
    auto random = Data::Factory();
    crypto.Nonce(ID_BYTES, random);
    auto output = Identifier::Factory();
    output->CalculateDigest(random);

    return output;
}

std::string Contact::EmailAddresses(bool active) const
{
    Lock lock(lock_);
    const auto data = merged_data(lock);
    lock.unlock();

    if (false == bool(data)) {

        return {};
    }

    return data->EmailAddresses(active);
}

std::string Contact::ExtractLabel(const Nym& nym)
{
    return nym.Claims().Name();
}

proto::ContactItemType Contact::ExtractType(const Nym& nym)
{
    return nym.Claims().Type();
}

const Identifier& Contact::ID() const { return id_; }

void Contact::init_nyms()
{
    OT_ASSERT(contact_data_);

    const auto nyms = contact_data_->Group(
        proto::CONTACTSECTION_RELATIONSHIP, proto::CITEMTYPE_CONTACT);

    if (false == bool(nyms)) {

        return;
    }

    primary_nym_ = nyms->Primary();

    for (const auto& it : *nyms) {
        const auto& item = it.second;

        OT_ASSERT(item);

        const Identifier& nymID = Identifier::Factory(item->Value());
        auto& nym = nyms_[nymID];
        nym = wallet_.Nym(nymID);

        if (false == bool(nym)) {
            otErr << OT_METHOD << __FUNCTION__ << ": Failed to load nym "
                  << String(nymID) << std::endl;
        }
    }
}

const std::string& Contact::Label() const { return label_; }

std::time_t Contact::LastUpdated() const
{
    OT_ASSERT(contact_data_);

    const auto group = contact_data_->Group(
        proto::CONTACTSECTION_EVENT, proto::CITEMTYPE_REFRESHED);

    if (false == bool(group)) {

        return {};
    }

    const auto claim = group->PrimaryClaim();

    if (false == bool(claim)) {

        return {};
    }

    try {
        if (sizeof(int) == sizeof(std::time_t)) {

            return std::stoi(claim->Value());
        } else if (sizeof(long) == sizeof(std::time_t)) {

            return std::stol(claim->Value());
        } else if (sizeof(long long) == sizeof(std::time_t)) {

            return std::stoll(claim->Value());
        } else {
            OT_FAIL;
        }

    } catch (const std::out_of_range&) {

        return {};
    } catch (const std::invalid_argument&) {

        return {};
    }
}

std::shared_ptr<ContactData> Contact::merged_data(const Lock& lock) const
{
    OT_ASSERT(contact_data_);
    OT_ASSERT(verify_write_lock(lock));

    if (cached_contact_data_) {

        return cached_contact_data_;
    }

    cached_contact_data_.reset(new ContactData(*contact_data_));
    auto& output = cached_contact_data_;

    OT_ASSERT(output);

    if (false == primary_nym_->empty()) {
        try {
            auto& primary = nyms_.at(primary_nym_);

            if (primary) {
                output.reset(new ContactData(*output + primary->Claims()));
            }
        } catch (const std::out_of_range&) {
        }
    }

    for (const auto& it : nyms_) {
        const auto& nymID = it.first;
        const auto& nym = it.second;

        if (false == bool(nym)) {

            continue;
        }

        if (nymID == primary_nym_) {
            continue;
        }

        output.reset(new ContactData(*output + nym->Claims()));
    }

    return output;
}

std::vector<opentxs::OTIdentifier> opentxs::Contact::Nyms(
    const bool includeInactive) const
{
    Lock lock(lock_);
    const auto data = merged_data(lock);
    lock.unlock();

    if (false == bool(data)) {

        return {};
    }

    const auto group = data->Group(
        proto::CONTACTSECTION_RELATIONSHIP, proto::CITEMTYPE_CONTACT);

    if (false == bool(group)) {

        return {};
    }

    std::vector<OTIdentifier> output{};
    const auto& primaryID = group->Primary();

    for (const auto& it : *group) {
        const auto& item = it.second;

        OT_ASSERT(item);

        const auto& itemID = item->ID();

        if (false == (includeInactive || item->isActive())) {

            continue;
        }

        if (primaryID == itemID) {
            output.emplace(output.begin(), Identifier::Factory(item->Value()));
        } else {
            output.emplace(output.end(), Identifier::Factory(item->Value()));
        }
    }

    return output;
}

std::shared_ptr<ContactGroup> Contact::payment_codes(
    const Lock& lock,
    const proto::ContactItemType currency) const
{
    const auto data = merged_data(lock);

    if (false == bool(data)) {

        return {};
    }

    return data->Group(proto::CONTACTSECTION_PROCEDURE, currency);
}

std::string Contact::PaymentCode(
    const ContactData& data,
    const proto::ContactItemType currency)
{
    auto group = data.Group(proto::CONTACTSECTION_PROCEDURE, currency);

    if (false == bool(group)) {

        return {};
    }

    const auto item = Best(*group);

    if (false == bool(item)) {

        return {};
    }

    return item->Value();
}

std::string Contact::PaymentCode(const proto::ContactItemType currency) const
{
    Lock lock(lock_);
    const auto data = merged_data(lock);
    lock.unlock();

    if (false == bool(data)) {

        return {};
    }

    return PaymentCode(*data, currency);
}

std::vector<std::string> Contact::PaymentCodes(
    const proto::ContactItemType currency) const
{
    Lock lock(lock_);
    const auto group = payment_codes(lock, currency);
    lock.unlock();

    if (false == bool(group)) {

        return {};
    }

    std::vector<std::string> output{};

    for (const auto& it : *group) {
        OT_ASSERT(it.second);

        const auto& item = *it.second;
        output.emplace_back(item.Value());
    }

    return output;
}

std::string Contact::PhoneNumbers(bool active) const
{
    Lock lock(lock_);
    const auto data = merged_data(lock);
    lock.unlock();

    if (false == bool(data)) {

        return {};
    }

    return data->PhoneNumbers(active);
}

std::string Contact::Print() const
{
    Lock lock(lock_);
    std::stringstream out{};
    out << "Contact: " << String(id_).Get() << ", version " << version_
        << "revision " << revision_ << "\n"
        << "Label: " << label_ << "\n";

    if (false == parent_->empty()) {
        out << "Merged to: " << String(parent_).Get() << "\n";
    }

    if (false == merged_children_.empty()) {
        out << "Merged contacts:\n";

        for (const auto& id : merged_children_) {
            out << " * " << String(id).Get() << "\n";
        }
    }

    if (0 < nyms_.size()) {
        out << "Contains nyms:\n";

        for (const auto& it : nyms_) {
            const auto& id = it.first;
            out << " * " << String(id).Get();

            if (id == primary_nym_) {
                out << " (primary)";
            }

            out << "\n";
        }
    }

    auto data = merged_data(lock);

    if (data) {
        out << std::string(*data);
    }

    out << std::endl;

    return out.str();
}

bool Contact::RemoveNym(const Identifier& nymID)
{
    Lock lock(lock_);

    auto result = nyms_.erase(nymID);

    if (primary_nym_ == nymID) {
        primary_nym_ = Identifier::Factory();
    }

    return (0 < result);
}

void Contact::SetLabel(const std::string& label)
{
    Lock lock(lock_);
    label_ = label;
    revision_++;

    for (const auto& it : nyms_) {
        const auto& nymID = it.first;
        wallet_.SetNymAlias(nymID, label);
    }
}

std::string Contact::SocialMediaProfiles(
    const proto::ContactItemType type,
    bool active) const
{
    Lock lock(lock_);
    const auto data = merged_data(lock);
    lock.unlock();

    if (false == bool(data)) {

        return {};
    }

    return data->SocialMediaProfiles(type, active);
}

const std::set<proto::ContactItemType> Contact::SocialMediaProfileTypes() const
{
    Lock lock(lock_);
    const auto data = merged_data(lock);
    lock.unlock();

    return data->SocialMediaProfileTypes();
}

proto::ContactItemType Contact::type(const Lock& lock) const
{
    OT_ASSERT(verify_write_lock(lock));

    const auto data = merged_data(lock);

    if (false == bool(data)) {

        return proto::CITEMTYPE_ERROR;
    }

    return data->Type();
}

proto::ContactItemType Contact::Type() const
{
    Lock lock(lock_);

    return type(lock);
}

void Contact::Update(const proto::CredentialIndex& serialized)
{
    auto nym = wallet_.Nym(serialized);

    if (false == bool(nym)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Invalid serialized nym."
              << std::endl;

        return;
    }

    Lock lock(lock_);
    const auto& nymID = nym->ID();
    auto it = nyms_.find(nymID);

    if (nyms_.end() == it) {
        add_nym(lock, nym, false);
    } else {
        it->second = nym;
    }

    update_label(lock, *nym);
    std::shared_ptr<ContactItem> claim(new ContactItem(
        String(id_).Get(),
        CONTACT_CONTACT_DATA_VERSION,
        CONTACT_CONTACT_DATA_VERSION,
        proto::CONTACTSECTION_EVENT,
        proto::CITEMTYPE_REFRESHED,
        std::to_string(std::time(nullptr)),
        {proto::CITEMATTR_PRIMARY,
         proto::CITEMATTR_ACTIVE,
         proto::CITEMATTR_LOCAL},
        NULL_START,
        NULL_END));
    add_claim(lock, claim);
}

void Contact::update_label(const Lock& lock, const Nym& nym)
{
    OT_ASSERT(verify_write_lock(lock));

    if (false == label_.empty()) {

        return;
    }

    label_ = ExtractLabel(nym);
}

bool Contact::verify_write_lock(const Lock& lock) const
{
    if (lock.mutex() != &lock_) {
        otErr << OT_METHOD << __FUNCTION__ << ": Incorrect mutex." << std::endl;

        return false;
    }

    if (false == lock.owns_lock()) {
        otErr << OT_METHOD << __FUNCTION__ << ": Lock not owned." << std::endl;

        return false;
    }

    return true;
}
}  // namespace opentxs
