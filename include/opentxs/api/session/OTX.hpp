// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <chrono>
#include <cstdint>
#include <future>
#include <memory>
#include <tuple>

#include "opentxs/Export.hpp"
#include "opentxs/core/UnitType.hpp"
#include "opentxs/core/contract/peer/Types.hpp"
#include "opentxs/otx/Types.hpp"
#include "opentxs/otx/client/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Time.hpp"

#define OT_CHEQUE_DAYS 30
#define OT_CHEQUE_HOURS 24 * OT_CHEQUE_DAYS
#define DEFAULT_PROCESS_INBOX_ITEMS 5

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace session
{
namespace internal
{
class OTX;
}  // namespace internal
}  // namespace session
}  // namespace api

namespace contract
{
class Server;
}  // namespace contract

namespace identifier
{
class Account;
class Generic;
class Nym;
class Notary;
class UnitDefinition;
}  // namespace identifier

class Amount;
class OTPayment;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::api::session
{
class OPENTXS_EXPORT OTX
{
public:
    using TaskID = int;
    using MessageID = identifier::Generic;
    using Result = std::pair<otx::LastReplyStatus, std::shared_ptr<Message>>;
    using Future = std::shared_future<Result>;
    using BackgroundTask = std::pair<TaskID, Future>;
    using Finished = std::shared_future<void>;

