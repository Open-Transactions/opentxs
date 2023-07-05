// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <Context.pb.h>
#include <cstddef>

#include "internal/otx/consensus/Consensus.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/identity/Types.hpp"
#include "opentxs/otx/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"
#include "otx/consensus/Base.hpp"
#include "otx/consensus/ClientPrivate.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

namespace identifier
{
class Notary;
}  // namespace identifier

namespace otx
{
namespace context
{
class TransactionStatement;
}  // namespace context
}  // namespace otx

}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::otx::context::implementation
{
class ClientContext final : public internal::Client,
                            public Base<ClientContext, ClientPrivate>
{
public:
    auto hasOpenTransactions() const -> bool final;
    using Base<ClientContext, ClientPrivate>::IssuedNumbers;
    auto IssuedNumbers(const UnallocatedSet<TransactionNumber>& exclude) const
        -> std::size_t final;
    auto OpenCronItems() const -> std::size_t final;
    auto Type() const -> otx::ConsensusType final;
    auto Verify(
        const otx::context::TransactionStatement& statement,
        const UnallocatedSet<TransactionNumber>& excluded,
        const UnallocatedSet<TransactionNumber>& included) const -> bool final;
    auto VerifyCronItem(const TransactionNumber number) const -> bool final;
    using Base<ClientContext, ClientPrivate>::VerifyIssuedNumber;
    auto VerifyIssuedNumber(
        const TransactionNumber& number,
        const UnallocatedSet<TransactionNumber>& exclude) const -> bool final;

    auto AcceptIssuedNumbers(UnallocatedSet<TransactionNumber>& newNumbers)
        -> bool final;
    auto CloseCronItem(const TransactionNumber number) -> bool final;
    void FinishAcknowledgements(const UnallocatedSet<RequestNumber>& req) final;
    auto IssueNumber(const TransactionNumber& number) -> bool final;
    auto OpenCronItem(const TransactionNumber number) -> bool final;

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
    ClientContext() = delete;
    ClientContext(const ClientContext&) = delete;
    ClientContext(ClientContext&&) = delete;
    auto operator=(const ClientContext&) -> ClientContext& = delete;
    auto operator=(ClientContext&&) -> ClientContext& = delete;

    ~ClientContext() final = default;

private:
    friend Base<ClientContext, ClientPrivate>;

    static constexpr auto current_version_ = VersionNumber{1};

    GuardedData data_;

    auto client_nym_id() const -> const identifier::Nym& final;
    using Base<ClientContext, ClientPrivate>::serialize;
    auto serialize(const Data& data) const -> proto::Context final;
    auto server_nym_id() const -> const identifier::Nym& final;
    auto type() const -> UnallocatedCString final { return "client"; }
};
}  // namespace opentxs::otx::context::implementation
