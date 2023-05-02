// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/interface/qt/ActivityThread.hpp"  // IWYU pragma: associated

#include <QDateTime>
#include <QObject>
#include <QString>
#include <QVariant>
#include <memory>

#include "interface/qt/DraftValidator.hpp"
#include "interface/ui/activitythread/ActivityThreadItem.hpp"
#include "internal/interface/ui/UI.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/Types.hpp"
#include "opentxs/core/identifier/Account.hpp"
#include "opentxs/otx/client/Types.hpp"
#include "opentxs/util/Time.hpp"
#include "util/Polarity.hpp"  // IWYU pragma: keep

namespace opentxs::factory
{
auto ActivityThreadQtModel(ui::internal::ActivityThread& parent) noexcept
    -> std::unique_ptr<ui::ActivityThreadQt>
{
    using ReturnType = ui::ActivityThreadQt;

    return std::make_unique<ReturnType>(parent);
}
}  // namespace opentxs::factory

namespace opentxs::ui
{
struct ActivityThreadQt::Imp {
    internal::ActivityThread& parent_;
    implementation::DraftValidator validator_;

    Imp(internal::ActivityThread& parent) noexcept
        : parent_(parent)
        , validator_(parent_)
    {
    }
};

ActivityThreadQt::ActivityThreadQt(internal::ActivityThread& parent) noexcept
    : Model(parent.GetQt())
    , imp_(std::make_unique<Imp>(parent).release())
{
    if (nullptr != internal_) {
        internal_->SetColumnCount(nullptr, 7);
        internal_->SetRoleData({
            {ActivityThreadQt::AmountRole, "amount"},
            {ActivityThreadQt::LoadingRole, "loading"},
            {ActivityThreadQt::MemoRole, "memo"},
            {ActivityThreadQt::PendingRole, "pending"},
            {ActivityThreadQt::PolarityRole, "polarity"},
            {ActivityThreadQt::TextRole, "text"},
            {ActivityThreadQt::TimeRole, "time"},
            {ActivityThreadQt::TypeRole, "type"},
            {ActivityThreadQt::OutgoingRole, "outgoing"},
            {ActivityThreadQt::FromRole, "from"},
        });
    }

    imp_->parent_.SetCallbacks({
        []() -> void {},
        [this]() -> void { Q_EMIT displayNameUpdate(); },
        [this]() -> void { Q_EMIT draftUpdate(); },
        [this](bool value) -> void { Q_EMIT canMessageUpdate(value); },
    });
}

auto ActivityThreadQt::canMessage() const noexcept -> bool
{
    return imp_->parent_.CanMessage();
}

auto ActivityThreadQt::displayName() const noexcept -> QString
{
    return imp_->parent_.DisplayName().c_str();
}

auto ActivityThreadQt::draft() const noexcept -> QString
{
    return imp_->parent_.GetDraft().c_str();
}

auto ActivityThreadQt::draftValidator() const noexcept -> QValidator*
{
    return &(imp_->validator_);
}

auto ActivityThreadQt::headerData(int section, Qt::Orientation, int role)
    const noexcept -> QVariant
{
    if (Qt::DisplayRole != role) { return {}; }

    switch (section) {
        case TimeColumn: {
            return "Time";
        }
        case FromColumn: {
            return "Sender";
        }
        case TextColumn: {
            return "Event";
        }
        case AmountColumn: {
            return "Amount";
        }
        case MemoColumn: {
            return "Memo";
        }
        case LoadingColumn: {
            return "Loading";
        }
        case PendingColumn: {
            return "Pending";
        }
        default: {

            return {};
        }
    }
}

auto ActivityThreadQt::participants() const noexcept -> QString
{
    return imp_->parent_.Participants().c_str();
}

auto ActivityThreadQt::pay(
    const QString& amount,
    const QString& sourceAccount,
    const QString& memo) const noexcept -> bool
{
    return imp_->parent_.Pay(
        amount.toStdString(),
        imp_->parent_.API().Factory().AccountIDFromBase58(
            sourceAccount.toStdString()),
        memo.toStdString(),
        otx::client::PaymentType::Cheque);
}

auto ActivityThreadQt::paymentCode(const int currency) const noexcept -> QString
{
    return imp_->parent_.PaymentCode(static_cast<UnitType>(currency)).c_str();
}

auto ActivityThreadQt::sendDraft() const noexcept -> bool
{
    return imp_->parent_.SendDraft();
}

auto ActivityThreadQt::setDraft(QString draft) -> void
{
    imp_->parent_.SetDraft(draft.toStdString());
}

auto ActivityThreadQt::threadID() const noexcept -> QString
{
    return imp_->parent_.ThreadID().c_str();
}

ActivityThreadQt::~ActivityThreadQt()
{
    if (nullptr != imp_) {
        delete imp_;
        imp_ = nullptr;
    }
}
}  // namespace opentxs::ui

namespace opentxs::ui::implementation
{
auto ActivityThreadItem::qt_data(
    const int column,
    const int role,
    QVariant& out) const noexcept -> void
{
    switch (role) {
        case Qt::DisplayRole: {
            switch (column) {
                case ActivityThreadQt::TimeColumn: {
                    qt_data(column, ActivityThreadQt::TimeRole, out);
                } break;
                case ActivityThreadQt::FromColumn: {
                    qt_data(column, ActivityThreadQt::FromRole, out);
                } break;
                case ActivityThreadQt::TextColumn: {
                    qt_data(column, ActivityThreadQt::TextRole, out);
                } break;
                case ActivityThreadQt::AmountColumn: {
                    qt_data(column, ActivityThreadQt::AmountRole, out);
                } break;
                case ActivityThreadQt::MemoColumn: {
                    qt_data(column, ActivityThreadQt::MemoRole, out);
                } break;
                case ActivityThreadQt::LoadingColumn:
                case ActivityThreadQt::PendingColumn:
                default: {
                }
            }
        } break;
        case Qt::CheckStateRole: {
            switch (column) {
                case ActivityThreadQt::LoadingColumn: {
                    qt_data(column, ActivityThreadQt::LoadingRole, out);
                } break;
                case ActivityThreadQt::PendingColumn: {
                    qt_data(column, ActivityThreadQt::PendingRole, out);
                } break;
                case ActivityThreadQt::TextColumn:
                case ActivityThreadQt::AmountColumn:
                case ActivityThreadQt::MemoColumn:
                case ActivityThreadQt::TimeColumn:
                default: {
                }
            }
        } break;
        case ActivityThreadQt::AmountRole: {
            out = DisplayAmount().c_str();
        } break;
        case ActivityThreadQt::LoadingRole: {
            out = Loading();
        } break;
        case ActivityThreadQt::MemoRole: {
            out = Memo().c_str();
        } break;
        case ActivityThreadQt::PendingRole: {
            out = Pending();
        } break;
        case ActivityThreadQt::PolarityRole: {
            out = polarity(Amount());
        } break;
        case ActivityThreadQt::TextRole: {
            out = Text().c_str();
        } break;
        case ActivityThreadQt::TimeRole: {
            auto output = QDateTime{};
            output.setSecsSinceEpoch(Clock::to_time_t(Timestamp()));

            out = output;
        } break;
        case ActivityThreadQt::TypeRole: {
            out = static_cast<int>(Type());
        } break;
        case ActivityThreadQt::OutgoingRole: {
            out = Outgoing();
        } break;
        case ActivityThreadQt::FromRole: {
            out = From().c_str();
        } break;
        default: {
        }
    }
}
}  // namespace opentxs::ui::implementation
