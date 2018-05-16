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

#ifndef OPENTXS_CONTACT_CONTACT_HPP
#define OPENTXS_CONTACT_CONTACT_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/Proto.hpp"
#include "opentxs/Types.hpp"

#include <atomic>
#include <cstdint>
#include <mutex>
#include <set>
#include <string>
#include <tuple>
#include <vector>

namespace opentxs
{
class Contact
{
public:
    typedef std::pair<proto::ContactItemType, std::string> BlockchainAddress;

    static std::shared_ptr<ContactItem> Best(const ContactGroup& group);
    static std::string ExtractLabel(const Nym& nym);
    static proto::ContactItemType ExtractType(const Nym& nym);
    static std::string PaymentCode(
        const ContactData& data,
        const proto::ContactItemType currency);

    Contact(
        const api::client::Wallet& wallet,
        const proto::Contact& serialized);
    Contact(const api::client::Wallet& wallet, const std::string& label);

    operator proto::Contact() const;
    Contact& operator+=(Contact& rhs);

    std::string BestEmail() const;
    std::string BestPhoneNumber() const;
    std::string BestSocialMediaProfile(const proto::ContactItemType type) const;
    std::vector<BlockchainAddress> BlockchainAddresses() const;
    std::shared_ptr<ContactData> Data() const;
    std::string EmailAddresses(bool active = true) const;
    const Identifier& ID() const;
    const std::string& Label() const;
    std::time_t LastUpdated() const;
    std::vector<OTIdentifier> Nyms(const bool includeInactive = false) const;
    std::string PaymentCode(
        const proto::ContactItemType currency = proto::CITEMTYPE_BTC) const;
    std::vector<std::string> PaymentCodes(
        const proto::ContactItemType currency = proto::CITEMTYPE_BTC) const;
    std::string PhoneNumbers(bool active = true) const;
    std::string Print() const;
    std::string SocialMediaProfiles(
        const proto::ContactItemType type,
        bool active = true) const;
    const std::set<proto::ContactItemType> SocialMediaProfileTypes() const;
    proto::ContactItemType Type() const;

    bool AddBlockchainAddress(
        const std::string& address,
        const proto::ContactItemType currency = proto::CITEMTYPE_BTC);
    bool AddEmail(
        const std::string& value,
        const bool primary,
        const bool active);
    bool AddNym(const std::shared_ptr<const Nym>& nym, const bool primary);
    bool AddNym(const Identifier& nymID, const bool primary);
    bool AddPaymentCode(
        const class PaymentCode& code,
        const bool primary,
        const proto::ContactItemType currency = proto::CITEMTYPE_BTC,
        const bool active = true);
    bool AddPhoneNumber(
        const std::string& value,
        const bool primary,
        const bool active);
    bool AddSocialMediaProfile(
        const std::string& value,
        const proto::ContactItemType type,
        const bool primary,
        const bool active);
    bool RemoveBlockchainAddress(
        const std::string& address,
        const proto::ContactItemType currency = proto::CITEMTYPE_BTC);
    bool RemoveNym(const Identifier& nymID);
    void SetLabel(const std::string& label);
    void Update(const proto::CredentialIndex& nym);

    ~Contact() = default;

private:
    const api::client::Wallet& wallet_;
    std::uint32_t version_{0};
    std::string label_{""};
    mutable std::mutex lock_{};
    const Identifier& id_;
    OTIdentifier parent_;
    OTIdentifier primary_nym_;
    std::map<OTIdentifier, std::shared_ptr<const Nym>> nyms_;
    std::set<OTIdentifier> merged_children_;
    std::unique_ptr<ContactData> contact_data_{};
    mutable std::shared_ptr<ContactData> cached_contact_data_{};
    std::atomic<std::uint64_t> revision_{0};

    static std::uint32_t check_version(
        const std::uint32_t in,
        const std::uint32_t targetVersion);

    std::shared_ptr<ContactGroup> payment_codes(
        const Lock& lock,
        const proto::ContactItemType currency) const;
    OTIdentifier generate_id() const;
    std::shared_ptr<ContactData> merged_data(const Lock& lock) const;
    proto::ContactItemType type(const Lock& lock) const;
    bool verify_write_lock(const Lock& lock) const;

    bool add_nym(
        const Lock& lock,
        const std::shared_ptr<const Nym>& nym,
        const bool primary);
    bool add_claim(const std::shared_ptr<ContactItem>& item);
    bool add_claim(const Lock& lock, const std::shared_ptr<ContactItem>& item);
    void add_nym_claim(
        const Lock& lock,
        const Identifier& nymID,
        const bool primary);
    void add_verified_claim(
        const Lock& lock,
        const std::shared_ptr<ContactItem>& item);
    void init_nyms();
    void update_label(const Lock& lock, const Nym& nym);

    Contact() = delete;
    Contact(const Contact&) = delete;
    Contact(Contact&&) = delete;
    Contact& operator=(const Contact&) = delete;
    Contact& operator=(Contact&&) = delete;
};
}  // namespace opentxs
#endif  // OPENTXS_CONTACT_CONTACT_HPP
