// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <QMetaObject>
#include <QValidator>

class QString;

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace ui
{

namespace internal
{
struct ContactActivity;
}  // namespace internal
}  // namespace ui
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::ui::implementation
{
class DraftValidator final : public QValidator
{
    Q_OBJECT

public:
    auto fixup(QString& input) const -> void final;
    auto validate(QString& input, int& pos) const -> State final;

    DraftValidator(internal::ContactActivity& parent) noexcept;
    DraftValidator() = delete;
    DraftValidator(const DraftValidator&) = delete;
    DraftValidator(DraftValidator&&) = delete;
    auto operator=(const DraftValidator&) -> DraftValidator& = delete;
    auto operator=(DraftValidator&&) -> DraftValidator& = delete;

    ~DraftValidator() final = default;

private:
    internal::ContactActivity& parent_;
};
}  // namespace opentxs::ui::implementation
