// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <QObject>
#include <QString>
#include <QValidator>
#include <memory>

class QObject;

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace ui
{
namespace implementation
{
class AccountActivity;
}  // namespace implementation

class AmountValidator;
}  // namespace ui
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

class OPENTXS_EXPORT opentxs::ui::AmountValidator final : public QValidator
{
    Q_OBJECT
    Q_PROPERTY(int maxDecimals READ getMaxDecimals WRITE setMaxDecimals)
    Q_PROPERTY(int minDecimals READ getMinDecimals WRITE setMinDecimals)
    Q_PROPERTY(int scale READ getScale WRITE setScale)

signals:
    void scaleChanged(int previousValue);

public slots:
    void setMaxDecimals(int);
    void setMinDecimals(int);
    void setScale(int);

public:
    // NOLINTNEXTLINE(modernize-use-trailing-return-type)
    Q_INVOKABLE QString revise(QString& input, int previousValue) const;

public:
    int getMaxDecimals() const;
    int getMinDecimals() const;
    int getScale() const;
    void fixup(QString& input) const final;
    State validate(QString& input, int& pos) const final;

    AmountValidator(implementation::AccountActivity& parent) noexcept;

    ~AmountValidator() final;

private:
    struct Imp;

    std::unique_ptr<Imp> imp_;

    AmountValidator() = delete;
    AmountValidator(const AmountValidator&) = delete;
    AmountValidator(AmountValidator&&) = delete;
    AmountValidator& operator=(const AmountValidator&) = delete;
    AmountValidator& operator=(AmountValidator&&) = delete;
};
