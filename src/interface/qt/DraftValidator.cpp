// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "interface/qt/DraftValidator.hpp"  // IWYU pragma: associated

#include <QString>

#include "internal/interface/ui/UI.hpp"

namespace opentxs::ui::implementation
{
DraftValidator::DraftValidator(internal::ContactActivity& parent) noexcept
    : parent_(parent)
{
}

auto DraftValidator::fixup(QString& input) const -> void
{
    if (false == parent_.CanMessage()) { input.clear(); }

    if (false == parent_.SetDraft(input.toStdString())) { input.clear(); }
}

auto DraftValidator::validate(QString& input, int&) const -> State
{
    if (false == parent_.CanMessage()) {
        input.clear();

        return State::Invalid;
    }

    if (0 == input.size()) { return State::Intermediate; }

    if (parent_.SetDraft(input.toStdString())) { return State::Acceptable; }

    input.clear();

    return Invalid;
}
}  // namespace opentxs::ui::implementation
