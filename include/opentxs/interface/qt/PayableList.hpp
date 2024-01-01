// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <QMetaObject>
#include <QObject>
#include <QtQmlIntegration>

#include "opentxs/Export.hpp"
#include "opentxs/interface/qt/Model.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace ui
{
namespace internal
{
struct PayableList;
}  // namespace internal
}  // namespace ui
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::ui
{
class OPENTXS_EXPORT PayableListQt final : public qt::Model
{
    Q_OBJECT
    QML_ELEMENT
public:
    // Table layout: name, payment code
    enum Roles {
        ContactIDRole = Qt::UserRole + 0,
        SectionRole = Qt::UserRole + 1,
    };
    Q_ENUM(Roles)

    OPENTXS_NO_EXPORT PayableListQt(internal::PayableList& parent) noexcept;
    PayableListQt(const PayableListQt&) = delete;
    PayableListQt(PayableListQt&&) = delete;
    auto operator=(const PayableListQt&) -> PayableListQt& = delete;
    auto operator=(PayableListQt&&) -> PayableListQt& = delete;

    OPENTXS_NO_EXPORT ~PayableListQt() final;

private:
    struct Imp;

    Imp* imp_;
};
}  // namespace opentxs::ui
