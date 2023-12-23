// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/util/Writer.hpp"

#pragma once

#include <functional>
#include <future>
#include <memory>
#include <tuple>
#include <utility>

#include "internal/otx/consensus/Server.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/session/OTX.hpp"
#include "opentxs/contract/ContractType.hpp"  // IWYU pragma: keep
#include "opentxs/contract/Types.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/contract/peer/Reply.hpp"
#include "opentxs/core/contract/peer/Request.hpp"
#include "opentxs/identifier/Account.hpp"
#include "opentxs/identifier/Generic.hpp"
#include "opentxs/identifier/Notary.hpp"
#include "opentxs/identifier/Nym.hpp"
#include "opentxs/identifier/UnitDefinition.hpp"
#include "opentxs/identity/wot/claim/Types.hpp"
#include "opentxs/otx/Types.hpp"
#include "opentxs/otx/client/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Time.hpp"
#include "util/Blank.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

namespace otx
{
namespace blind
{
class Purse;
}  // namespace blind
}  // namespace otx

namespace proto
{
class UnitDefinition;
}  // namespace proto

class Cheque;
class OTPayment;
class String;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs
{
struct OT_DownloadNymboxType {
};
struct OT_GetTransactionNumbersType {
};

auto operator<(
    const OT_DownloadNymboxType&,
    const OT_DownloadNymboxType&) noexcept -> bool;

auto operator<(
    const OT_GetTransactionNumbersType&,
    const OT_GetTransactionNumbersType&) noexcept -> bool;
}  // namespace opentxs

namespace opentxs::otx::client
{
using CheckNymTask = identifier::Nym;
/** DepositPaymentTask: unit id, accountID, payment */
using DepositPaymentTask = std::tuple<
    identifier::UnitDefinition,
    identifier::Account,
    std::shared_ptr<const OTPayment>>;
using DownloadContractTask = identifier::Notary;
using DownloadMintTask = std::pair<identifier::UnitDefinition, int>;
using DownloadNymboxTask = OT_DownloadNymboxType;
using DownloadUnitDefinitionTask = identifier::UnitDefinition;
using GetTransactionNumbersTask = OT_GetTransactionNumbersType;
/** IssueUnitDefinitionTask: unit definition id, account label, claim */
using IssueUnitDefinitionTask =
    std::tuple<identifier::UnitDefinition, UnallocatedCString, UnitType>;
/** MessageTask: recipientID, message */
using MessageTask =
    std::tuple<identifier::Nym, UnallocatedCString, std::shared_ptr<SetID>>;
/** PayCashTask: recipientID, workflow ID */
using PayCashTask = std::pair<identifier::Nym, identifier::Generic>;
/** PaymentTask: recipientID, payment */
using PaymentTask =
    std::pair<identifier::Nym, std::shared_ptr<const OTPayment>>;
/** PeerReplyTask: targetNymID, peer reply, peer request */
using PeerReplyTask =
    std::tuple<identifier::Nym, contract::peer::Reply, contract::peer::Request>;
/** PeerRequestTask: targetNymID, peer request */
using PeerRequestTask = std::pair<identifier::Nym, contract::peer::Request>;
using ProcessInboxTask = identifier::Account;
using PublishServerContractTask = std::pair<identifier::Notary, bool>;
/** RegisterAccountTask: account label, unit definition id */
using RegisterAccountTask =
    std::pair<UnallocatedCString, identifier::UnitDefinition>;
using RegisterNymTask = bool;
/** SendChequeTask: sourceAccountID, targetNymID, value, memo, validFrom,
 * validTo
 */
using SendChequeTask = std::tuple<
    identifier::Account,
    identifier::Nym,
    Amount,
    UnallocatedCString,
    Time,
    Time>;
/** SendTransferTask: source account, destination account, amount, memo
 */
using SendTransferTask = std::
    tuple<identifier::Account, identifier::Account, Amount, UnallocatedCString>;
/** WithdrawCashTask: Account ID, amount*/
using WithdrawCashTask = std::pair<identifier::Account, Amount>;
}  // namespace opentxs::otx::client

