// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Forward.hpp"

namespace opentxs::implementation
{
class ClientContext final : virtual public internal::ClientContext,
                            public Context
{
public:
    proto::Context GetContract(const Lock& lock) const override
    {
        return contract(lock);
    }
    bool hasOpenTransactions() const override;
    using implementation::Context::IssuedNumbers;
    std::size_t IssuedNumbers(
        const std::set<TransactionNumber>& exclude) const override;
    std::size_t OpenCronItems() const override;
    proto::ConsensusType Type() const override;
    bool ValidateContext(const Lock& lock) const override
    {
        return validate(lock);
    }
    bool Verify(
        const TransactionStatement& statement,
        const std::set<TransactionNumber>& excluded,
        const std::set<TransactionNumber>& included) const override;
    bool VerifyCronItem(const TransactionNumber number) const override;
    using implementation::Context::VerifyIssuedNumber;
    bool VerifyIssuedNumber(
        const TransactionNumber& number,
        const std::set<TransactionNumber>& exclude) const override;

    bool AcceptIssuedNumbers(std::set<TransactionNumber>& newNumbers) override;
    bool CloseCronItem(const TransactionNumber number) override;
    void FinishAcknowledgements(const std::set<RequestNumber>& req) override;
    std::mutex& GetLock() override { return lock_; }
    bool IssueNumber(const TransactionNumber& number) override;
    bool OpenCronItem(const TransactionNumber number) override;
    bool UpdateSignature(const Lock& lock) override
    {
        return update_signature(lock);
    }

    ~ClientContext() = default;

private:
    friend opentxs::Factory;

    std::set<TransactionNumber> open_cron_items_{};

    const identifier::Nym& client_nym_id(const Lock& lock) const override;
    using implementation::Context::serialize;
    proto::Context serialize(const Lock& lock) const override;
    const identifier::Nym& server_nym_id(const Lock& lock) const override;
    std::string type() const override { return "client"; }

    ClientContext(
        const api::Core& api,
        const ConstNym& local,
        const ConstNym& remote,
        const Identifier& server);
    ClientContext(
        const api::Core& api,
        const proto::Context& serialized,
        const ConstNym& local,
        const ConstNym& remote,
        const Identifier& server);
    ClientContext() = delete;
    ClientContext(const ClientContext&) = delete;
    ClientContext(ClientContext&&) = delete;
    ClientContext& operator=(const ClientContext&) = delete;
    ClientContext& operator=(ClientContext&&) = delete;
};
}  // namespace opentxs::implementation
