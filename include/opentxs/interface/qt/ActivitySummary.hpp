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
struct ActivitySummary;
}  // namespace internal

class ActivitySummaryQt;
}  // namespace ui
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

class OPENTXS_EXPORT opentxs::ui::ActivitySummaryQt final : public qt::Model
{
    Q_OBJECT

public:
    ActivitySummaryQt(internal::ActivitySummary& parent) noexcept;
    ActivitySummaryQt(const ActivitySummaryQt&) = delete;
    ActivitySummaryQt(ActivitySummaryQt&&) = delete;
    auto operator=(const ActivitySummaryQt&) -> ActivitySummaryQt& = delete;
    auto operator=(ActivitySummaryQt&&) -> ActivitySummaryQt& = delete;

    ~ActivitySummaryQt() final;

private:
    struct Imp;

    Imp* imp_;
};
