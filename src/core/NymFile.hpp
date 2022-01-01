// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <algorithm>
#include <cstdint>
#include <deque>
#include <map>
#include <memory>
#include <set>
#include <string>

#include "internal/core/Core.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/core/Lockable.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/identity/Nym.hpp"
#include "opentxs/util/Pimpl.hpp"

namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

namespace identifier
{
class Nym;
}  // namespace identifier

class Factory;
class Message;
class OTPassword;
class OTPayment;
class PasswordPrompt;
}  // namespace opentxs

namespace opentxs::implementation
{
using dequeOfMail = std::pmr::deque<std::shared_ptr<Message>>;
using mapOfIdentifiers = std::pmr::map<std::string, OTIdentifier>;

class NymFile final : public opentxs::internal::NymFile, Lockable
{
public:
    auto CompareID(const identifier::Nym& rhs) const -> bool final;
    void DisplayStatistics(String& strOutput) const final;
    auto GetInboxHash(
        const std::string& acct_id,
        Identifier& theOutput) const -> bool final;  // client-side
    auto GetOutboxHash(
        const std::string& acct_id,
        Identifier& theOutput) const -> bool final;  // client-side
    auto GetOutpaymentsByIndex(const std::int32_t nIndex) const
        -> std::shared_ptr<Message> final;
    auto GetOutpaymentsByTransNum(
        const std::int64_t lTransNum,
        const PasswordPrompt& reason,
        std::unique_ptr<OTPayment>* pReturnPayment = nullptr,
        std::int32_t* pnReturnIndex = nullptr) const
        -> std::shared_ptr<Message> final;
    auto GetOutpaymentsCount() const -> std::int32_t final;
    auto GetUsageCredits() const -> const std::int64_t& final
    {
        sLock lock(shared_lock_);

        return m_lUsageCredits;
    }
    auto ID() const -> const identifier::Nym& final
    {
        return target_nym_->ID();
    }
    auto PaymentCode() const -> std::string final
    {
        return target_nym_->PaymentCode();
    }
    auto SerializeNymFile(String& output) const -> bool final;

    void AddOutpayments(std::shared_ptr<Message> theMessage) final;
    auto GetSetAssetAccounts() -> std::pmr::set<std::string>& final
    {
        sLock lock(shared_lock_);

        return m_setAccounts;
    }
    auto RemoveOutpaymentsByIndex(const std::int32_t nIndex) -> bool final;
    auto RemoveOutpaymentsByTransNum(
        const std::int64_t lTransNum,
        const PasswordPrompt& reason) -> bool final;
    auto SaveSignedNymFile(const identity::Nym& SIGNER_NYM) -> bool;
    auto SetInboxHash(
        const std::string& acct_id,
        const Identifier& theInput) -> bool final;  // client-side
    auto SetOutboxHash(
        const std::string& acct_id,
        const Identifier& theInput) -> bool final;  // client-side
    void SetUsageCredits(const std::int64_t& lUsage) final
    {
        eLock lock(shared_lock_);

        m_lUsageCredits = lUsage;
    }

    ~NymFile() final;

private:
    friend opentxs::Factory;

    const api::Session& api_;
    const Nym_p target_nym_{nullptr};
    const Nym_p signer_nym_{nullptr};
    std::int64_t m_lUsageCredits{-1};
    bool m_bMarkForDeletion{false};
    OTString m_strNymFile;
    OTString m_strVersion;
    OTString m_strDescription;

    // Whenever client downloads Inbox, its hash is stored here. (When
    // downloading account, can compare ITS inbox hash to this one, to see if I
    // already have latest one.)
    mapOfIdentifiers m_mapInboxHash;
    // Whenever client downloads Outbox, its hash is stored here. (When
    // downloading account, can compare ITS outbox hash to this one, to see if I
    // already have latest one.)
    mapOfIdentifiers m_mapOutboxHash;
    // Any outoing payments sent by this Nym. (And not yet deleted.) (payments
    // screen.)
    dequeOfMail m_dequeOutpayments;
    // (SERVER side)
    // A list of asset account IDs. Server side only (client side uses wallet;
    // has multiple servers.)
    std::pmr::set<std::string> m_setAccounts;

    auto GetHash(
        const mapOfIdentifiers& the_map,
        const std::string& str_id,
        Identifier& theOutput) const -> bool;

    void ClearAll();
    auto DeserializeNymFile(
        const String& strNym,
        bool& converted,
        String::Map* pMapCredentials = nullptr,
        const OTPassword* pImportPassword = nullptr) -> bool;
    template <typename T>
    auto deserialize_nymfile(
        const T& lock,
        const String& strNym,
        bool& converted,
        String::Map* pMapCredentials,
        const OTPassword* pImportPassword = nullptr) -> bool;
    auto LoadSignedNymFile(const PasswordPrompt& reason) -> bool final;
    template <typename T>
    auto load_signed_nymfile(const T& lock, const PasswordPrompt& reason)
        -> bool;
    void RemoveAllNumbers(const String& pstrNotaryID = String::Factory());
    auto SaveSignedNymFile(const PasswordPrompt& reason) -> bool final;
    template <typename T>
    auto save_signed_nymfile(const T& lock, const PasswordPrompt& reason)
        -> bool;
    template <typename T>
    auto serialize_nymfile(const T& lock, String& strNym) const -> bool;
    auto SerializeNymFile(const char* szFoldername, const char* szFilename)
        -> bool;
    auto SetHash(
        mapOfIdentifiers& the_map,
        const std::string& str_id,
        const Identifier& theInput) -> bool;

    NymFile(const api::Session& api, Nym_p targetNym, Nym_p signerNym);
    NymFile() = delete;
    NymFile(const NymFile&) = delete;
    NymFile(NymFile&&) = delete;
    auto operator=(const NymFile&) -> NymFile& = delete;
    auto operator=(NymFile&&) -> NymFile& = delete;
};
}  // namespace opentxs::implementation
