// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <QAbstractListModel>
#include <tuple>

#include "opentxs/identity/IdentityType.hpp"
#include "opentxs/identity/Types.hpp"
#include "opentxs/util/Container.hpp"

namespace opentxs::ui::identitymanager
{
class NymType final : public QAbstractListModel
{
public:
    auto data(const QModelIndex& index, int role) const noexcept
        -> QVariant final
    {
        try {
            const auto row = index.row();

            if (0 > row) { return {}; }

            const auto& data = data_.at(static_cast<std::size_t>(row));

            switch (role) {
                case Qt::DisplayRole: {
                    switch (index.column()) {
                        case 0: {

                            return QString::fromStdString(data.second);
                        }
                        default: {
                        }
                    }
                } break;
                case Qt::UserRole: {

                    return static_cast<int>(data.first);
                }
                default: {
                }
            }
        } catch (...) {
        }

        return {};
    }
    auto rowCount(const QModelIndex& parent) const noexcept -> int final
    {
        static const auto blank = QModelIndex{};

        if (parent == blank) {

            return static_cast<int>(data_.size());
        } else {

            return 0;
        }
    }

    NymType() noexcept
        : data_([] {
            auto out = Data{};
            out.reserve(6);
            out.emplace_back(std::make_pair(
                identity::Type::individual,
                opentxs::print(identity::Type::individual)));
            out.emplace_back(std::make_pair(
                identity::Type::organization,
                opentxs::print(identity::Type::organization)));
            out.emplace_back(std::make_pair(
                identity::Type::business,
                opentxs::print(identity::Type::business)));
            out.emplace_back(std::make_pair(
                identity::Type::government,
                opentxs::print(identity::Type::government)));
            out.emplace_back(std::make_pair(
                identity::Type::server,
                opentxs::print(identity::Type::server)));
            out.emplace_back(std::make_pair(
                identity::Type::bot, opentxs::print(identity::Type::bot)));
            std::sort(
                out.begin(),
                out.end(),
                [](const auto& lhs, const auto& rhs) -> bool {
                    return lhs.second < rhs.second;
                });

            return out;
        }())
    {
    }

    ~NymType() final = default;

private:
    using Data =
        UnallocatedVector<std::pair<identity::Type, UnallocatedCString>>;

    const Data data_;
};
}  // namespace opentxs::ui::identitymanager
