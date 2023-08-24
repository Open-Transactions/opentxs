// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/interface/qt/NymType.hpp"  // IWYU pragma: associated

#include <QString>
#include <array>
#include <cstddef>
#include <string_view>
#include <utility>

#include "internal/util/Size.hpp"
#include "opentxs/identity/IdentityType.hpp"  // IWYU pragma: keep
#include "opentxs/identity/Types.hpp"

namespace opentxs::ui
{
using namespace std::literals;

class NymTypePrivate
{
public:
    using enum identity::Type;
    static constexpr auto data_ =
        std::array<std::pair<identity::Type, std::string_view>, 6>{{
            {individual, "Individual"sv},
            {organization, "Organization"sv},
            {business, "Business"sv},
            {government, "Government"sv},
            {server, "Server"sv},
            {bot, "Bot"sv},
        }};
};
}  // namespace opentxs::ui

namespace opentxs::ui
{
NymType::NymType() noexcept
    : imp_(nullptr)
{
}

auto NymType::data(const QModelIndex& index, int role) const noexcept
    -> QVariant
{
    const auto& data = NymTypePrivate::data_;
    const auto row = index.row();

    if (row < 0) { return {}; }

    const auto effectiveRow = static_cast<std::size_t>(row);

    if (effectiveRow >= data.size()) { return {}; }

    const auto& [type, name] = data[effectiveRow];

    switch (role) {
        case NymTypeValue: {

            return static_cast<int>(type);
        }
        case Qt::DisplayRole:
        case NymTypeName: {
            return QString::fromUtf8(name.data(), size_to_int(name.size()));
        }
        default: {

            return {};
        }
    }
}

auto NymType::headerData(int section, Qt::Orientation orientation, int role)
    const noexcept -> QVariant
{
    if (Qt::DisplayRole != role) { return {}; }

    switch (section) {
        case 0: {
            return "Name";
        }
        default: {

            return {};
        }
    }
}

auto NymType::rowCount(const QModelIndex& parent) const noexcept -> int
{
    return static_cast<int>(NymTypePrivate::data_.size());
}

NymType::~NymType() = default;
}  // namespace opentxs::ui
