// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                     // IWYU pragma: associated
#include "1_Internal.hpp"                   // IWYU pragma: associated
#include "otx/consensus/ManagedNumber.hpp"  // IWYU pragma: associated

#include <utility>

#include "internal/otx/consensus/Consensus.hpp"
#include "internal/util/Flag.hpp"
#include "opentxs/otx/consensus/ManagedNumber.hpp"
#include "opentxs/otx/consensus/Server.hpp"

namespace opentxs::otx::context
{
auto operator<(const ManagedNumber& lhs, const ManagedNumber& rhs) noexcept
    -> bool
{
    return lhs.imp_->operator<(rhs);
}
}  // namespace opentxs::otx::context

namespace opentxs::factory
{
auto ManagedNumber(
    const TransactionNumber number,
    otx::context::Server& context) -> otx::context::ManagedNumber
{
    using ReturnType = otx::context::ManagedNumber;

    return ReturnType{new ReturnType::Imp{number, context}};
}
}  // namespace opentxs::factory

namespace opentxs::otx::context
{

ManagedNumber::ManagedNumber(Imp* imp) noexcept
    : imp_(imp)
{
}

ManagedNumber::ManagedNumber(ManagedNumber&& rhs) noexcept
    : imp_{nullptr}
{
    swap(rhs);
}

auto ManagedNumber::operator=(ManagedNumber&& rhs) noexcept -> ManagedNumber&
{
    swap(rhs);

    return *this;
}

ManagedNumber::~ManagedNumber()
{
    if (nullptr != imp_) {
        delete imp_;
        imp_ = nullptr;
    }
}

auto ManagedNumber::swap(ManagedNumber& rhs) noexcept -> void
{
    std::swap(imp_, rhs.imp_);
}

void ManagedNumber::SetSuccess(const bool value) const
{
    imp_->SetSuccess(value);
}

auto ManagedNumber::Valid() const -> bool { return imp_->Valid(); }

auto ManagedNumber::Value() const -> TransactionNumber { return imp_->Value(); }

ManagedNumber::Imp::Imp(
    const TransactionNumber number,
    otx::context::Server& context)
    : context_(context)
    , number_(number)
    , success_(Flag::Factory(false))
    , managed_(0 != number)
{
}

ManagedNumber::Imp::~Imp()
{
    if (false == managed_) { return; }

    if (success_.get()) { return; }

    context_.RecoverAvailableNumber(number_);
}

auto ManagedNumber::Imp::operator<(const ManagedNumber& rhs) const noexcept
    -> bool
{
    return Value() < rhs.Value();
}

void ManagedNumber::Imp::SetSuccess(const bool value) const
{
    success_->Set(value);
}

auto ManagedNumber::Imp::Valid() const -> bool { return managed_; }

auto ManagedNumber::Imp::Value() const -> TransactionNumber { return number_; }

}  // namespace opentxs::otx::context
