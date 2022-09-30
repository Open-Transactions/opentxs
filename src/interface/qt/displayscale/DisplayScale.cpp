// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                           // IWYU pragma: associated
#include "opentxs/interface/qt/DisplayScale.hpp"  // IWYU pragma: associated

#include <QString>

#include "opentxs/core/display/Definition.hpp"
#include "opentxs/util/Container.hpp"

namespace opentxs::ui
{
DisplayScaleQt::DisplayScaleQt(const display::Definition& data) noexcept
    : data_(data)
{
}

DisplayScaleQt::DisplayScaleQt(const DisplayScaleQt& rhs) noexcept
    : DisplayScaleQt(rhs.data_)
{
}

auto DisplayScaleQt::data(const QModelIndex& index, int role) const -> QVariant
{
    if (false == index.isValid()) { return {}; }

    if (Qt::DisplayRole != role) { return {}; }

    try {
        const auto row = index.row();

        if (0 > row) { return {}; }

        using Index = display::Definition::Index;

        return QString{
            data_.GetScales().at(static_cast<Index>(index.row())).c_str()};
    } catch (...) {

        return {};
    }
}

auto DisplayScaleQt::rowCount(const QModelIndex&) const -> int
{
    return static_cast<int>(data_.GetScales().size());
}
}  // namespace opentxs::ui
