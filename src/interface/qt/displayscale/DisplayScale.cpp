// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/interface/qt/DisplayScale.hpp"  // IWYU pragma: associated

#include <QString>

#include "internal/util/Size.hpp"
#include "opentxs/display/Definition.hpp"
#include "opentxs/display/Types.hpp"

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

        using Index = display::ScaleIndex;
        const auto name = data_.ScaleName(static_cast<Index>(index.row()));

        return QString::fromUtf8(name.data(), size_to_int(name.size()));
    } catch (...) {

        return {};
    }
}

auto DisplayScaleQt::defaultScale() const noexcept -> int
{
    return static_cast<int>(data_.DefaultScale());
}

auto DisplayScaleQt::rowCount(const QModelIndex&) const -> int
{
    return static_cast<int>(data_.ScaleCount());
}
}  // namespace opentxs::ui
