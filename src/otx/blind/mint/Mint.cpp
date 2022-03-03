// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                // IWYU pragma: associated
#include "1_Internal.hpp"              // IWYU pragma: associated
#include "opentxs/otx/blind/Mint.hpp"  // IWYU pragma: associated

#include <memory>
#include <utility>

#include "internal/otx/common/Contract.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "otx/blind/mint/Mint.hpp"

namespace opentxs::otx::blind::internal
{
Mint::Mint(const api::Session& api) noexcept
    : Contract(api)
{
}

Mint::Mint(
    const api::Session& api,
    const identifier::UnitDefinition& unit) noexcept
    : Contract(api, [&] {
        auto out = String::Factory();
        unit.GetString(out);

        return out;
    }())
{
}
}  // namespace opentxs::otx::blind::internal

namespace opentxs::otx::blind
{
Mint::Imp::Imp(const api::Session& api) noexcept
    : otx::blind::internal::Mint(api)
{
}

Mint::Imp::Imp(
    const api::Session& api,
    const identifier::UnitDefinition& unit) noexcept
    : otx::blind::internal::Mint(api, unit)
{
}

auto Mint::Imp::AccountID() const -> OTIdentifier
{
    return api_.Factory().Identifier();
}

auto Mint::Imp::InstrumentDefinitionID() const
    -> const identifier::UnitDefinition&
{
    static const auto blank = api_.Factory().UnitID();

    return blank.get();
}
}  // namespace opentxs::otx::blind

namespace opentxs::otx::blind
{
auto swap(Mint& lhs, Mint& rhs) noexcept -> void { lhs.swap(rhs); }

Mint::Mint(Imp* imp) noexcept
    : imp_(imp)
{
    OT_ASSERT(nullptr != imp);
}

Mint::Mint(const api::Session& api) noexcept
    : Mint(std::make_unique<Imp>(api).release())
{
}

Mint::Mint(Mint&& rhs) noexcept
    : Mint(rhs.Internal().API())
{
    swap(rhs);
}

Mint::operator bool() const noexcept { return imp_->isValid(); }

auto Mint::AccountID() const -> OTIdentifier { return imp_->AccountID(); }

auto Mint::Expired() const -> bool { return imp_->Expired(); }

auto Mint::GetDenomination(std::int32_t nIndex) const -> Amount
{
    return imp_->GetDenomination(nIndex);
}

auto Mint::GetDenominationCount() const -> std::int32_t
{
    return imp_->GetDenominationCount();
}

auto Mint::GetExpiration() const -> Time { return imp_->GetExpiration(); }

auto Mint::GetLargestDenomination(const Amount& lAmount) const -> Amount
{
    return imp_->GetLargestDenomination(lAmount);
}

auto Mint::GetPrivate(Armored& theArmor, const Amount& lDenomination) const
    -> bool
{
    return imp_->GetPrivate(theArmor, lDenomination);
}

auto Mint::GetPublic(Armored& theArmor, const Amount& lDenomination) const
    -> bool
{
    return imp_->GetPublic(theArmor, lDenomination);
}

auto Mint::GetSeries() const -> std::int32_t { return imp_->GetSeries(); }

auto Mint::GetValidFrom() const -> Time { return imp_->GetValidFrom(); }

auto Mint::GetValidTo() const -> Time { return imp_->GetValidTo(); }

auto Mint::InstrumentDefinitionID() const -> const identifier::UnitDefinition&
{
    return imp_->InstrumentDefinitionID();
}

auto Mint::Internal() const noexcept -> const otx::blind::internal::Mint&
{
    return *imp_;
}

auto Mint::Internal() noexcept -> otx::blind::internal::Mint& { return *imp_; }

auto Mint::operator=(Mint&& rhs) noexcept -> Mint&
{
    swap(rhs);

    return *this;
}

auto Mint::Release() noexcept -> otx::blind::internal::Mint*
{
    auto* output = std::make_unique<Imp>(imp_->API()).release();
    std::swap(imp_, output);

    return output;
}

auto Mint::swap(Mint& rhs) noexcept -> void { std::swap(imp_, rhs.imp_); }

Mint::~Mint()
{
    if (nullptr != imp_) {
        delete imp_;
        imp_ = nullptr;
    }
}
}  // namespace opentxs::otx::blind
