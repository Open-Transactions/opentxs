// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <QMetaObject>
#include <QObject>

#include "opentxs/Export.hpp"
#include "opentxs/interface/qt/Model.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace ui
{
namespace internal
{
struct NymList;
}  // namespace internal
}  // namespace ui
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::ui
{
class OPENTXS_EXPORT NymListQt final : public qt::Model
{
    Q_OBJECT

public:
    enum Roles {
        IDRole = Qt::UserRole + 0,    // QString
        NameRole = Qt::UserRole + 1,  // QString
    };
    // This model is designed to be used in a list view
    enum Columns {
        NameColumn = 0,
    };

    OPENTXS_NO_EXPORT NymListQt(internal::NymList& parent) noexcept;
    NymListQt() = delete;
    NymListQt(const NymListQt&) = delete;
    NymListQt(NymListQt&&) = delete;
    auto operator=(const NymListQt&) -> NymListQt& = delete;
    auto operator=(NymListQt&&) -> NymListQt& = delete;

    OPENTXS_NO_EXPORT ~NymListQt() final;

private:
    struct Imp;

    Imp* imp_;
};
}  // namespace opentxs::ui
