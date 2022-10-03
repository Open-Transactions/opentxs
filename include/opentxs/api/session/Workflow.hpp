// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/otx/blind/Purse.hpp"

#pragma once

#include <memory>

#include "opentxs/Export.hpp"
#include "opentxs/otx/client/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace session
{
namespace internal
{
class Workflow;
}  // namespace internal
}  // namespace session

class Session;
}  // namespace api

namespace identifier
{
class Generic;
class Nym;
class Notary;
}  // namespace identifier

namespace otx
{
namespace blind
{
class Purse;
}  // namespace blind
}  // namespace otx

namespace proto
{
class PaymentWorkflow;
class Purse;
}  // namespace proto

class Cheque;
class Item;
class OTTransaction;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::api::session
{
/** Store and retrieve payment workflow events
 *
 *  Sequence for sending a cheque:
 *    1. WriteCheque
 *    2. SendCheque or ExportCheque
 *       a. SendCheque may be repeated as necessary until successful
 *    3. (optional) ExpireCheque
 *       a. May be performed after step 1 or after step 2
 *       b. It's possible that a cheque that's been expired locally was already
 *          accepted by the notary
 *    4. (optional) CancelCheque
 *       a. May be performed after step 1 or after step 2
 *    5. ClearCheque
 *       a. Called after receiving a cheque deposit receipt
 *    6. FinishCheque
 *       a. Called after a process inbox attempt
 *       b. May be repeated as necessary until successful
 *
 *  Sequence for receiving a cheque:
 *    1. ReceiveCheque or ImportCheque
 *    2. (optional) ExpireCheque
 *    3. DepositCheque
 *       a. May be repeated as necessary until successful
 *
 *  Sequence for sending a transfer:
 *    1. CreateTransfer
 *       a. Called just before sending a notarizeTransaction message to notary.
 *    2. AcknowledgeTransfer
 *       a. Called after receiving a server response with a successful
 *          transaction reply
 *    3. AbortTransfer
 *       a. Called after receiving a server response with an unsuccessful
 *          transaction reply, or after proving the server did not receive the
 *          original transaction
 *    4. ClearTransfer
 *       a. Called after receiving a transfer receipt
 *    5. CompleteTransfer
 *       a. Called after a process inbox attempt
 *       b. May be repeated as necessary until successful
 *
 *  Sequence for receiving a transfer:
 *    1. ConveyTransfer
 *       a. Called after receiving a transfer
 *    3. AcceptTransfer
 *       a. May be repeated as necessary until successful
 *
 *  Sequence for an internal transfer:
 *    NOTE: AcknowledgeTransfer and ConveyTransfer may be called out of order
 *
 *    1. CreateTransfer
 *       a. Called just before sending a notarizeTransaction message to notary.
 *    2. AcknowledgeTransfer
 *       a. Called after receiving a server response with a successful
 *          transaction reply
 *    3. AbortTransfer
 *       a. Called after receiving a server response with an unsuccessful
 *          transaction reply, or after proving the server did not receive the
 *          original transaction
 *    4. ConveyTransfer
 *       a. Called after receiving a transfer
 *    5. ClearTransfer
 *       a. Called after receiving a transfer receipt
 *    6. CompleteTransfer
 *       a. Called after a process inbox attempt
 *       b. May be repeated as necessary until successful
 *
 *  Sequence for sending cash
 *
 *    1. AllocateCash
 *    2. SendCash
 *
 *  Sequence for receiving cash
 *
 *    1. ReceiveCash
 *    2. AcceptCash
 *    2. RejectCash
 *
 */
class OPENTXS_EXPORT Workflow
{
public:
    using Cheque = std::pair<
        otx::client::PaymentWorkflowState,
        std::unique_ptr<opentxs::Cheque>>;
    using Purse =
        std::pair<otx::client::PaymentWorkflowState, otx::blind::Purse>;
    using Transfer = std::
        pair<otx::client::PaymentWorkflowState, std::unique_ptr<opentxs::Item>>;

