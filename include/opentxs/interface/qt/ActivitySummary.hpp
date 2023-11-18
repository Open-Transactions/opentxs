// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <QMetaObject>

#include "opentxs/Export.hpp"
#include "opentxs/interface/qt/Model.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace ui
{
namespace internal
{
struct ActivitySummary;
}  // namespace internal
}  // namespace ui
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::ui
{
class OPENTXS_EXPORT ActivitySummaryQt final : public qt::Model
{
    Q_OBJECT

public:
    OPENTXS_NO_EXPORT ActivitySummaryQt(
        internal::ActivitySummary& parent) noexcept;
    ActivitySummaryQt(const ActivitySummaryQt&) = delete;
    ActivitySummaryQt(ActivitySummaryQt&&) = delete;
    auto operator=(const ActivitySummaryQt&) -> ActivitySummaryQt& = delete;
    auto operator=(ActivitySummaryQt&&) -> ActivitySummaryQt& = delete;

    OPENTXS_NO_EXPORT ~ActivitySummaryQt() final;

private:
    struct Imp;

    Imp* imp_;
};
}  // namespace opentxs::ui
