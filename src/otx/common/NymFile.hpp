// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>
#include <memory>

#include "internal/core/Core.hpp"
#include "internal/core/String.hpp"
#include "internal/util/Lockable.hpp"
#include "internal/util/Mutex.hpp"
#include "internal/util/Pimpl.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/identity/Nym.hpp"
#include "opentxs/identity/Types.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Session;
}  // namespace api
class Factory;
class Message;
class OTPassword;
class OTPayment;
class PasswordPrompt;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::implementation
{
using dequeOfMail = UnallocatedDeque<std::shared_ptr<Message>>;
using mapOfIdentifiers =
    UnallocatedMap<UnallocatedCString, identifier::Generic>;

class NymFile final : public opentxs::internal::NymFile, Lockable
{
public:
    auto CompareID(const identifier::Nym& rhs) const -> bool final;
    void DisplayStatistics(opentxs::String& strOutput) const final;
    auto GetInboxHash(
        const UnallocatedCString& acct_id,
        opentxs::identifier::Generic& theOutput) const
        -> bool final;  // client-side
    auto GetOutboxHash(
        const UnallocatedCString& acct_id,
        opentxs::identifier::Generic& theOutput) const
        -> bool final;  // client-side
    auto GetOutpaymentsByIndex(const std::int32_t nIndex) const
        -> std::shared_ptr<Message> final;
    auto GetOutpaymentsByTransNum(
        const std::int64_t lTransNum,
        const opentxs::PasswordPrompt& reason,
        std::unique_ptr<OTPayment>* pReturnPayment = nullptr,
        std::int32_t* pnReturnIndex = nullptr) const
        -> std::shared_ptr<Message> final;
    auto GetOutpaymentsCount() const -> std::int32_t final;
    auto GetUsageCredits() const -> const std::int64_t& final
    {
        sLock lock(shared_lock_);

        return usage_credits_;
    }
    auto ID() const -> const identifier::Nym& final
    {
        return target_nym_->ID();
    }
    auto PaymentCode() const -> UnallocatedCString final
    {
        return target_nym_->PaymentCode();
    }
    auto SerializeNymFile(opentxs::String& output) const -> bool final;

    void AddOutpayments(std::shared_ptr<Message> theMessage) final;
    auto GetSetAssetAccounts() -> UnallocatedSet<UnallocatedCString>& final
    {
        sLock lock(shared_lock_);

        return accounts_;
    }
    auto RemoveOutpaymentsByIndex(const std::int32_t nIndex) -> bool final;
    auto RemoveOutpaymentsByTransNum(
        const std::int64_t lTransNum,
        const opentxs::PasswordPrompt& reason) -> bool final;
    auto SaveSignedNymFile(const identity::Nym& SIGNER_NYM) -> bool;
    auto SetInboxHash(
        const UnallocatedCString& acct_id,
        const identifier::Generic& theInput) -> bool final;  // client-side
    auto SetOutboxHash(
        const UnallocatedCString& acct_id,
        const identifier::Generic& theInput) -> bool final;  // client-side
    void SetUsageCredits(const std::int64_t& lUsage) final
    {
        eLock lock(shared_lock_);

        usage_credits_ = lUsage;
    }

    NymFile() = delete;
    NymFile(const NymFile&) = delete;
    NymFile(NymFile&&) = delete;
    auto operator=(const NymFile&) -> NymFile& = delete;
    auto operator=(NymFile&&) -> NymFile& = delete;

    ~NymFile() final;

private:
    friend opentxs::Factory;

    const api::Session& api_;
    const Nym_p target_nym_{nullptr};
    const Nym_p signer_nym_{nullptr};
    std::int64_t usage_credits_{-1};
    bool mark_for_deletion_{false};
    OTString nym_file_;
    OTString version_;
    OTString description_;

    // Whenever client downloads Inbox, its hash is stored here. (When
    // downloading account, can compare ITS inbox hash to this one, to see if I
    // already have latest one.)
    mapOfIdentifiers inbox_hash_;
    // Whenever client downloads Outbox, its hash is stored here. (When
    // downloading account, can compare ITS outbox hash to this one, to see if I
    // already have latest one.)
    mapOfIdentifiers outbox_hash_;
    // Any outoing payments sent by this Nym. (And not yet deleted.) (payments
    // screen.)
    dequeOfMail outpayments_;
    // (SERVER side)
    // A list of asset account IDs. Server side only (client side uses wallet;
    // has multiple servers.)
    UnallocatedSet<UnallocatedCString> accounts_;

    auto GetHash(
        const mapOfIdentifiers& the_map,
        const UnallocatedCString& str_id,
        opentxs::identifier::Generic& theOutput) const -> bool;

    void ClearAll();
    auto DeserializeNymFile(
        const String& strNym,
        bool& converted,
        String::Map* pMapCredentials = nullptr,
        const OTPassword* pImportPassword = nullptr) -> bool;
    template <typename T>
    auto deserialize_nymfile(
        const T& lock,
        const opentxs::String& strNym,
        bool& converted,
        opentxs::String::Map* pMapCredentials,
        const OTPassword* pImportPassword = nullptr) -> bool;
    auto LoadSignedNymFile(const opentxs::PasswordPrompt& reason) -> bool final;
    template <typename T>
    auto load_signed_nymfile(
        const T& lock,
        const opentxs::PasswordPrompt& reason) -> bool;
    void RemoveAllNumbers(
        const opentxs::String& pstrNotaryID = String::Factory());
    auto SaveSignedNymFile(const opentxs::PasswordPrompt& reason) -> bool final;
    template <typename T>
    auto save_signed_nymfile(
        const T& lock,
        const opentxs::PasswordPrompt& reason) -> bool;
    template <typename T>
    auto serialize_nymfile(const T& lock, opentxs::String& strNym) const
        -> bool;
    auto SerializeNymFile(const char* szFoldername, const char* szFilename)
        -> bool;
    auto SetHash(
        mapOfIdentifiers& the_map,
        const UnallocatedCString& str_id,
        const identifier::Generic& theInput) -> bool;

    NymFile(const api::Session& api, Nym_p targetNym, Nym_p signerNym);
};
}  // namespace opentxs::implementation
