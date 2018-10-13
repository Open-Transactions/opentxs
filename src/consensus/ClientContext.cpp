// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/consensus/ClientContext.hpp"

#include "opentxs/api/Core.hpp"
#include "opentxs/consensus/TransactionStatement.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/Nym.hpp"

#define CURRENT_VERSION 1

#define OT_METHOD "ClientContext::"

namespace opentxs
{
ClientContext::ClientContext(
    const api::Core& api,
    const ConstNym& local,
    const ConstNym& remote,
    const Identifier& server)
    : ot_super(api, CURRENT_VERSION, local, remote, server)
{
}

ClientContext::ClientContext(
    const api::Core& api,
    const proto::Context& serialized,
    const ConstNym& local,
    const ConstNym& remote,
    const Identifier& server)
    : ot_super(api, CURRENT_VERSION, serialized, local, remote, server)
{
    if (serialized.has_clientcontext()) {
        for (const auto& it : serialized.clientcontext().opencronitems()) {
            open_cron_items_.insert(it);
        }
    }
}

bool ClientContext::AcceptIssuedNumbers(std::set<TransactionNumber>& newNumbers)
{
    Lock lock(lock_);

    std::size_t added = 0;
    const auto offered = newNumbers.size();

    if (0 == offered) { return false; }

    for (const auto& number : newNumbers) {
        // If number wasn't already on issued list, then add to BOTH lists.
        // Otherwise do nothing (it's already on the issued list, and no longer
        // valid on the available list--thus shouldn't be re-added there
        // anyway.)
        const bool exists = 1 == issued_transaction_numbers_.count(number);

        if (!exists) {
            if (issue_number(lock, number)) { added++; }
        }
    }

    return (added == offered);
}

const Identifier& ClientContext::client_nym_id(const Lock& lock) const
{
    OT_ASSERT(remote_nym_);

    return remote_nym_->ID();
}

bool ClientContext::CloseCronItem(const TransactionNumber number)
{
    Lock lock(lock_);

    auto output = open_cron_items_.erase(number);

    lock.unlock();

    return (0 < output);
}

void ClientContext::FinishAcknowledgements(const std::set<RequestNumber>& req)
{
    Lock lock(lock_);

    finish_acknowledgements(lock, req);
}

bool ClientContext::hasOpenTransactions() const
{
    Lock lock(lock_);

    const auto available = available_transaction_numbers_.size();
    const auto issued = issued_transaction_numbers_.size();

    return available != issued;
}

std::size_t ClientContext::IssuedNumbers(
    const std::set<TransactionNumber>& exclude) const
{
    Lock lock(lock_);

    std::size_t output = 0;

    for (const auto& number : issued_transaction_numbers_) {
        const bool excluded = (1 == exclude.count(number));

        if (!excluded) { output++; }
    }

    return output;
}

bool ClientContext::IssueNumber(const TransactionNumber& number)
{
    Lock lock(lock_);

    return issue_number(lock, number);
}

bool ClientContext::OpenCronItem(const TransactionNumber number)
{
    Lock lock(lock_);

    auto output = open_cron_items_.insert(number);

    lock.unlock();

    return output.second;
}

std::size_t ClientContext::OpenCronItems() const
{
    return open_cron_items_.size();
}

proto::Context ClientContext::serialize(const Lock& lock) const
{
    OT_ASSERT(verify_write_lock(lock));

    auto output = serialize(lock, Type());
    auto& client = *output.mutable_clientcontext();
    client.set_version(output.version());

    for (const auto& it : open_cron_items_) { client.add_opencronitems(it); }

    return output;
}

const Identifier& ClientContext::server_nym_id(const Lock& lock) const
{
    OT_ASSERT(nym_);

    return nym_->ID();
}

proto::ConsensusType ClientContext::Type() const
{
    return proto::CONSENSUSTYPE_CLIENT;
}

bool ClientContext::Verify(
    const TransactionStatement& statement,
    const std::set<TransactionNumber>& excluded,
    const std::set<TransactionNumber>& included) const
{
    Lock lock(lock_);

    std::set<TransactionNumber> effective = issued_transaction_numbers_;

    for (const auto& number : included) {
        const bool inserted = effective.insert(number).second;

        if (!inserted) {
            otOut << OT_METHOD << __FUNCTION__ << ": New transaction # "
                  << number << " already exists in context." << std::endl;

            return false;
        }

        LogDetail(OT_METHOD)(__FUNCTION__)(": Transaction statement MUST ")(
            "include number ")(number)(" which IS NOT currently in ")(
            "the context. ")
            .Flush();
    }

    for (const auto& number : excluded) {
        const bool removed = (1 == effective.erase(number));

        if (!removed) {
            otOut << OT_METHOD << __FUNCTION__ << ": Burned transaction # "
                  << number << " does not exist in context." << std::endl;

            return false;
        }

        LogDetail(OT_METHOD)(__FUNCTION__)(": Transaction statement MUST ")(
            "NOT include number ")(number)(" which IS currently in ")(
            "the context. ")
            .Flush();
    }

    for (const auto& number : statement.Issued()) {
        const bool found = (1 == effective.count(number));

        if (!found) {
            otOut << OT_METHOD << __FUNCTION__ << ": Issued transaction # "
                  << number << " from statement not found on context."
                  << std::endl;

            return false;
        }
    }

    for (const auto& number : effective) {
        const bool found = (1 == statement.Issued().count(number));

        if (!found) {
            otOut << OT_METHOD << __FUNCTION__ << ": Issued transaction # "
                  << number << " from context not found on statement."
                  << std::endl;

            return false;
        }
    }

    return true;
}

bool ClientContext::VerifyCronItem(const TransactionNumber number) const
{
    return (0 < open_cron_items_.count(number));
}

bool ClientContext::VerifyIssuedNumber(
    const TransactionNumber& number,
    const std::set<TransactionNumber>& exclude) const
{
    const bool excluded = (1 == exclude.count(number));

    if (excluded) { return false; }

    return VerifyIssuedNumber(number);
}
}  // namespace opentxs
