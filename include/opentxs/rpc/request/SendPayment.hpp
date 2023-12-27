// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Export.hpp"
#include "opentxs/rpc/request/Message.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace proto
{
class RPCCommand;
}  // namespace proto

class Amount;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::rpc::request
{
class OPENTXS_EXPORT SendPayment final : public Message
{
public:
    static auto DefaultVersion() noexcept -> VersionNumber;

    auto Amount() const noexcept -> opentxs::Amount;
    auto DestinationAccount() const noexcept -> const UnallocatedCString&;
    auto Memo() const noexcept -> const UnallocatedCString&;
    auto PaymentType() const noexcept -> rpc::PaymentType;
    auto RecipientContact() const noexcept -> const UnallocatedCString&;
    auto SourceAccount() const noexcept -> const UnallocatedCString&;

    /// throws std::runtime_error for invalid constructor arguments
    /// cheque, voucher, invoice, blinded
    SendPayment(
        SessionIndex session,
        rpc::PaymentType type,
        const UnallocatedCString& sourceAccount,
        const UnallocatedCString& recipientContact,
        opentxs::Amount amount,
        const UnallocatedCString& memo = {},
        const AssociateNyms& nyms = {}) noexcept(false);
    /// throws std::runtime_error for invalid constructor arguments
    /// transfer
    SendPayment(
        SessionIndex session,
        const UnallocatedCString& sourceAccount,
        const UnallocatedCString& recipientContact,
        const UnallocatedCString& destinationAccount,
        opentxs::Amount amount,
        const UnallocatedCString& memo = {},
        const AssociateNyms& nyms = {}) noexcept(false);
    /// throws std::runtime_error for invalid constructor arguments
    /// blockchain
    SendPayment(
        SessionIndex session,
        const UnallocatedCString& sourceAccount,
        const UnallocatedCString& destinationAddress,
        opentxs::Amount amount,
        const UnallocatedCString& recipientContact = {},
        const UnallocatedCString& memo = {},
        const AssociateNyms& nyms = {}) noexcept(false);
    OPENTXS_NO_EXPORT SendPayment(const proto::RPCCommand& serialized) noexcept(
        false);
    SendPayment() noexcept;
    SendPayment(const SendPayment&) = delete;
    SendPayment(SendPayment&&) = delete;
    auto operator=(const SendPayment&) -> SendPayment& = delete;
    auto operator=(SendPayment&&) -> SendPayment& = delete;

    ~SendPayment() final;
};
}  // namespace opentxs::rpc::request