namespace std
{
// NOLINTBEGIN(cert-dcl58-cpp)
template <>
struct less<opentxs::otx::client::MessageTask> {
    auto operator()(
        const opentxs::otx::client::MessageTask& lhs,
        const opentxs::otx::client::MessageTask& rhs) const noexcept -> bool;
};
template <>
struct less<opentxs::otx::client::PaymentTask> {
    auto operator()(
        const opentxs::otx::client::PaymentTask& lhs,
        const opentxs::otx::client::PaymentTask& rhs) const noexcept -> bool;
};
template <>
struct less<opentxs::otx::client::PeerReplyTask> {
    auto operator()(
        const opentxs::otx::client::PeerReplyTask& lhs,
        const opentxs::otx::client::PeerReplyTask& rhs) const noexcept -> bool;
};
template <>
struct less<opentxs::otx::client::PeerRequestTask> {
    auto operator()(
        const opentxs::otx::client::PeerRequestTask& lhs,
        const opentxs::otx::client::PeerRequestTask& rhs) const noexcept
        -> bool;
};
template <>
struct less<opentxs::OT_DownloadNymboxType> {
    auto operator()(
        const opentxs::OT_DownloadNymboxType& lhs,
        const opentxs::OT_DownloadNymboxType& rhs) const noexcept -> bool;
};
template <>
struct less<opentxs::OT_GetTransactionNumbersType> {
    auto operator()(
        const opentxs::OT_GetTransactionNumbersType& lhs,
        const opentxs::OT_GetTransactionNumbersType& rhs) const noexcept
        -> bool;
};
// NOLINTEND(cert-dcl58-cpp)
}  // namespace std

namespace opentxs
{
template <>
struct make_blank<otx::client::DepositPaymentTask> {
    static auto value(const api::Session& api)
        -> otx::client::DepositPaymentTask
    {
        return {
            make_blank<identifier::UnitDefinition>::value(api),
            make_blank<identifier::Account>::value(api),
            nullptr};
    }
};
template <>
struct make_blank<otx::client::DownloadMintTask> {
    static auto value(const api::Session& api) -> otx::client::DownloadMintTask
    {
        return {make_blank<identifier::UnitDefinition>::value(api), 0};
    }
};
template <>
struct make_blank<otx::client::IssueUnitDefinitionTask> {
    static auto value(const api::Session& api)
        -> otx::client::IssueUnitDefinitionTask
    {
        return {
            make_blank<identifier::UnitDefinition>::value(api),
            "",
            UnitType::Error};
    }
};
template <>
struct make_blank<otx::client::MessageTask> {
    static auto value(const api::Session& api) -> otx::client::MessageTask
    {
        return {make_blank<identifier::Nym>::value(api), "", nullptr};
    }
};
template <>
struct make_blank<otx::client::PayCashTask> {
    static auto value(const api::Session& api) -> otx::client::PayCashTask
    {
        return {
            make_blank<identifier::Nym>::value(api),
            make_blank<identifier::Generic>::value(api)};
    }
};
template <>
struct make_blank<otx::client::PaymentTask> {
    static auto value(const api::Session& api) -> otx::client::PaymentTask
    {
        return {make_blank<identifier::Nym>::value(api), nullptr};
    }
};
template <>
struct make_blank<otx::client::PeerReplyTask> {
    static auto value(const api::Session& api) -> otx::client::PeerReplyTask
    {
        return {
            make_blank<identifier::Nym>::value(api),
            make_blank<contract::peer::Reply>::value(api),
            make_blank<contract::peer::Request>::value(api)};
    }
};
template <>
struct make_blank<otx::client::PeerRequestTask> {
    static auto value(const api::Session& api) -> otx::client::PeerRequestTask
    {
        return {
            make_blank<identifier::Nym>::value(api),
            make_blank<contract::peer::Request>::value(api)};
    }
};
template <>
struct make_blank<otx::client::PublishServerContractTask> {
    static auto value(const api::Session& api)
        -> otx::client::PublishServerContractTask
    {
        return {make_blank<identifier::Notary>::value(api), false};
    }
};
template <>
struct make_blank<otx::client::RegisterAccountTask> {
    static auto value(const api::Session& api)
        -> otx::client::RegisterAccountTask
    {
        return {"", make_blank<identifier::UnitDefinition>::value(api)};
    }
};
template <>
struct make_blank<otx::client::SendChequeTask> {
    static auto value(const api::Session& api) -> otx::client::SendChequeTask
    {
        return {
            make_blank<identifier::Account>::value(api),
            make_blank<identifier::Nym>::value(api),
            0,
            "",
            Clock::now(),
            Clock::now()};
    }
};
template <>
struct make_blank<otx::client::SendTransferTask> {
    static auto value(const api::Session& api) -> otx::client::SendTransferTask
    {
        return {
            make_blank<identifier::Account>::value(api),
            make_blank<identifier::Account>::value(api),
            0,
            ""};
    }
};
template <>
struct make_blank<otx::client::WithdrawCashTask> {
    static auto value(const api::Session& api) -> otx::client::WithdrawCashTask
    {
        return {make_blank<identifier::Account>::value(api), 0};
    }
};
}  // namespace opentxs

