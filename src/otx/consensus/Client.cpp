// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "otx/consensus/Client.hpp"  // IWYU pragma: associated

#include <ClientContext.pb.h>
#include <Context.pb.h>
#include <memory>
#include <shared_mutex>
#include <utility>

#include "internal/otx/consensus/TransactionStatement.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/identity/Nym.hpp"
#include "opentxs/otx/ConsensusType.hpp"  // IWYU pragma: keep
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
    : Base<ClientContext, ClientPrivate>(
          api,
          current_version_,
          local,
          remote,
          server)
    , data_(current_version_)
{
}

ClientContext::ClientContext(
    const api::Session& api,
    const proto::Context& serialized,
    const Nym_p& local,
    const Nym_p& remote,
    const identifier::Notary& server)
    : Base<ClientContext, ClientPrivate>(
          api,
          current_version_,
          serialized,
          local,
          remote,
          server)
    , data_(api, current_version_, serialized)
{
    init_serialized();
}

auto ClientContext::AcceptIssuedNumbers(
    UnallocatedSet<TransactionNumber>& newNumbers) -> bool
{
    auto handle = get_data();
    auto& data = *handle;
    auto added = 0_uz;
    const auto offered = newNumbers.size();

    if (0 == offered) { return false; }

    for (const auto& number : newNumbers) {
        // If number wasn't already on issued list, then add to BOTH lists.
        // Otherwise do nothing (it's already on the issued list, and no longer
        // valid on the available list--thus shouldn't be re-added there
        // anyway.)
        if (false == verify_issued_number(data, number)) {
            if (issue_number(data, number)) { added++; }
        }
    }

    return (added == offered);
}

auto ClientContext::client_nym_id() const -> const identifier::Nym&
{
    assert_false(nullptr == remote_nym_);

    return remote_nym_->ID();
}

auto ClientContext::CloseCronItem(const TransactionNumber number) -> bool
{
    auto handle = get_data();
    auto& data = *handle;
    auto output = data.open_cron_items_.erase(number);

    return (0 < output);
}

auto ClientContext::FinishAcknowledgements(
    const UnallocatedSet<RequestNumber>& req) -> void
{
    finish_acknowledgements(*get_data(), req);
}

auto ClientContext::hasOpenTransactions() const -> bool
{
    auto handle = get_data();
    const auto& data = *handle;
    const auto available = data.available_transaction_numbers_.size();
    const auto issued = data.issued_transaction_numbers_.size();

    return available != issued;
}

auto ClientContext::IssuedNumbers(
    const UnallocatedSet<TransactionNumber>& exclude) const -> std::size_t
{
    auto output = 0_uz;
    auto handle = get_data();
    const auto& data = *handle;

    for (const auto& number : data.issued_transaction_numbers_) {
        if (false == exclude.contains(number)) { output++; }
    }

    return output;
}

auto ClientContext::IssueNumber(const TransactionNumber& number) -> bool
{
    return issue_number(*get_data(), number);
}

auto ClientContext::OpenCronItem(const TransactionNumber number) -> bool
{
    auto handle = get_data();
    auto& data = *handle;
    auto output = data.open_cron_items_.insert(number);

    return output.second;
}

auto ClientContext::OpenCronItems() const -> std::size_t
{
    auto handle = get_data();
    const auto& data = *handle;

    return data.open_cron_items_.size();
}

auto ClientContext::serialize(const Data& data) const -> proto::Context
{
    auto output = serialize(data, Type());
    auto& client = *output.mutable_clientcontext();
    client.set_version(output.version());

    for (const auto& it : data.open_cron_items_) {
        client.add_opencronitems(it);
    }

    return output;
}

auto ClientContext::server_nym_id() const -> const identifier::Nym&
{
    assert_false(nullptr == Signer());

    return Signer()->ID();
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
    auto effective = get_data()->issued_transaction_numbers_;

    for (const auto& number : included) {
        const bool inserted = effective.insert(number).second;

        if (!inserted) {
            LogConsole()()("New transaction # ")(
                number)(" already exists in context.")
                .Flush();

            return false;
        }

        LogDetail()()("Transaction statement MUST ")("include number ")(
            number)(" which IS NOT currently in "
                    "the context. ")
            .Flush();
    }

    for (const auto& number : excluded) {
        const bool removed = (1 == effective.erase(number));

        if (!removed) {
            LogConsole()()("Burned transaction # ")(
                number)(" does not exist in context.")
                .Flush();

            return false;
        }

        LogDetail()()("Transaction statement MUST NOT include number ")(
            number)(" which IS currently in the context.")
            .Flush();
    }

    for (const auto& number : statement.Issued()) {
        const bool found = effective.contains(number);

        if (!found) {
            LogConsole()()("Issued transaction # ")(
                number)(" from statement not found on context.")
                .Flush();

            return false;
        }
    }

    for (const auto& number : effective) {
        const bool found = statement.Issued().contains(number);

        if (!found) {
            LogConsole()()("Issued transaction # ")(
                number)(" from context not found on statement.")
                .Flush();

            return false;
        }
    }

    return true;
}

auto ClientContext::VerifyCronItem(const TransactionNumber number) const -> bool
{
    auto handle = get_data();
    const auto& data = *handle;

    return (data.open_cron_items_.contains(number));
}

auto ClientContext::VerifyIssuedNumber(
    const TransactionNumber& number,
    const UnallocatedSet<TransactionNumber>& exclude) const -> bool
{
    const bool excluded = (1 == exclude.count(number));

    if (excluded) {
        LogVerbose()()("Transaction number ")(
            number)(" appears on the list of numbers which are being removed")
            .Flush();

        return false;
    }

    return VerifyIssuedNumber(number);
}
}  // namespace opentxs::otx::context::implementation
