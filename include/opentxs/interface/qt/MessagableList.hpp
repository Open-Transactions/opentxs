// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <QMetaObject>
#include <QObject>
#include <QString>

#include "opentxs/interface/qt/Model.hpp"

class QObject;

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace ui
{
namespace internal
{
struct MessagableList;
}  // namespace internal

class MessagableListQt;
}  // namespace ui
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

class OPENTXS_EXPORT opentxs::ui::MessagableListQt final : public qt::Model
{
    Q_OBJECT
public:
    // List layout
    enum Roles {
        ContactIDRole = Qt::UserRole + 0,
        SectionRole = Qt::UserRole + 1,
    };

    MessagableListQt(internal::MessagableList& parent) noexcept;
    MessagableListQt() = delete;
    MessagableListQt(const MessagableListQt&) = delete;
    MessagableListQt(MessagableListQt&&) = delete;
    auto operator=(const MessagableListQt&) -> MessagableListQt& = delete;
    auto operator=(MessagableListQt&&) -> MessagableListQt& = delete;

    ~MessagableListQt() final;

private:
    struct Imp;

    Imp* imp_;
};
