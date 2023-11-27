// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/api/network/ZAP.hpp"  // IWYU pragma: associated

#include "opentxs/api/network/ZAP.internal.hpp"

namespace opentxs::api::network
{
ZAP::ZAP(internal::ZAP* imp) noexcept
    : imp_(imp)
{
}

auto ZAP::Internal() const noexcept -> const internal::ZAP& { return *imp_; }

auto ZAP::Internal() noexcept -> internal::ZAP& { return *imp_; }

auto ZAP::RegisterDomain(std::string_view domain, std::string_view handler)
    const noexcept -> bool
{
    return imp_->RegisterDomain(domain, handler);
}

auto ZAP::SetDefaultPolicy(ZAPPolicy policy) const noexcept -> bool
{
    return imp_->SetDefaultPolicy(policy);
}

ZAP::~ZAP()
{
    if (nullptr != imp_) {
        delete imp_;
        imp_ = nullptr;
    }
}
}  // namespace opentxs::api::network