    OPENTXS_NO_EXPORT static auto ContainsCash(
        const proto::PaymentWorkflow& workflow) -> bool;
    OPENTXS_NO_EXPORT static auto ContainsCheque(
        const proto::PaymentWorkflow& workflow) -> bool;
    OPENTXS_NO_EXPORT static auto ContainsTransfer(
        const proto::PaymentWorkflow& workflow) -> bool;
    OPENTXS_NO_EXPORT static auto ExtractCheque(
        const proto::PaymentWorkflow& workflow) -> UnallocatedCString;
    OPENTXS_NO_EXPORT static auto ExtractPurse(
        const proto::PaymentWorkflow& workflow,
        proto::Purse& out) -> bool;
    OPENTXS_NO_EXPORT static auto ExtractTransfer(
        const proto::PaymentWorkflow& workflow) -> UnallocatedCString;
    OPENTXS_NO_EXPORT static auto InstantiateCheque(
        const api::Session& api,
        const proto::PaymentWorkflow& workflow) -> Cheque;
    OPENTXS_NO_EXPORT static auto InstantiatePurse(
        const api::Session& api,
        const proto::PaymentWorkflow& workflow) -> Purse;
    OPENTXS_NO_EXPORT static auto InstantiateTransfer(
        const api::Session& api,
        const proto::PaymentWorkflow& workflow) -> Transfer;
    OPENTXS_NO_EXPORT static auto UUID(
        const api::Session& api,
        const proto::PaymentWorkflow& workflown) -> identifier::Generic;
    static auto UUID(
        const api::Session& api,
        const identifier::Generic& notary,
        const TransactionNumber& number) -> identifier::Generic;

