// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                // IWYU pragma: associated
#include "opentxs/otx/blind/Mint.hpp"  // IWYU pragma: associated

#include <memory>
#include <utility>

#include "internal/core/String.hpp"
#include "internal/otx/common/Contract.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/Pimpl.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
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
        unit.GetString(api.Crypto(), out);

        return out;
    }())
{
}
}  // namespace opentxs::otx::blind::internal

namespace opentxs::otx::blind
{
Mint::Imp::Imp(const api::Session& api) noexcept
    : otx::blind::internal::Mint(api)
    , need_release_(true)
{
}

Mint::Imp::Imp(
    const api::Session& api,
    const identifier::UnitDefinition& unit) noexcept
    : otx::blind::internal::Mint(api, unit)
    , need_release_(true)
{
}

auto Mint::Imp::AccountID() const -> identifier::Generic
{
    return identifier::Generic{};
}

auto Mint::Imp::InstrumentDefinitionID() const
    -> const identifier::UnitDefinition&
{
    static const auto blank = identifier::UnitDefinition{};

    return blank;
}

auto Mint::Imp::Release_Mint() -> void { need_release_ = false; }

Mint::Imp::~Imp()
{
    if (need_release_) { Imp::Release_Mint(); }
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

auto Mint::AccountID() const -> identifier::Generic
{
    return imp_->AccountID();
}

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
