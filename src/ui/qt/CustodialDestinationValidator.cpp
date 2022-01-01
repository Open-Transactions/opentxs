// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                    // IWYU pragma: associated
#include "1_Internal.hpp"                  // IWYU pragma: associated
#include "ui/qt/DestinationValidator.hpp"  // IWYU pragma: associated

#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/util/Pimpl.hpp"

namespace opentxs::ui
{
struct CustodialDestionationValidator final : public DestinationValidator::Imp {
    const api::session::Client& api_;

    auto fixup(QString& input) const -> void final
    {
        strip_invalid(input, false);
    }
    auto getDetails() const -> QString final { return {}; }
    auto validate(QString& input, int& pos) const -> QValidator::State final
    {
        const auto id =
            api_.Factory().IdentifierFromBase58(input.toStdString());

        if (id->empty()) {

            return QValidator::State::Invalid;
        } else {

            return QValidator::State::Acceptable;
        }
    }

    CustodialDestionationValidator(
        const api::session::Client& api,
        Parent& parent) noexcept
        : api_(api)
    {
    }

    ~CustodialDestionationValidator() final = default;
};

auto DestinationValidator::Imp::Custodial(
    const api::session::Client& api,
    Parent& parent) noexcept -> std::unique_ptr<Imp>
{
    return std::make_unique<CustodialDestionationValidator>(api, parent);
}
}  // namespace opentxs::ui
