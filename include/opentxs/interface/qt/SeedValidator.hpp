// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <QList>
#include <QMetaObject>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QValidator>
#include <QVector>
#include <cstdint>
#include <memory>

class QObject;

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace api
{
namespace session
{
class Client;
}  // namespace session
}  // namespace api

namespace ui
{
class SeedValidator;
}  // namespace ui
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

class OPENTXS_EXPORT opentxs::ui::SeedValidator final : public QValidator
{
    Q_OBJECT

public:
    // NOLINTBEGIN(modernize-use-trailing-return-type)
    using Phrase = QVector<QString>;

    Q_INVOKABLE bool checkPhrase(const Phrase& phrase) const;
    // NOLINTEND(modernize-use-trailing-return-type)

public:
    auto fixup(QString& input) const -> void final;
    auto validate(QString& input, int& pos) const -> State final;

    SeedValidator(
        const api::session::Client&,
        std::uint8_t,
        std::uint8_t) noexcept;
    SeedValidator() = delete;
    SeedValidator(const SeedValidator&) = delete;
    SeedValidator(SeedValidator&&) = delete;
    auto operator=(const SeedValidator&) -> SeedValidator& = delete;
    auto operator=(SeedValidator&&) -> SeedValidator& = delete;

    ~SeedValidator() final;

private:
    struct Imp;

    std::unique_ptr<Imp> imp_;
};
