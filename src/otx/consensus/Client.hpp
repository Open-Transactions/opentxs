// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <iosfwd>
#include <mutex>

#include "Proto.hpp"
#include "internal/otx/consensus/Consensus.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/otx/ConsensusType.hpp"
#include "opentxs/otx/Types.hpp"
#include "opentxs/otx/consensus/Client.hpp"
#include "opentxs/otx/consensus/TransactionStatement.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"
#include "otx/consensus/Base.hpp"
#include "serialization/protobuf/ConsensusEnums.pb.h"
#include "serialization/protobuf/Context.pb.h"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace api
{
class Session;
}  // namespace api

namespace identifier
{
class Notary;
class Nym;
}  // namespace identifier

class PasswordPrompt;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::otx::context::implementation
{
class ClientContext final : virtual public internal::Client, public Base
{
public:
    auto GetContract(const Lock& lock) const -> proto::Context final
    {
        return contract(lock);
    }
    auto hasOpenTransactions() const -> bool final;
    using Base::IssuedNumbers;
    auto IssuedNumbers(const UnallocatedSet<TransactionNumber>& exclude) const
        -> std::size_t final;
    auto OpenCronItems() const -> std::size_t final;
    auto Type() const -> otx::ConsensusType final;
    auto ValidateContext(const Lock& lock) const -> bool final
    {
        return validate(lock);
    }
    auto Verify(
        const otx::context::TransactionStatement& statement,
        const UnallocatedSet<TransactionNumber>& excluded,
        const UnallocatedSet<TransactionNumber>& included) const -> bool final;
    auto VerifyCronItem(const TransactionNumber number) const -> bool final;
    using Base::VerifyIssuedNumber;
    auto VerifyIssuedNumber(
        const TransactionNumber& number,
        const UnallocatedSet<TransactionNumber>& exclude) const -> bool final;

    auto AcceptIssuedNumbers(UnallocatedSet<TransactionNumber>& newNumbers)
        -> bool final;
    auto CloseCronItem(const TransactionNumber number) -> bool final;
    void FinishAcknowledgements(const UnallocatedSet<RequestNumber>& req) final;
    auto GetLock() -> std::mutex& final { return lock_; }
    auto IssueNumber(const TransactionNumber& number) -> bool final;
    auto OpenCronItem(const TransactionNumber number) -> bool final;
    auto UpdateSignature(const Lock& lock, const PasswordPrompt& reason)
        -> bool final
    {
        return update_signature(lock, reason);
    }

    ClientContext(
        const api::Session& api,
        const Nym_p& local,
        const Nym_p& remote,
        const identifier::Notary& server);
    ClientContext(
        const api::Session& api,
        const proto::Context& serialized,
        const Nym_p& local,
        const Nym_p& remote,
        const identifier::Notary& server);
    ~ClientContext() final = default;

private:
    static constexpr auto current_version_ = VersionNumber{1};

    UnallocatedSet<TransactionNumber> open_cron_items_;

    auto client_nym_id(const Lock& lock) const -> const identifier::Nym& final;
    using Base::serialize;
    auto serialize(const Lock& lock) const -> proto::Context final;
    auto server_nym_id(const Lock& lock) const -> const identifier::Nym& final;
    auto type() const -> UnallocatedCString final { return "client"; }

    ClientContext() = delete;
    ClientContext(const otx::context::Client&) = delete;
    ClientContext(otx::context::Client&&) = delete;
    auto operator=(const otx::context::Client&)
        -> otx::context::Client& = delete;
    auto operator=(ClientContext&&) -> otx::context::Client& = delete;
};
}  // namespace opentxs::otx::context::implementation
