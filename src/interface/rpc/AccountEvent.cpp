// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::proto::PaymentWorkflowState

#include "opentxs/interface/rpc/AccountEvent.hpp"  // IWYU pragma: associated

#include <AccountEvent.pb.h>
#include <PaymentWorkflowEnums.pb.h>
#include <algorithm>
#include <cassert>
#include <memory>
#include <utility>

#include "internal/core/Factory.hpp"
#include "internal/interface/rpc/RPC.hpp"
#include "internal/util/Time.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/interface/rpc/AccountEventType.hpp"  // IWYU pragma: keep
#include "opentxs/interface/rpc/Types.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Numbers.hpp"
#include "opentxs/util/Writer.hpp"

namespace opentxs::rpc
{
struct AccountEvent::Imp {
    static constexpr auto default_version_ = VersionNumber{2};

    const VersionNumber version_;
    const UnallocatedCString account_;
    const AccountEventType type_;
    const UnallocatedCString contact_;
    const UnallocatedCString workflow_;
    const UnallocatedCString amount_formatted_;
    const UnallocatedCString pending_formatted_;
    const Amount amount_;
    const Amount pending_;
    const opentxs::Time time_;
    const UnallocatedCString memo_;
    const UnallocatedCString uuid_;
    const int state_;

    Imp(const VersionNumber version,
        const UnallocatedCString& account,
        AccountEventType type,
        const UnallocatedCString& contact,
        const UnallocatedCString& workflow,
        const UnallocatedCString& amountF,
        const UnallocatedCString& pendingF,
        Amount amount,
        Amount pending,
        opentxs::Time time,
        const UnallocatedCString& memo,
        const UnallocatedCString& uuid,
        int state) noexcept(false)
        : version_(version)
        , account_(account)
        , type_(type)
        , contact_(contact)
        , workflow_(workflow)
        , amount_formatted_(amountF)
        , pending_formatted_(pendingF)
        , amount_(amount)
        , pending_(pending)
        , time_(time)
        , memo_(memo)
        , uuid_(uuid)
        , state_(state)
    {
        // FIXME validate member variables
    }
    Imp() noexcept
        : Imp(0,
              "",
              AccountEventType::error,
              "",
              "",
              "",
              "",
              0,
              0,
              {},
              "",
              "",
              0)
    {
    }
    Imp(const Imp& rhs) noexcept
        : Imp(rhs.version_,
              rhs.account_,
              rhs.type_,
              rhs.contact_,
              rhs.workflow_,
              rhs.amount_formatted_,
              rhs.pending_formatted_,
              rhs.amount_,
              rhs.pending_,
              rhs.time_,
              rhs.memo_,
              rhs.uuid_,
              rhs.state_)
    {
    }
    Imp(Imp&&) = delete;
    auto operator=(const Imp&) -> Imp& = delete;
    auto operator=(Imp&&) -> Imp& = delete;
};

AccountEvent::AccountEvent(
    const UnallocatedCString& account,
    AccountEventType type,
    const UnallocatedCString& contact,
    const UnallocatedCString& workflow,
    const UnallocatedCString& amountS,
    const UnallocatedCString& pendingS,
    Amount amount,
    Amount pending,
    opentxs::Time time,
    const UnallocatedCString& memo,
    const UnallocatedCString& uuid,
    int state) noexcept(false)
    : imp_(std::make_unique<Imp>(
               Imp::default_version_,
               account,
               type,
               contact,
               workflow,
               amountS,
               pendingS,
               amount,
               pending,
               time,
               memo,
               uuid,
               state)
               .release())
{
}

AccountEvent::AccountEvent(const proto::AccountEvent& in) noexcept(false)
    : imp_(std::make_unique<Imp>(
               in.version(),
               in.id(),
               translate(in.type()),
               in.contact(),
               in.workflow(),
               in.amountformatted(),
               in.pendingamountformatted(),
               factory::Amount(in.amount()),
               factory::Amount(in.pendingamount()),
               convert_stime(in.timestamp()),
               in.memo(),
               in.uuid(),
               in.state())
               .release())
{
}

AccountEvent::AccountEvent(const AccountEvent& rhs) noexcept
    : imp_(std::make_unique<Imp>(*rhs.imp_).release())
{
    assert(nullptr != imp_);
}

AccountEvent::AccountEvent(AccountEvent&& rhs) noexcept
    : imp_(std::make_unique<Imp>().release())
{
    std::swap(imp_, rhs.imp_);

    assert(nullptr != imp_);
}

auto AccountEvent::AccountID() const noexcept -> const UnallocatedCString&
{
    return imp_->account_;
}

auto AccountEvent::ConfirmedAmount() const noexcept -> Amount
{
    return imp_->amount_;
}

auto AccountEvent::ConfirmedAmount_str() const noexcept
    -> const UnallocatedCString&
{
    return imp_->amount_formatted_;
}

auto AccountEvent::ContactID() const noexcept -> const UnallocatedCString&
{
    return imp_->contact_;
}

auto AccountEvent::Memo() const noexcept -> const UnallocatedCString&
{
    return imp_->memo_;
}

auto AccountEvent::PendingAmount() const noexcept -> Amount
{
    return imp_->pending_;
}

auto AccountEvent::PendingAmount_str() const noexcept
    -> const UnallocatedCString&
{
    return imp_->pending_formatted_;
}

auto AccountEvent::Serialize(proto::AccountEvent& dest) const noexcept -> bool
{
    const auto& imp = *imp_;
    dest.set_version(std::max(Imp::default_version_, imp.version_));
    dest.set_id(imp.account_);
    dest.set_type(translate(imp.type_));
    dest.set_contact(imp.contact_);
    dest.set_workflow(imp.workflow_);
    imp.amount_.Serialize(writer(dest.mutable_amount()));
    imp.pending_.Serialize(writer(dest.mutable_pendingamount()));
    dest.set_timestamp(Clock::to_time_t(imp.time_));
    dest.set_memo(imp.memo_);
    dest.set_uuid(imp.uuid_);
    dest.set_state(static_cast<proto::PaymentWorkflowState>(imp.state_));
    dest.set_amountformatted(imp.amount_formatted_);
    dest.set_pendingamountformatted(imp.pending_formatted_);

    return true;
}

auto AccountEvent::State() const noexcept -> int { return imp_->state_; }

auto AccountEvent::Timestamp() const noexcept -> Time { return imp_->time_; }

auto AccountEvent::Type() const noexcept -> AccountEventType
{
    return imp_->type_;
}

auto AccountEvent::UUID() const noexcept -> const UnallocatedCString&
{
    return imp_->uuid_;
}

auto AccountEvent::WorkflowID() const noexcept -> const UnallocatedCString&
{
    return imp_->workflow_;
}

AccountEvent::~AccountEvent()
{
    std::unique_ptr<Imp>{imp_}.reset();
    imp_ = nullptr;
}
}  // namespace opentxs::rpc
