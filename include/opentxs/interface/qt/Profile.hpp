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
struct Profile;
}  // namespace internal

class ProfileQt;
}  // namespace ui
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

class OPENTXS_EXPORT opentxs::ui::ProfileQt final : public qt::Model
{
    Q_OBJECT
    Q_PROPERTY(QString displayName READ displayName NOTIFY displayNameChanged)
    Q_PROPERTY(QString nymID READ nymID CONSTANT)
    Q_PROPERTY(QString paymentCode READ paymentCode NOTIFY paymentCodeChanged)

Q_SIGNALS:
    void displayNameChanged(QString) const;
    void paymentCodeChanged(QString) const;

public:
    // Tree layout
    auto displayName() const noexcept -> QString;
    auto nymID() const noexcept -> QString;
    auto paymentCode() const noexcept -> QString;

    ProfileQt(internal::Profile& parent) noexcept;
    ProfileQt(const ProfileQt&) = delete;
    ProfileQt(ProfileQt&&) = delete;
    auto operator=(const ProfileQt&) -> ProfileQt& = delete;
    auto operator=(ProfileQt&&) -> ProfileQt& = delete;

    ~ProfileQt() final;

private:
    struct Imp;

    Imp* imp_;
};
