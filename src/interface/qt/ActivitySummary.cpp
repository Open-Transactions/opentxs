// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/interface/qt/ActivitySummary.hpp"  // IWYU pragma: associated

#include <QDateTime>
#include <QVariant>
#include <memory>

#include "interface/ui/activitysummary/ActivitySummaryItem.hpp"
#include "internal/interface/ui/UI.hpp"
#include "opentxs/util/Time.hpp"

namespace opentxs::factory
{
auto ActivitySummaryQtModel(ui::internal::ActivitySummary& parent) noexcept
    -> std::unique_ptr<ui::ActivitySummaryQt>
{
    using ReturnType = ui::ActivitySummaryQt;

    return std::make_unique<ReturnType>(parent);
}
}  // namespace opentxs::factory

namespace opentxs::ui
{
struct ActivitySummaryQt::Imp {
    internal::ActivitySummary& parent_;

    Imp(internal::ActivitySummary& parent)
        : parent_(parent)
    {
    }
};

ActivitySummaryQt::ActivitySummaryQt(internal::ActivitySummary& parent) noexcept
    : Model(parent.GetQt())
    , imp_(std::make_unique<Imp>(parent).release())
{
    if (nullptr != internal_) { internal_->SetColumnCount(nullptr, 6); }
}

ActivitySummaryQt::~ActivitySummaryQt()
{
    if (nullptr != imp_) {
        delete imp_;
        imp_ = nullptr;
    }
}
}  // namespace opentxs::ui

namespace opentxs::ui::implementation
{
auto ActivitySummaryItem::qt_data(
    const int column,
    const int role,
    QVariant& out) const noexcept -> void
{
    switch (column) {
        case 0: {
            out = ThreadID().c_str();
        } break;
        case 1: {
            out = DisplayName().c_str();
        } break;
        case 2: {
            out = ImageURI().c_str();
        } break;
        case 3: {
            out = Text().c_str();
        } break;
        case 4: {
            QDateTime qdatetime;
            qdatetime.setSecsSinceEpoch(Clock::to_time_t(Timestamp()));
            out = qdatetime;
        } break;
        case 5: {
            out = static_cast<int>(Type());
        } break;
        default: {
        }
    }
}
}  // namespace opentxs::ui::implementation