    virtual auto AcknowledgeBailment(
        const identifier::Nym& localNymID,
        const identifier::Notary& serverID,
        const identifier::Nym& targetNymID,
        const identifier::Generic& requestID,
        const UnallocatedCString& instructions,
        const otx::client::SetID setID = {}) const -> BackgroundTask = 0;
    virtual auto AcknowledgeNotice(
        const identifier::Nym& localNymID,
        const identifier::Notary& serverID,
        const identifier::Nym& recipientID,
        const identifier::Generic& requestID,
        const bool ack,
        const otx::client::SetID setID = {}) const -> BackgroundTask = 0;
    virtual auto AcknowledgeOutbailment(
        const identifier::Nym& localNymID,
        const identifier::Notary& serverID,
        const identifier::Nym& recipientID,
        const identifier::Generic& requestID,
        const UnallocatedCString& details,
        const otx::client::SetID setID = {}) const -> BackgroundTask = 0;
    virtual auto AcknowledgeConnection(
        const identifier::Nym& localNymID,
        const identifier::Notary& serverID,
        const identifier::Nym& recipientID,
        const identifier::Generic& requestID,
        const bool ack,
        const UnallocatedCString& url,
        const UnallocatedCString& login,
        const UnallocatedCString& password,
        const UnallocatedCString& key,
        const otx::client::SetID setID = {}) const -> BackgroundTask = 0;
    virtual auto AutoProcessInboxEnabled() const -> bool = 0;
    virtual auto CanDeposit(
        const identifier::Nym& recipientNymID,
        const OTPayment& payment) const -> otx::client::Depositability = 0;
    virtual auto CanDeposit(
        const identifier::Nym& recipientNymID,
        const identifier::Account& accountID,
        const OTPayment& payment) const -> otx::client::Depositability = 0;
    virtual auto CanMessage(
        const identifier::Nym& senderNymID,
        const identifier::Generic& recipientContactID,
        const bool startIntroductionServer = true) const
        -> otx::client::Messagability = 0;
    virtual auto CheckTransactionNumbers(
        const identifier::Nym& nym,
        const identifier::Notary& serverID,
        const std::size_t quantity) const -> bool = 0;
    virtual auto ContextIdle(
        const identifier::Nym& nym,
        const identifier::Notary& server) const -> Finished = 0;
    /** Deposit all available cheques for specified nym
     *
     *  \returns the number of cheques queued for deposit
     */
    virtual auto DepositCheques(const identifier::Nym& nymID) const
        -> std::size_t = 0;
    /** Deposit the specified list of cheques for specified nym
     *
     *  If the list of chequeIDs is empty, then all cheques will be deposited
     *
     *  \returns the number of cheques queued for deposit
     */
    virtual auto DepositCheques(
        const identifier::Nym& nymID,
        const UnallocatedSet<identifier::Generic>& chequeIDs) const
        -> std::size_t = 0;
    virtual auto DepositPayment(
        const identifier::Nym& recipientNymID,
        const std::shared_ptr<const OTPayment>& payment) const
        -> BackgroundTask = 0;
    virtual auto DepositPayment(
        const identifier::Nym& recipientNymID,
        const identifier::Account& accountID,
        const std::shared_ptr<const OTPayment>& payment) const
        -> BackgroundTask = 0;
    /** Used by unit tests */
    virtual void DisableAutoaccept() const = 0;
    virtual auto DownloadMint(
        const identifier::Nym& nym,
        const identifier::Notary& server,
        const identifier::UnitDefinition& unit) const -> BackgroundTask = 0;
    virtual auto DownloadNym(
        const identifier::Nym& localNymID,
        const identifier::Notary& serverID,
        const identifier::Nym& targetNymID) const -> BackgroundTask = 0;
    virtual auto DownloadNymbox(
        const identifier::Nym& localNymID,
        const identifier::Notary& serverID) const -> BackgroundTask = 0;
    virtual auto DownloadServerContract(
        const identifier::Nym& localNymID,
        const identifier::Notary& serverID,
        const identifier::Notary& contractID) const -> BackgroundTask = 0;
    virtual auto DownloadUnitDefinition(
        const identifier::Nym& localNymID,
        const identifier::Notary& serverID,
        const identifier::UnitDefinition& contractID) const
        -> BackgroundTask = 0;
    virtual auto FindNym(const identifier::Nym& nymID) const
        -> BackgroundTask = 0;
    virtual auto FindNym(
        const identifier::Nym& nymID,
        const identifier::Notary& serverIDHint) const -> BackgroundTask = 0;
    virtual auto FindServer(const identifier::Notary& serverID) const
        -> BackgroundTask = 0;
    virtual auto FindUnitDefinition(
        const identifier::UnitDefinition& unit) const -> BackgroundTask = 0;
    virtual auto InitiateBailment(
        const identifier::Nym& localNymID,
        const identifier::Notary& serverID,
        const identifier::Nym& targetNymID,
        const identifier::UnitDefinition& instrumentDefinitionID,
        const otx::client::SetID setID = {}) const -> BackgroundTask = 0;
    virtual auto InitiateOutbailment(
        const identifier::Nym& localNymID,
        const identifier::Notary& serverID,
        const identifier::Nym& targetNymID,
        const identifier::UnitDefinition& instrumentDefinitionID,
        const Amount amount,
        const UnallocatedCString& message,
        const otx::client::SetID setID = {}) const -> BackgroundTask = 0;
    virtual auto InitiateRequestConnection(
        const identifier::Nym& localNymID,
        const identifier::Notary& serverID,
        const identifier::Nym& targetNymID,
        const contract::peer::ConnectionInfoType& type,
        const otx::client::SetID setID = {}) const -> BackgroundTask = 0;
    virtual auto InitiateStoreSecret(
        const identifier::Nym& localNymID,
        const identifier::Notary& serverID,
        const identifier::Nym& targetNymID,
        const contract::peer::SecretType& type,
        const UnallocatedCString& primary,
        const UnallocatedCString& secondary,
        const otx::client::SetID setID = {}) const -> BackgroundTask = 0;
    OPENTXS_NO_EXPORT virtual auto Internal() const noexcept
        -> const internal::OTX& = 0;
    virtual auto IntroductionServer() const -> const identifier::Notary& = 0;
    virtual auto IssueUnitDefinition(
        const identifier::Nym& localNymID,
        const identifier::Notary& serverID,
        const identifier::UnitDefinition& unitID,
        const UnitType advertise = UnitType::Error,
        const UnallocatedCString& label = "") const -> BackgroundTask = 0;
    virtual auto MessageContact(
        const identifier::Nym& senderNymID,
        const identifier::Generic& contactID,
        const UnallocatedCString& message,
        const otx::client::SetID setID = {}) const -> BackgroundTask = 0;
    virtual auto MessageStatus(const TaskID taskID) const
        -> std::pair<otx::client::ThreadStatus, MessageID> = 0;
    virtual auto NotifyBailment(
        const identifier::Nym& localNymID,
        const identifier::Notary& serverID,
        const identifier::Nym& targetNymID,
        const identifier::UnitDefinition& instrumentDefinitionID,
        const identifier::Generic& requestID,
        const UnallocatedCString& txid,
        const Amount amount,
        const otx::client::SetID setID = {}) const -> BackgroundTask = 0;
    virtual auto PayContact(
        const identifier::Nym& senderNymID,
        const identifier::Generic& contactID,
        std::shared_ptr<const OTPayment> payment) const -> BackgroundTask = 0;
    virtual auto PayContactCash(
        const identifier::Nym& senderNymID,
        const identifier::Generic& contactID,
        const identifier::Generic& workflowID) const -> BackgroundTask = 0;
    virtual auto ProcessInbox(
        const identifier::Nym& localNymID,
        const identifier::Notary& serverID,
        const identifier::Account& accountID) const -> BackgroundTask = 0;
    virtual auto PublishServerContract(
        const identifier::Nym& localNymID,
        const identifier::Notary& serverID,
        const identifier::Generic& contractID) const -> BackgroundTask = 0;
    virtual void Refresh() const = 0;
    virtual auto RefreshCount() const -> std::uint64_t = 0;
    virtual auto RegisterAccount(
        const identifier::Nym& localNymID,
        const identifier::Notary& serverID,
        const identifier::UnitDefinition& unitID,
        const UnallocatedCString& label = "") const -> BackgroundTask = 0;
    virtual auto RegisterNym(
        const identifier::Nym& localNymID,
        const identifier::Notary& serverID,
        const bool resync = false) const -> BackgroundTask = 0;
    virtual auto RegisterNymPublic(
        const identifier::Nym& nymID,
        const identifier::Notary& server,
        const bool setContactData,
        const bool forcePrimary = false,
        const bool resync = false) const -> BackgroundTask = 0;
    virtual auto SetIntroductionServer(const contract::Server& contract) const
        -> identifier::Notary = 0;
    virtual auto SendCheque(
        const identifier::Nym& localNymID,
        const identifier::Account& sourceAccountID,
        const identifier::Generic& recipientContactID,
        const Amount value,
        const UnallocatedCString& memo,
        const Time validFrom = Clock::now(),
        const Time validTo =
            (Clock::now() + std::chrono::hours(OT_CHEQUE_HOURS))) const
        -> BackgroundTask = 0;
    virtual auto SendExternalTransfer(
        const identifier::Nym& localNymID,
        const identifier::Notary& serverID,
        const identifier::Account& sourceAccountID,
        const identifier::Account& targetAccountID,
        const Amount& value,
        const UnallocatedCString& memo) const -> BackgroundTask = 0;
    virtual auto SendTransfer(
        const identifier::Nym& localNymID,
        const identifier::Notary& serverID,
        const identifier::Account& sourceAccountID,
        const identifier::Account& targetAccountID,
        const Amount& value,
        const UnallocatedCString& memo) const -> BackgroundTask = 0;
    virtual void StartIntroductionServer(
        const identifier::Nym& localNymID) const = 0;
    virtual auto Status(const TaskID taskID) const
        -> otx::client::ThreadStatus = 0;
    virtual auto WithdrawCash(
        const identifier::Nym& nymID,
        const identifier::Notary& serverID,
        const identifier::Account& account,
        const Amount value) const -> BackgroundTask = 0;

    OPENTXS_NO_EXPORT virtual auto Internal() noexcept -> internal::OTX& = 0;

    OTX(const OTX&) = delete;
    OTX(OTX&&) = delete;
    auto operator=(const OTX&) -> OTX& = delete;
    auto operator=(OTX&&) -> OTX& = delete;

    OPENTXS_NO_EXPORT virtual ~OTX() = default;

protected:
    OTX() = default;
};
}  // namespace opentxs::api::session
