// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <QMetaObject>
#include <QObject>
#include <QString>
#include <QValidator>
#include <memory>

class QObject;
class QString;

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace ui
{
namespace implementation
{
class DraftValidator;
}  // namespace implementation

namespace internal
{
struct ActivityThread;
}  // namespace internal
}  // namespace ui
// }  // namespace v1
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

    DraftValidator(internal::ActivityThread& parent) noexcept;
    DraftValidator() = delete;
    DraftValidator(const DraftValidator&) = delete;
    DraftValidator(DraftValidator&&) = delete;
    auto operator=(const DraftValidator&) -> DraftValidator& = delete;
    auto operator=(DraftValidator&&) -> DraftValidator& = delete;

    ~DraftValidator() final = default;

private:
    internal::ActivityThread& parent_;
};
}  // namespace opentxs::ui::implementation
