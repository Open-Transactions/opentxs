// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                           // IWYU pragma: associated
#include "opentxs/interface/rpc/AccountData.hpp"  // IWYU pragma: associated

#include <AccountData.pb.h>
#include <algorithm>
#include <cassert>
#include <memory>
#include <utility>

#include "internal/core/Factory.hpp"
#include "internal/interface/rpc/RPC.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/interface/rpc/AccountType.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Numbers.hpp"
#include "opentxs/util/Writer.hpp"

namespace opentxs::rpc
{
struct AccountData::Imp {
    static constexpr auto default_version_ = VersionNumber{2};

    const VersionNumber version_;
    const UnallocatedCString id_;
    const UnallocatedCString name_;
    const UnallocatedCString unit_;
    const UnallocatedCString owner_;
    const UnallocatedCString issuer_;
    const UnallocatedCString balance_formatted_;
    const UnallocatedCString pending_formatted_;
    const Amount balance_;
    const Amount pending_;
    const AccountType type_;

    Imp(const VersionNumber version,
        const UnallocatedCString& id,
        const UnallocatedCString& name,
        const UnallocatedCString& unit,
        const UnallocatedCString& owner,
        const UnallocatedCString& issuer,
        const UnallocatedCString& balanceF,
        const UnallocatedCString& pendingF,
        Amount balance,
        Amount pending,
        AccountType type) noexcept(false)
        : version_(version)
        , id_(id)
        , name_(name)
        , unit_(unit)
        , owner_(owner)
        , issuer_(issuer)
        , balance_formatted_(balanceF)
        , pending_formatted_(balanceF)
        , balance_(balance)
        , pending_(pending)
        , type_(type)
    {
        // FIXME validate member variables
    }
    Imp() noexcept
        : Imp(0, "", "", "", "", "", "", "", 0, 0, AccountType::error)
    {
    }
    Imp(const Imp& rhs) noexcept
        : Imp(rhs.version_,
              rhs.id_,
              rhs.name_,
              rhs.unit_,
              rhs.owner_,
              rhs.issuer_,
              rhs.balance_formatted_,
              rhs.pending_formatted_,
              rhs.balance_,
              rhs.pending_,
              rhs.type_)
    {
    }
    Imp(Imp&&) = delete;
    auto operator=(const Imp&) -> Imp& = delete;
    auto operator=(Imp&&) -> Imp& = delete;
};

AccountData::AccountData(
    const UnallocatedCString& id,
    const UnallocatedCString& name,
    const UnallocatedCString& unit,
    const UnallocatedCString& owner,
    const UnallocatedCString& issuer,
    const UnallocatedCString& balanceS,
    const UnallocatedCString& pendingS,
    Amount balance,
    Amount pending,
    AccountType type) noexcept(false)
    : imp_(std::make_unique<Imp>(
               Imp::default_version_,
               id,
               name,
               unit,
               owner,
               issuer,
               balanceS,
               pendingS,
               balance,
               pending,
               type)
               .release())
{
}

AccountData::AccountData(const proto::AccountData& in) noexcept(false)
    : imp_(std::make_unique<Imp>(
               in.version(),
               in.id(),
               in.label(),
               in.unit(),
               in.owner(),
               in.issuer(),
               in.balanceformatted(),
               in.pendingbalanceformatted(),
               factory::Amount(in.balance()),
               factory::Amount(in.pendingbalance()),
               translate(in.type()))
               .release())
{
}

AccountData::AccountData(const AccountData& rhs) noexcept
    : imp_(std::make_unique<Imp>(*rhs.imp_).release())
{
    assert(nullptr != imp_);
}

AccountData::AccountData(AccountData&& rhs) noexcept
    : imp_(std::make_unique<Imp>().release())
{
    std::swap(imp_, rhs.imp_);

    assert(nullptr != imp_);
}

auto AccountData::ConfirmedBalance() const noexcept -> Amount
{
    return imp_->balance_;
}

auto AccountData::ConfirmedBalance_str() const noexcept -> UnallocatedCString
{
    return imp_->balance_formatted_;
}

auto AccountData::ID() const noexcept -> const UnallocatedCString&
{
    return imp_->id_;
}

auto AccountData::Issuer() const noexcept -> const UnallocatedCString&
{
    return imp_->issuer_;
}

auto AccountData::Name() const noexcept -> const UnallocatedCString&
{
    return imp_->name_;
}

auto AccountData::Owner() const noexcept -> const UnallocatedCString&
{
    return imp_->owner_;
}

auto AccountData::PendingBalance() const noexcept -> Amount
{
    return imp_->pending_;
}

auto AccountData::PendingBalance_str() const noexcept -> UnallocatedCString
{
    return imp_->pending_formatted_;
}

auto AccountData::Serialize(proto::AccountData& dest) const noexcept -> bool
{
    const auto& imp = *imp_;
    dest.set_version(std::max(Imp::default_version_, imp.version_));
    dest.set_id(imp.id_);
    dest.set_label(imp.name_);
    dest.set_unit(imp.unit_);
    dest.set_owner(imp.owner_);
    dest.set_issuer(imp.issuer_);
    imp.balance_.Serialize(writer(dest.mutable_balance()));
    imp.pending_.Serialize(writer(dest.mutable_pendingbalance()));
    dest.set_type(translate(imp.type_));
    dest.set_balanceformatted(imp.balance_formatted_);
    dest.set_pendingbalanceformatted(imp.pending_formatted_);

    return true;
}

auto AccountData::Type() const noexcept -> AccountType { return imp_->type_; }

auto AccountData::Unit() const noexcept -> const UnallocatedCString&
{
    return imp_->unit_;
}

AccountData::~AccountData()
{
    std::unique_ptr<Imp>{imp_}.reset();
    imp_ = nullptr;
}
}  // namespace opentxs::rpc
