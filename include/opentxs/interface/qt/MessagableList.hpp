// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <QMetaObject>
#include <QObject>

#include "opentxs/Export.hpp"
#include "opentxs/interface/qt/Model.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace ui
{
namespace internal
{
struct MessagableList;
}  // namespace internal
}  // namespace ui
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::ui
{
class OPENTXS_EXPORT MessagableListQt final : public qt::Model
{
    Q_OBJECT
public:
    // List layout
    enum Roles {
        ContactIDRole = Qt::UserRole + 0,
        SectionRole = Qt::UserRole + 1,
    };

    OPENTXS_NO_EXPORT MessagableListQt(
        internal::MessagableList& parent) noexcept;
    MessagableListQt() = delete;
    MessagableListQt(const MessagableListQt&) = delete;
    MessagableListQt(MessagableListQt&&) = delete;
    auto operator=(const MessagableListQt&) -> MessagableListQt& = delete;
    auto operator=(MessagableListQt&&) -> MessagableListQt& = delete;

    OPENTXS_NO_EXPORT ~MessagableListQt() final;

private:
    struct Imp;

    Imp* imp_;
};
}  // namespace opentxs::ui
