// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                     // IWYU pragma: associated
#include "1_Internal.hpp"                   // IWYU pragma: associated
#include "otx/consensus/ManagedNumber.hpp"  // IWYU pragma: associated

#include "internal/otx/consensus/Consensus.hpp"
#include "internal/util/Flag.hpp"
#include "opentxs/otx/consensus/ManagedNumber.hpp"
#include "opentxs/otx/consensus/Server.hpp"

namespace opentxs
{
auto operator<(const OTManagedNumber& lhs, const OTManagedNumber& rhs) -> bool
{
    return lhs->Value() < rhs->Value();
}
}  // namespace opentxs

namespace opentxs::factory
{
auto ManagedNumber(
    const TransactionNumber number,
    otx::context::Server& context) -> otx::context::ManagedNumber*
{
    using ReturnType = otx::context::implementation::ManagedNumber;

    return new ReturnType(number, context);
}
}  // namespace opentxs::factory

namespace opentxs::otx::context::implementation
{
ManagedNumber::ManagedNumber(
    const TransactionNumber number,
    otx::context::Server& context)
    : context_(context)
    , number_(number)
    , success_(Flag::Factory(false))
    , managed_(0 != number)
{
}

void ManagedNumber::SetSuccess(const bool value) const { success_->Set(value); }

auto ManagedNumber::Valid() const -> bool { return managed_; }

auto ManagedNumber::Value() const -> TransactionNumber { return number_; }

ManagedNumber::~ManagedNumber()
{
    if (false == managed_) { return; }

    if (success_.get()) { return; }

    context_.RecoverAvailableNumber(number_);
}
}  // namespace opentxs::otx::context::implementation
