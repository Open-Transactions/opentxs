// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <QMetaObject>
#include <QObject>
#include <QString>

#include "opentxs/Export.hpp"
#include "opentxs/interface/qt/Model.hpp"

class QObject;

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace ui
{
namespace internal
{
struct ActivitySummary;
}  // namespace internal

class ActivitySummaryQt;
}  // namespace ui
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
