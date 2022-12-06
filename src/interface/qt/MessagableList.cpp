// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/interface/qt/MessagableList.hpp"  // IWYU pragma: associated

#include <memory>

#include "internal/interface/ui/UI.hpp"

namespace opentxs::factory
{
auto MessagableListQtModel(ui::internal::MessagableList& parent) noexcept
    -> std::unique_ptr<ui::MessagableListQt>
{
    using ReturnType = ui::MessagableListQt;

    return std::make_unique<ReturnType>(parent);
}
}  // namespace opentxs::factory

namespace opentxs::ui
{
struct MessagableListQt::Imp {
    internal::MessagableList& parent_;

    Imp(internal::MessagableList& parent)
        : parent_(parent)
    {
    }
};

MessagableListQt::MessagableListQt(internal::MessagableList& parent) noexcept
    : Model(parent.GetQt())
    , imp_(std::make_unique<Imp>(parent).release())
{
    if (nullptr != internal_) {
        internal_->SetColumnCount(nullptr, 1);
        internal_->SetRoleData({
            {MessagableListQt::ContactIDRole, "id"},
            {MessagableListQt::SectionRole, "section"},
        });
    }
}

MessagableListQt::~MessagableListQt()
{
    if (nullptr != imp_) {
        delete imp_;
        imp_ = nullptr;
    }
}
}  // namespace opentxs::ui
