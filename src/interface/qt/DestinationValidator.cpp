// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/interface/qt/DestinationValidator.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <cctype>

#include "interface/qt/DestinationValidator.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/core/AccountType.hpp"  // IWYU pragma: keep
#include "opentxs/core/Types.hpp"
#include "opentxs/util/Container.hpp"

namespace opentxs::ui
{
auto DestinationValidator::Imp::strip_invalid(
    QString& input,
    bool cashaddr) noexcept -> void
{
    const auto test = [&](const char c) {
        return (false == std::isalnum(c)) && (cashaddr ? (c != ':') : true);
    };
    auto raw = input.toStdString();
    raw.erase(std::remove_if(raw.begin(), raw.end(), test), raw.end());
    input = QString{raw.c_str()};
}

DestinationValidator::DestinationValidator(
    const api::session::Client& api,
    std::int8_t type,
    const identifier::Generic& account,
    implementation::AccountActivity& parent) noexcept
    : imp_(
          (AccountType::Blockchain == static_cast<AccountType>(type))
              ? Imp::Blockchain(api, *this, account, parent)
              : Imp::Custodial(api, parent))
{
    OT_ASSERT(imp_);
}

auto DestinationValidator::fixup(QString& input) const -> void
{
    imp_->fixup(input);
}

auto DestinationValidator::getDetails() const -> QString
{
    return imp_->getDetails();
}

auto DestinationValidator::validate(QString& input, int& pos) const -> State
{
    return imp_->validate(input, pos);
}

DestinationValidator::~DestinationValidator() = default;
}  // namespace opentxs::ui