    /** Record a failed transfer attempt */
    virtual auto AbortTransfer(
        const identifier::Nym& nymID,
        const Item& transfer,
        const Message& reply) const -> bool = 0;
    /** Record a transfer accept, or accept attempt */
    virtual auto AcceptTransfer(
        const identifier::Nym& nymID,
        const identifier::Notary& notaryID,
        const OTTransaction& pending,
        const Message& reply) const -> bool = 0;
    /** Record a successful transfer attempt */
    virtual auto AcknowledgeTransfer(
        const identifier::Nym& nymID,
        const Item& transfer,
        const Message& reply) const -> bool = 0;
    virtual auto AllocateCash(
        const identifier::Nym& id,
        const otx::blind::Purse& purse) const -> identifier::Generic = 0;
    /** Record a cheque cancellation or cancellation attempt */
    virtual auto CancelCheque(
        const opentxs::Cheque& cheque,
        const Message& request,
        const Message* reply) const -> bool = 0;
    /** Record a cheque deposit receipt */
    virtual auto ClearCheque(
        const identifier::Nym& recipientNymID,
        const OTTransaction& receipt) const -> bool = 0;
    /** Record receipt of a transfer receipt */
    virtual auto ClearTransfer(
        const identifier::Nym& nymID,
        const identifier::Notary& notaryID,
        const OTTransaction& receipt) const -> bool = 0;
    /** Record a process inbox for sender that accepts a transfer receipt */
    virtual auto CompleteTransfer(
        const identifier::Nym& nymID,
        const identifier::Notary& notaryID,
        const OTTransaction& receipt,
        const Message& reply) const -> bool = 0;
    /** Create a new incoming transfer workflow, or update an existing internal
     *  transfer workflow. */
    virtual auto ConveyTransfer(
        const identifier::Nym& nymID,
        const identifier::Notary& notaryID,
        const OTTransaction& pending) const -> identifier::Generic = 0;
    /** Record a new outgoing or internal "sent transfer" (or attempt) workflow
     */
    virtual auto CreateTransfer(const Item& transfer, const Message& request)
        const -> identifier::Generic = 0;
    /** Record a cheque deposit or deposit attempt */
    virtual auto DepositCheque(
        const identifier::Nym& nymID,
        const identifier::Generic& accountID,
        const opentxs::Cheque& cheque,
        const Message& request,
        const Message* reply) const -> bool = 0;
    /** Mark a cheque workflow as expired */
    virtual auto ExpireCheque(
        const identifier::Nym& nymID,
        const opentxs::Cheque& cheque) const -> bool = 0;
    /** Record an out of band cheque conveyance */
    virtual auto ExportCheque(const opentxs::Cheque& cheque) const -> bool = 0;
    /** Record a process inbox that accepts a cheque deposit receipt */
    virtual auto FinishCheque(
        const opentxs::Cheque& cheque,
        const Message& request,
        const Message* reply) const -> bool = 0;
    /** Create a new incoming cheque workflow from an out of band cheque */
    virtual auto ImportCheque(
        const identifier::Nym& nymID,
        const opentxs::Cheque& cheque) const -> identifier::Generic = 0;
    virtual auto InstantiateCheque(
        const identifier::Nym& nymID,
        const identifier::Generic& workflowID) const -> Cheque = 0;
    virtual auto InstantiatePurse(
        const identifier::Nym& nymID,
        const identifier::Generic& workflowID) const -> Purse = 0;
    OPENTXS_NO_EXPORT virtual auto Internal() const noexcept
        -> const internal::Workflow& = 0;
    virtual auto List(
        const identifier::Nym& nymID,
        const otx::client::PaymentWorkflowType type,
        const otx::client::PaymentWorkflowState state) const
        -> UnallocatedSet<identifier::Generic> = 0;
    virtual auto LoadCheque(
        const identifier::Nym& nymID,
        const identifier::Generic& chequeID) const -> Cheque = 0;
    virtual auto LoadChequeByWorkflow(
        const identifier::Nym& nymID,
        const identifier::Generic& workflowID) const -> Cheque = 0;
    virtual auto LoadTransfer(
        const identifier::Nym& nymID,
        const identifier::Generic& transferID) const -> Transfer = 0;
    virtual auto LoadTransferByWorkflow(
        const identifier::Nym& nymID,
        const identifier::Generic& workflowID) const -> Transfer = 0;
    /** Load a serialized workflow, if it exists*/
    OPENTXS_NO_EXPORT virtual auto LoadWorkflow(
        const identifier::Nym& nymID,
        const identifier::Generic& workflowID,
        proto::PaymentWorkflow& out) const -> bool = 0;
    virtual auto ReceiveCash(
        const identifier::Nym& receiver,
        const otx::blind::Purse& purse,
        const Message& message) const -> identifier::Generic = 0;
    /** Create a new incoming cheque workflow from an OT message */
    virtual auto ReceiveCheque(
        const identifier::Nym& nymID,
        const opentxs::Cheque& cheque,
        const Message& message) const -> identifier::Generic = 0;
    virtual auto SendCash(
        const identifier::Nym& sender,
        const identifier::Nym& recipient,
        const identifier::Generic& workflowID,
        const Message& request,
        const Message* reply) const -> bool = 0;
    /** Record a send or send attempt via an OT notary */
    virtual auto SendCheque(
        const opentxs::Cheque& cheque,
        const Message& request,
        const Message* reply) const -> bool = 0;
    virtual auto WorkflowParty(
        const identifier::Nym& nymID,
        const identifier::Generic& workflowID,
        const int index) const -> const UnallocatedCString = 0;
    virtual auto WorkflowPartySize(
        const identifier::Nym& nymID,
        const identifier::Generic& workflowID,
        int& partysize) const -> bool = 0;
    virtual auto WorkflowState(
        const identifier::Nym& nymID,
        const identifier::Generic& workflowID) const
        -> otx::client::PaymentWorkflowState = 0;
    virtual auto WorkflowType(
        const identifier::Nym& nymID,
        const identifier::Generic& workflowID) const
        -> otx::client::PaymentWorkflowType = 0;
    /** Get a list of workflow IDs relevant to a specified account */
    virtual auto WorkflowsByAccount(
        const identifier::Nym& nymID,
        const identifier::Generic& accountID) const
        -> UnallocatedVector<identifier::Generic> = 0;
    /** Create a new outgoing cheque workflow */
    virtual auto WriteCheque(const opentxs::Cheque& cheque) const
        -> identifier::Generic = 0;

    OPENTXS_NO_EXPORT virtual auto Internal() noexcept
        -> internal::Workflow& = 0;

    Workflow(const Workflow&) = delete;
    Workflow(Workflow&&) = delete;
    auto operator=(const Workflow&) -> Workflow& = delete;
    auto operator=(Workflow&&) -> Workflow& = delete;

    OPENTXS_NO_EXPORT virtual ~Workflow() = default;

protected:
    Workflow() = default;
};
}  // namespace opentxs::api::session
