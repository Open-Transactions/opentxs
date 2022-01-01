// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <chrono>
#include <cstdint>
#include <future>
#include <memory>
#include <set>
#include <tuple>

#include "opentxs/Types.hpp"
#include "opentxs/core/UnitType.hpp"
#include "opentxs/core/contract/peer/Types.hpp"
#include "opentxs/core/identifier/Server.hpp"
#include "opentxs/otx/Types.hpp"
#include "opentxs/util/Time.hpp"

#define OT_CHEQUE_DAYS 30
#define OT_CHEQUE_HOURS 24 * OT_CHEQUE_DAYS
#define DEFAULT_PROCESS_INBOX_ITEMS 5

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
class Nym;
class Server;
class UnitDefinition;
}  // namespace identifier

class Amount;
class OTPayment;
}  // namespace opentxs

namespace opentxs::api::session
{
class OPENTXS_EXPORT OTX
{
public:
    using TaskID = int;
    using MessageID = OTIdentifier;
    using Result = std::pair<otx::LastReplyStatus, std::shared_ptr<Message>>;
    using Future = std::shared_future<Result>;
    using BackgroundTask = std::pair<TaskID, Future>;
    using Finished = std::shared_future<void>;

    virtual auto AcknowledgeBailment(
        const identifier::Nym& localNymID,
        const identifier::Server& serverID,
        const identifier::Nym& targetNymID,
        const Identifier& requestID,
        const std::string& instructions,
        const SetID setID = {}) const -> BackgroundTask = 0;
    virtual auto AcknowledgeNotice(
        const identifier::Nym& localNymID,
        const identifier::Server& serverID,
        const identifier::Nym& recipientID,
        const Identifier& requestID,
        const bool ack,
        const SetID setID = {}) const -> BackgroundTask = 0;
    virtual auto AcknowledgeOutbailment(
        const identifier::Nym& localNymID,
        const identifier::Server& serverID,
        const identifier::Nym& recipientID,
        const Identifier& requestID,
        const std::string& details,
        const SetID setID = {}) const -> BackgroundTask = 0;
    virtual auto AcknowledgeConnection(
        const identifier::Nym& localNymID,
        const identifier::Server& serverID,
        const identifier::Nym& recipientID,
        const Identifier& requestID,
        const bool ack,
        const std::string& url,
        const std::string& login,
        const std::string& password,
        const std::string& key,
        const SetID setID = {}) const -> BackgroundTask = 0;
    virtual auto AutoProcessInboxEnabled() const -> bool = 0;
    virtual auto CanDeposit(
        const identifier::Nym& recipientNymID,
        const OTPayment& payment) const -> Depositability = 0;
    virtual auto CanDeposit(
        const identifier::Nym& recipientNymID,
        const Identifier& accountID,
        const OTPayment& payment) const -> Depositability = 0;
    virtual auto CanMessage(
        const identifier::Nym& senderNymID,
        const Identifier& recipientContactID,
        const bool startIntroductionServer = true) const -> Messagability = 0;
    virtual auto CheckTransactionNumbers(
        const identifier::Nym& nym,
        const identifier::Server& serverID,
        const std::size_t quantity) const -> bool = 0;
    virtual auto ContextIdle(
        const identifier::Nym& nym,
        const identifier::Server& server) const -> Finished = 0;
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
        const std::pmr::set<OTIdentifier>& chequeIDs) const -> std::size_t = 0;
    virtual auto DepositPayment(
        const identifier::Nym& recipientNymID,
        const std::shared_ptr<const OTPayment>& payment) const
        -> BackgroundTask = 0;
    virtual auto DepositPayment(
        const identifier::Nym& recipientNymID,
        const Identifier& accountID,
        const std::shared_ptr<const OTPayment>& payment) const
        -> BackgroundTask = 0;
    /** Used by unit tests */
    virtual void DisableAutoaccept() const = 0;
    virtual auto DownloadMint(
        const identifier::Nym& nym,
        const identifier::Server& server,
        const identifier::UnitDefinition& unit) const -> BackgroundTask = 0;
    virtual auto DownloadNym(
        const identifier::Nym& localNymID,
        const identifier::Server& serverID,
        const identifier::Nym& targetNymID) const -> BackgroundTask = 0;
    virtual auto DownloadNymbox(
        const identifier::Nym& localNymID,
        const identifier::Server& serverID) const -> BackgroundTask = 0;
    virtual auto DownloadServerContract(
        const identifier::Nym& localNymID,
        const identifier::Server& serverID,
        const identifier::Server& contractID) const -> BackgroundTask = 0;
    virtual auto DownloadUnitDefinition(
        const identifier::Nym& localNymID,
        const identifier::Server& serverID,
        const identifier::UnitDefinition& contractID) const
        -> BackgroundTask = 0;
    virtual auto FindNym(const identifier::Nym& nymID) const
        -> BackgroundTask = 0;
    virtual auto FindNym(
        const identifier::Nym& nymID,
        const identifier::Server& serverIDHint) const -> BackgroundTask = 0;
    virtual auto FindServer(const identifier::Server& serverID) const
        -> BackgroundTask = 0;
    virtual auto FindUnitDefinition(
        const identifier::UnitDefinition& unit) const -> BackgroundTask = 0;
    virtual auto InitiateBailment(
        const identifier::Nym& localNymID,
        const identifier::Server& serverID,
        const identifier::Nym& targetNymID,
        const identifier::UnitDefinition& instrumentDefinitionID,
        const SetID setID = {}) const -> BackgroundTask = 0;
    virtual auto InitiateOutbailment(
        const identifier::Nym& localNymID,
        const identifier::Server& serverID,
        const identifier::Nym& targetNymID,
        const identifier::UnitDefinition& instrumentDefinitionID,
        const Amount amount,
        const std::string& message,
        const SetID setID = {}) const -> BackgroundTask = 0;
    virtual auto InitiateRequestConnection(
        const identifier::Nym& localNymID,
        const identifier::Server& serverID,
        const identifier::Nym& targetNymID,
        const contract::peer::ConnectionInfoType& type,
        const SetID setID = {}) const -> BackgroundTask = 0;
    virtual auto InitiateStoreSecret(
        const identifier::Nym& localNymID,
        const identifier::Server& serverID,
        const identifier::Nym& targetNymID,
        const contract::peer::SecretType& type,
        const std::string& primary,
        const std::string& secondary,
        const SetID setID = {}) const -> BackgroundTask = 0;
    OPENTXS_NO_EXPORT virtual auto Internal() const noexcept
        -> const internal::OTX& = 0;
    virtual auto IntroductionServer() const -> const identifier::Server& = 0;
    virtual auto IssueUnitDefinition(
        const identifier::Nym& localNymID,
        const identifier::Server& serverID,
        const identifier::UnitDefinition& unitID,
        const core::UnitType advertise = core::UnitType::Error,
        const std::string& label = "") const -> BackgroundTask = 0;
    virtual auto MessageContact(
        const identifier::Nym& senderNymID,
        const Identifier& contactID,
        const std::string& message,
        const SetID setID = {}) const -> BackgroundTask = 0;
    virtual auto MessageStatus(const TaskID taskID) const
        -> std::pair<ThreadStatus, MessageID> = 0;
    virtual auto NotifyBailment(
        const identifier::Nym& localNymID,
        const identifier::Server& serverID,
        const identifier::Nym& targetNymID,
        const identifier::UnitDefinition& instrumentDefinitionID,
        const Identifier& requestID,
        const std::string& txid,
        const Amount amount,
        const SetID setID = {}) const -> BackgroundTask = 0;
    virtual auto PayContact(
        const identifier::Nym& senderNymID,
        const Identifier& contactID,
        std::shared_ptr<const OTPayment> payment) const -> BackgroundTask = 0;
    virtual auto PayContactCash(
        const identifier::Nym& senderNymID,
        const Identifier& contactID,
        const Identifier& workflowID) const -> BackgroundTask = 0;
    virtual auto ProcessInbox(
        const identifier::Nym& localNymID,
        const identifier::Server& serverID,
        const Identifier& accountID) const -> BackgroundTask = 0;
    virtual auto PublishServerContract(
        const identifier::Nym& localNymID,
        const identifier::Server& serverID,
        const Identifier& contractID) const -> BackgroundTask = 0;
    virtual void Refresh() const = 0;
    virtual auto RefreshCount() const -> std::uint64_t = 0;
    virtual auto RegisterAccount(
        const identifier::Nym& localNymID,
        const identifier::Server& serverID,
        const identifier::UnitDefinition& unitID,
        const std::string& label = "") const -> BackgroundTask = 0;
    virtual auto RegisterNym(
        const identifier::Nym& localNymID,
        const identifier::Server& serverID,
        const bool resync = false) const -> BackgroundTask = 0;
    virtual auto RegisterNymPublic(
        const identifier::Nym& nymID,
        const identifier::Server& server,
        const bool setContactData,
        const bool forcePrimary = false,
        const bool resync = false) const -> BackgroundTask = 0;
    virtual auto SetIntroductionServer(const contract::Server& contract) const
        -> OTServerID = 0;
    virtual auto SendCheque(
        const identifier::Nym& localNymID,
        const Identifier& sourceAccountID,
        const Identifier& recipientContactID,
        const Amount value,
        const std::string& memo,
        const Time validFrom = Clock::now(),
        const Time validTo =
            (Clock::now() + std::chrono::hours(OT_CHEQUE_HOURS))) const
        -> BackgroundTask = 0;
    virtual auto SendExternalTransfer(
        const identifier::Nym& localNymID,
        const identifier::Server& serverID,
        const Identifier& sourceAccountID,
        const Identifier& targetAccountID,
        const Amount& value,
        const std::string& memo) const -> BackgroundTask = 0;
    virtual auto SendTransfer(
        const identifier::Nym& localNymID,
        const identifier::Server& serverID,
        const Identifier& sourceAccountID,
        const Identifier& targetAccountID,
        const Amount& value,
        const std::string& memo) const -> BackgroundTask = 0;
    virtual void StartIntroductionServer(
        const identifier::Nym& localNymID) const = 0;
    virtual auto Status(const TaskID taskID) const -> ThreadStatus = 0;
    virtual auto WithdrawCash(
        const identifier::Nym& nymID,
        const identifier::Server& serverID,
        const Identifier& account,
        const Amount value) const -> BackgroundTask = 0;

    OPENTXS_NO_EXPORT virtual auto Internal() noexcept -> internal::OTX& = 0;

    OPENTXS_NO_EXPORT virtual ~OTX() = default;

protected:
    OTX() = default;

private:
    OTX(const OTX&) = delete;
    OTX(OTX&&) = delete;
    auto operator=(const OTX&) -> OTX& = delete;
    auto operator=(OTX&&) -> OTX& = delete;
};
}  // namespace opentxs::api::session
