// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <QString>
#include <QValidator>
#include <memory>

#include "interface/ui/accountactivity/AccountActivity.hpp"
#include "opentxs/identifier/Account.hpp"
#include "opentxs/interface/qt/DestinationValidator.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace session
{
class Client;
}  // namespace session
}  // namespace api
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::ui
{
struct DestinationValidator::Imp {
    using Parent = implementation::AccountActivity;

    static auto Blockchain(
        const api::session::Client& api,
        DestinationValidator& main,
        const identifier::Account& account,
        Parent& parent) noexcept -> std::unique_ptr<Imp>;
    static auto Custodial(
        const api::session::Client& api,
        Parent& parent) noexcept -> std::unique_ptr<Imp>;

    virtual auto fixup(QString& input) const -> void = 0;
    virtual auto getDetails() const -> QString = 0;
    virtual auto validate(QString& input, int& pos) const
        -> QValidator::State = 0;

    virtual ~Imp() = default;

protected:
    static auto strip_invalid(QString& input, bool cashaddr = false) noexcept
        -> void;
};
}  // namespace opentxs::ui
