// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/rpc/request/MessagePrivate.hpp"  // IWYU pragma: associated
#include "opentxs/rpc/request/SendPayment.hpp"     // IWYU pragma: associated

#include <opentxs/protobuf/RPCCommand.pb.h>
#include <opentxs/protobuf/SendPayment.pb.h>
#include <memory>

#include "internal/core/Factory.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/rpc/CommandType.hpp"
#include "opentxs/rpc/PaymentType.hpp"
#include "opentxs/rpc/Processor.internal.hpp"
#include "opentxs/rpc/Types.internal.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Writer.hpp"

namespace opentxs::rpc::request::implementation
{
struct SendPayment final : public Message::Imp {
    const VersionNumber send_payment_version_;
    const rpc::PaymentType payment_type_;
    const UnallocatedCString contact_;
    const UnallocatedCString source_;
    const UnallocatedCString destination_;
    const UnallocatedCString memo_;
    const opentxs::Amount amount_;

    auto asSendPayment() const noexcept -> const request::SendPayment& final
    {
        return static_cast<const request::SendPayment&>(*parent_);
    }
    auto serialize(protobuf::RPCCommand& dest) const noexcept -> bool final
    {
        if (Imp::serialize(dest)) {
            auto& payment = *dest.mutable_sendpayment();
            payment.set_version(send_payment_version_);
            payment.set_type(translate(payment_type_));
            payment.set_contact(contact_);
            payment.set_sourceaccount(source_);
            payment.set_destinationaccount(destination_);
            payment.set_memo(memo_);
            amount_.Serialize(writer(payment.mutable_amount()));

            return true;
        }

        return false;
    }

    SendPayment(
        const request::SendPayment* parent,
        VersionNumber version,
        SessionIndex session,
        rpc::PaymentType type,
        opentxs::Amount amount,
        const UnallocatedCString& sourceAccount,
        const UnallocatedCString& recipientContact,
        const UnallocatedCString& destinationAccount,
        const UnallocatedCString& memo,
        const Message::AssociateNyms& nyms) noexcept(false)
        : Imp(parent, CommandType::send_payment, version, session, nyms)
        , send_payment_version_(1u)
        , payment_type_(type)
        , contact_(recipientContact)
        , source_(sourceAccount)
        , destination_(destinationAccount)
        , memo_(memo)
        , amount_(amount)
    {
        check_session();
    }
    SendPayment(
        const request::SendPayment* parent,
        const protobuf::RPCCommand& in) noexcept(false)
        : Imp(parent, in)
        , send_payment_version_(in.sendpayment().version())
        , payment_type_(translate(in.sendpayment().type()))
        , contact_(in.sendpayment().contact())
        , source_(in.sendpayment().sourceaccount())
        , destination_(in.sendpayment().destinationaccount())
        , memo_(in.sendpayment().memo())
        , amount_(opentxs::factory::Amount(in.sendpayment().amount()))
    {
        check_session();
    }
    SendPayment() = delete;
    SendPayment(const SendPayment&) = delete;
    SendPayment(SendPayment&&) = delete;
    auto operator=(const SendPayment&) -> SendPayment& = delete;
    auto operator=(SendPayment&&) -> SendPayment& = delete;

    ~SendPayment() final = default;
};
}  // namespace opentxs::rpc::request::implementation

namespace opentxs::rpc::request
{
SendPayment::SendPayment(
    SessionIndex session,
    rpc::PaymentType type,
    const UnallocatedCString& sourceAccount,
    const UnallocatedCString& recipientContact,
    opentxs::Amount amount,
    const UnallocatedCString& memo,
    const AssociateNyms& nyms)
    : Message(std::make_unique<implementation::SendPayment>(
          this,
          DefaultVersion(),
          session,
          type,
          amount,
          sourceAccount,
          recipientContact,
          "",
          memo,
          nyms))
{
}

SendPayment::SendPayment(
    SessionIndex session,
    const UnallocatedCString& sourceAccount,
    const UnallocatedCString& recipientContact,
    const UnallocatedCString& destinationAccount,
    opentxs::Amount amount,
    const UnallocatedCString& memo,
    const AssociateNyms& nyms)
    : Message(std::make_unique<implementation::SendPayment>(
          this,
          DefaultVersion(),
          session,
          PaymentType::transfer,
          amount,
          sourceAccount,
          recipientContact,
          destinationAccount,
          memo,
          nyms))
{
}

SendPayment::SendPayment(
    SessionIndex session,
    const UnallocatedCString& sourceAccount,
    const UnallocatedCString& destinationAddress,
    opentxs::Amount amount,
    const UnallocatedCString& recipientContact,
    const UnallocatedCString& memo,
    const AssociateNyms& nyms)
    : Message(std::make_unique<implementation::SendPayment>(
          this,
          DefaultVersion(),
          session,
          PaymentType::blockchain,
          amount,
          sourceAccount,
          recipientContact,
          destinationAddress,
          memo,
          nyms))
{
}

SendPayment::SendPayment(const protobuf::RPCCommand& in) noexcept(false)
    : Message(std::make_unique<implementation::SendPayment>(this, in))
{
}

SendPayment::SendPayment() noexcept
    : Message(std::make_unique<implementation::SendPayment>(
          this,
          0,
          -1,
          PaymentType::error,
          0,
          "",
          "",
          "",
          "",
          AssociateNyms{}))
{
}

auto SendPayment::Amount() const noexcept -> opentxs::Amount
{
    return static_cast<const implementation::SendPayment&>(*imp_).amount_;
}

auto SendPayment::DefaultVersion() noexcept -> VersionNumber { return 3u; }

auto SendPayment::DestinationAccount() const noexcept
    -> const UnallocatedCString&
{
    return static_cast<const implementation::SendPayment&>(*imp_).destination_;
}

auto SendPayment::Memo() const noexcept -> const UnallocatedCString&
{
    return static_cast<const implementation::SendPayment&>(*imp_).memo_;
}

auto SendPayment::PaymentType() const noexcept -> rpc::PaymentType
{
    return static_cast<const implementation::SendPayment&>(*imp_).payment_type_;
}

auto SendPayment::RecipientContact() const noexcept -> const UnallocatedCString&
{
    return static_cast<const implementation::SendPayment&>(*imp_).contact_;
}

auto SendPayment::SourceAccount() const noexcept -> const UnallocatedCString&
{
    return static_cast<const implementation::SendPayment&>(*imp_).source_;
}

SendPayment::~SendPayment() = default;
}  // namespace opentxs::rpc::request