namespace opentxs::otx::client::internal
{
struct Operation {
    using Result = otx::context::Server::DeliveryResult;
    using Future = std::future<Result>;

    virtual auto NymID() const -> const identifier::Nym& = 0;
    virtual auto ServerID() const -> const identifier::Notary& = 0;

    virtual auto AddClaim(
        const identity::wot::claim::SectionType section,
        const identity::wot::claim::ClaimType type,
        const String& value,
        const bool primary) -> bool = 0;
    virtual auto ConveyPayment(
        const identifier::Nym& recipient,
        const std::shared_ptr<const OTPayment> payment) -> bool = 0;
    virtual auto DepositCash(
        const identifier::Account& depositAccountID,
        blind::Purse&& purse) -> bool = 0;
    virtual auto DepositCheque(
        const identifier::Account& depositAccountID,
        const std::shared_ptr<Cheque> cheque) -> bool = 0;
    virtual auto DownloadContract(
        const identifier::Generic& ID,
        const contract::Type type = contract::Type::invalid) -> bool = 0;
    virtual auto GetFuture() -> Future = 0;
    virtual auto IssueUnitDefinition(
        const std::shared_ptr<const proto::UnitDefinition> unitDefinition,
        const otx::context::Server::ExtraArgs& args = {}) -> bool = 0;
    virtual auto IssueUnitDefinition(
        const ReadView& unitDefinition,
        const otx::context::Server::ExtraArgs& args = {}) -> bool = 0;
    virtual void join() = 0;
    virtual auto PublishContract(const identifier::Nym& id) -> bool = 0;
    virtual auto PublishContract(const identifier::Notary& id) -> bool = 0;
    virtual auto PublishContract(const identifier::UnitDefinition& id)
        -> bool = 0;
    virtual auto RequestAdmin(const String& password) -> bool = 0;
    virtual auto SendCash(
        const identifier::Nym& recipient,
        const identifier::Generic& workflowID) -> bool = 0;
    virtual auto SendMessage(
        const identifier::Nym& recipient,
        const String& message,
        const SetID setID = {}) -> bool = 0;
    virtual auto SendPeerReply(
        const identifier::Nym& targetNymID,
        const contract::peer::Reply& peerreply,
        const contract::peer::Request& peerrequest) -> bool = 0;
    virtual auto SendPeerRequest(
        const identifier::Nym& targetNymID,
        const contract::peer::Request& peerrequest) -> bool = 0;
    virtual auto SendTransfer(
        const identifier::Account& sourceAccountID,
        const identifier::Account& destinationAccountID,
        const Amount& amount,
        const String& memo) -> bool = 0;
    virtual void SetPush(const bool enabled) = 0;
    virtual void Shutdown() = 0;
    virtual auto Start(
        const otx::OperationType type,
        const otx::context::Server::ExtraArgs& args = {}) -> bool = 0;
    virtual auto Start(
        const otx::OperationType type,
        const identifier::UnitDefinition& targetUnitID,
        const otx::context::Server::ExtraArgs& args = {}) -> bool = 0;
    virtual auto Start(
        const otx::OperationType type,
        const identifier::Nym& targetNymID,
        const otx::context::Server::ExtraArgs& args = {}) -> bool = 0;
    virtual auto UpdateAccount(const identifier::Account& accountID)
        -> bool = 0;
    virtual auto WithdrawCash(
        const identifier::Account& accountID,
        const Amount& amount) -> bool = 0;

    virtual ~Operation() = default;
};

struct StateMachine {
    using BackgroundTask = api::session::OTX::BackgroundTask;
    using Result = api::session::OTX::Result;
    using TaskID = api::session::OTX::TaskID;

    virtual auto api() const -> const api::Session& = 0;
    virtual auto DepositPayment(const otx::client::DepositPaymentTask& params)
        const -> BackgroundTask = 0;
    virtual auto DownloadUnitDefinition(
        const otx::client::DownloadUnitDefinitionTask& params) const
        -> BackgroundTask = 0;
    virtual auto error_result() const -> Result = 0;
    virtual auto finish_task(
        const TaskID taskID,
        const bool success,
        Result&& result) const -> bool = 0;
    virtual auto next_task_id() const -> TaskID = 0;
    virtual auto RegisterAccount(const otx::client::RegisterAccountTask& params)
        const -> BackgroundTask = 0;
    virtual auto start_task(const TaskID taskID, bool success) const
        -> BackgroundTask = 0;

    virtual ~StateMachine() = default;
};
}  // namespace opentxs::otx::client::internal
