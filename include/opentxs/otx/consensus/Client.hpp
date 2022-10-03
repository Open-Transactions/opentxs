// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Export.hpp"
#include "opentxs/otx/consensus/Base.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace otx
{
namespace context
{
namespace internal
{
class Client;
}  // namespace internal

class TransactionStatement;
}  // namespace context
}  // namespace otx
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::otx::context
{
class OPENTXS_EXPORT Client : virtual public Base
{
public:
    virtual auto hasOpenTransactions() const -> bool = 0;
    OPENTXS_NO_EXPORT virtual auto InternalClient() const noexcept
        -> const internal::Client& = 0;
    using Base::IssuedNumbers;
    virtual auto IssuedNumbers(const TransactionNumbers& exclude) const
        -> std::size_t = 0;
    virtual auto OpenCronItems() const -> std::size_t = 0;
    virtual auto Verify(
        const TransactionStatement& statement,
        const TransactionNumbers& excluded,
        const TransactionNumbers& included) const -> bool = 0;
    virtual auto VerifyCronItem(const TransactionNumber number) const
        -> bool = 0;
    using Base::VerifyIssuedNumber;
    virtual auto VerifyIssuedNumber(
        const TransactionNumber& number,
        const TransactionNumbers& exclude) const -> bool = 0;

    virtual auto AcceptIssuedNumbers(TransactionNumbers& newNumbers)
        -> bool = 0;
    virtual void FinishAcknowledgements(const RequestNumbers& req) = 0;
    OPENTXS_NO_EXPORT virtual auto InternalClient() noexcept
        -> internal::Client& = 0;
    virtual auto IssueNumber(const TransactionNumber& number) -> bool = 0;

    Client(const Client&) = delete;
    Client(Client&&) = delete;
    auto operator=(const Client&) -> Client& = delete;
    auto operator=(Client&&) -> Client& = delete;

    ~Client() override = default;

protected:
    Client() = default;
};
}  // namespace opentxs::otx::context
