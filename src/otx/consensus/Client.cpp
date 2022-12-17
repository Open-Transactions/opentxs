// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "otx/consensus/Client.hpp"  // IWYU pragma: associated

#include <ClientContext.pb.h>
#include <Context.pb.h>
#include <cstdint>
#include <memory>
#include <utility>

#include "internal/otx/consensus/TransactionStatement.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/identity/Nym.hpp"
#include "opentxs/otx/Types.hpp"
#include "opentxs/util/Log.hpp"
#include "otx/consensus/Base.hpp"

namespace opentxs::factory
{
auto ClientContext(
    const api::Session& api,
    const Nym_p& local,
    const Nym_p& remote,
    const identifier::Notary& server) -> otx::context::internal::Client*
{
    using ReturnType = otx::context::implementation::ClientContext;

    return new ReturnType(api, local, remote, server);
}

auto ClientContext(
    const api::Session& api,
    const proto::Context& serialized,
    const Nym_p& local,
    const Nym_p& remote,
    const identifier::Notary& server) -> otx::context::internal::Client*
{
    using ReturnType = otx::context::implementation::ClientContext;

    return new ReturnType(api, serialized, local, remote, server);
}
}  // namespace opentxs::factory

namespace opentxs::otx::context::implementation
{
ClientContext::ClientContext(
    const api::Session& api,
    const Nym_p& local,
    const Nym_p& remote,
    const identifier::Notary& server)
    : Base(api, current_version_, local, remote, server)
    , open_cron_items_()
{
    {
        Lock lock(lock_);
        first_time_init(lock);
    }
}

ClientContext::ClientContext(
    const api::Session& api,
    const proto::Context& serialized,
    const Nym_p& local,
    const Nym_p& remote,
    const identifier::Notary& server)
    : Base(api, current_version_, serialized, local, remote, server)
    , open_cron_items_()
{
    if (serialized.has_clientcontext()) {
        for (const auto& it : serialized.clientcontext().opencronitems()) {
            open_cron_items_.insert(it);
        }
    }

    {
        Lock lock(lock_);
        init_serialized(lock);
    }
}

auto ClientContext::AcceptIssuedNumbers(
    UnallocatedSet<TransactionNumber>& newNumbers) -> bool
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

auto ClientContext::client_nym_id(const Lock& lock) const
    -> const identifier::Nym&
{
    OT_ASSERT(remote_nym_);

    return remote_nym_->ID();
}

auto ClientContext::CloseCronItem(const TransactionNumber number) -> bool
{
    Lock lock(lock_);
    auto output = open_cron_items_.erase(number);
    lock.unlock();

    return (0 < output);
}

void ClientContext::FinishAcknowledgements(
    const UnallocatedSet<RequestNumber>& req)
{
    Lock lock(lock_);

    finish_acknowledgements(lock, req);
}

auto ClientContext::hasOpenTransactions() const -> bool
{
    Lock lock(lock_);

    const auto available = available_transaction_numbers_.size();
    const auto issued = issued_transaction_numbers_.size();

    return available != issued;
}

auto ClientContext::IssuedNumbers(
    const UnallocatedSet<TransactionNumber>& exclude) const -> std::size_t
{
    Lock lock(lock_);

    std::size_t output = 0;

    for (const auto& number : issued_transaction_numbers_) {
        const bool excluded = (1 == exclude.count(number));

        if (!excluded) { output++; }
    }

    return output;
}

auto ClientContext::IssueNumber(const TransactionNumber& number) -> bool
{
    Lock lock(lock_);

    return issue_number(lock, number);
}

auto ClientContext::OpenCronItem(const TransactionNumber number) -> bool
{
    Lock lock(lock_);

    auto output = open_cron_items_.insert(number);

    lock.unlock();

    return output.second;
}

auto ClientContext::OpenCronItems() const -> std::size_t
{
    return open_cron_items_.size();
}

auto ClientContext::serialize(const Lock& lock) const -> proto::Context
{
    OT_ASSERT(verify_write_lock(lock));

    auto output = serialize(lock, Type());
    auto& client = *output.mutable_clientcontext();
    client.set_version(output.version());

    for (const auto& it : open_cron_items_) { client.add_opencronitems(it); }

    return output;
}

auto ClientContext::server_nym_id(const Lock& lock) const
    -> const identifier::Nym&
{
    OT_ASSERT(nym_);

    return nym_->ID();
}

auto ClientContext::Type() const -> otx::ConsensusType
{
    return otx::ConsensusType::Client;
}

auto ClientContext::Verify(
    const TransactionStatement& statement,
    const UnallocatedSet<TransactionNumber>& excluded,
    const UnallocatedSet<TransactionNumber>& included) const -> bool
{
    Lock lock(lock_);

    UnallocatedSet<TransactionNumber> effective = issued_transaction_numbers_;

    for (const auto& number : included) {
        const bool inserted = effective.insert(number).second;

        if (!inserted) {
            LogConsole()(OT_PRETTY_CLASS())("New transaction # ")(
                number)(" already exists in context.")
                .Flush();

            return false;
        }

        LogDetail()(OT_PRETTY_CLASS())("Transaction statement MUST ")(
            "include number ")(number)(" which IS NOT currently in "
                                       "the context. ")
            .Flush();
    }

    for (const auto& number : excluded) {
        const bool removed = (1 == effective.erase(number));

        if (!removed) {
            LogConsole()(OT_PRETTY_CLASS())("Burned transaction # ")(
                number)(" does not exist in context.")
                .Flush();

            return false;
        }

        LogDetail()(OT_PRETTY_CLASS())(
            "Transaction statement MUST NOT include number ")(
            number)(" which IS currently in the context.")
            .Flush();
    }

    for (const auto& number : statement.Issued()) {
        const bool found = (1 == effective.count(number));

        if (!found) {
            LogConsole()(OT_PRETTY_CLASS())("Issued transaction # ")(
                number)(" from statement not found on context.")
                .Flush();

            return false;
        }
    }

    for (const auto& number : effective) {
        const bool found = (1 == statement.Issued().count(number));

        if (!found) {
            LogConsole()(OT_PRETTY_CLASS())("Issued transaction # ")(
                number)(" from context not found on statement.")
                .Flush();

            return false;
        }
    }

    return true;
}

auto ClientContext::VerifyCronItem(const TransactionNumber number) const -> bool
{
    return (0 < open_cron_items_.count(number));
}

auto ClientContext::VerifyIssuedNumber(
    const TransactionNumber& number,
    const UnallocatedSet<TransactionNumber>& exclude) const -> bool
{
    const bool excluded = (1 == exclude.count(number));

    if (excluded) {
        LogVerbose()(OT_PRETTY_CLASS())("Transaction number ")(
            number)(" appears on the list of numbers which are being removed")
            .Flush();

        return false;
    }

    return VerifyIssuedNumber(number);
}
}  // namespace opentxs::otx::context::implementation
