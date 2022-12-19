// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <QMetaObject>
#include <QString>
#include <QValidator>
#include <cstdint>
#include <memory>

#include "opentxs/Export.hpp"

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

namespace identifier
{
class Generic;
}  // namespace identifier

namespace ui
{
namespace implementation
{
class AccountActivity;
}  // namespace implementation

}  // namespace ui
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::ui
{
class OPENTXS_EXPORT DestinationValidator final : public QValidator
{
    Q_OBJECT
    Q_PROPERTY(QString details READ getDetails NOTIFY detailsChanged)

Q_SIGNALS:
    void detailsChanged(const QString&);

public:
    struct Imp;

    auto getDetails() const -> QString;
    auto fixup(QString& input) const -> void final;
    auto validate(QString& input, int& pos) const -> State final;

    OPENTXS_NO_EXPORT DestinationValidator(
        const api::session::Client&,
        std::int8_t,
        const identifier::Generic&,
        implementation::AccountActivity&) noexcept;
    DestinationValidator() = delete;
    DestinationValidator(const DestinationValidator&) = delete;
    DestinationValidator(DestinationValidator&&) = delete;
    auto operator=(const DestinationValidator&)
        -> DestinationValidator& = delete;
    auto operator=(DestinationValidator&&) -> DestinationValidator& = delete;

    OPENTXS_NO_EXPORT ~DestinationValidator() final;

private:
    std::unique_ptr<Imp> imp_;
};
}  // namespace opentxs::ui
