// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/interface/qt/ContactActivity.hpp"  // IWYU pragma: associated

#include <QDateTime>
#include <QObject>
#include <QString>
#include <QVariant>
#include <memory>

#include "interface/qt/DraftValidator.hpp"
#include "interface/ui/contactactivity/ContactActivityItem.hpp"
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
auto ContactActivityQtModel(ui::internal::ContactActivity& parent) noexcept
    -> std::unique_ptr<ui::ContactActivityQt>
{
    using ReturnType = ui::ContactActivityQt;

    return std::make_unique<ReturnType>(parent);
}
}  // namespace opentxs::factory

namespace opentxs::ui
{
struct ContactActivityQt::Imp {
    internal::ContactActivity& parent_;
    implementation::DraftValidator validator_;

    Imp(internal::ContactActivity& parent) noexcept
        : parent_(parent)
        , validator_(parent_)
    {
    }
};

ContactActivityQt::ContactActivityQt(internal::ContactActivity& parent) noexcept
    : Model(parent.GetQt())
    , imp_(std::make_unique<Imp>(parent).release())
{
    if (nullptr != internal_) {
        internal_->SetColumnCount(nullptr, 8);
        internal_->SetRoleData({
            {ContactActivityQt::AmountRole, "amount"},
            {ContactActivityQt::LoadingRole, "loading"},
            {ContactActivityQt::MemoRole, "memo"},
            {ContactActivityQt::PendingRole, "pending"},
            {ContactActivityQt::PolarityRole, "polarity"},
            {ContactActivityQt::TextRole, "text"},
            {ContactActivityQt::TimeRole, "timestamp"},
            {ContactActivityQt::TypeRole, "type"},
            {ContactActivityQt::OutgoingRole, "outgoing"},
            {ContactActivityQt::FromRole, "from"},
            {ContactActivityQt::UUIDRole, "uuid"},
        });
    }

    imp_->parent_.SetCallbacks({
        []() -> void {},
        [this]() -> void { Q_EMIT displayNameUpdate(); },
        [this]() -> void { Q_EMIT draftUpdate(); },
        [this](bool value) -> void { Q_EMIT canMessageUpdate(value); },
    });
}

auto ContactActivityQt::canMessage() const noexcept -> bool
{
    return imp_->parent_.CanMessage();
}

auto ContactActivityQt::displayName() const noexcept -> QString
{
    return imp_->parent_.DisplayName().c_str();
}

auto ContactActivityQt::draft() const noexcept -> QString
{
    return imp_->parent_.GetDraft().c_str();
}

auto ContactActivityQt::draftValidator() const noexcept -> QValidator*
{
    return &(imp_->validator_);
}

auto ContactActivityQt::headerData(int section, Qt::Orientation, int role)
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
        case TxidColumn: {
            return "TransactionID";
        }
        default: {

            return {};
        }
    }
}

auto ContactActivityQt::participants() const noexcept -> QString
{
    return imp_->parent_.Participants().c_str();
}

auto ContactActivityQt::pay(
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

auto ContactActivityQt::paymentCode(const int currency) const noexcept
    -> QString
{
    return imp_->parent_.PaymentCode(static_cast<UnitType>(currency)).c_str();
}

auto ContactActivityQt::sendDraft() const noexcept -> bool
{
    return imp_->parent_.SendDraft();
}

auto ContactActivityQt::sendFaucetRequest(const int currency) const noexcept
    -> bool
{
    return imp_->parent_.SendFaucetRequest(static_cast<UnitType>(currency));
}

auto ContactActivityQt::setDraft(QString draft) -> void
{
    imp_->parent_.SetDraft(draft.toStdString());
}

auto ContactActivityQt::threadID() const noexcept -> QString
{
    return imp_->parent_.ThreadID().c_str();
}

ContactActivityQt::~ContactActivityQt()
{
    if (nullptr != imp_) {
        delete imp_;
        imp_ = nullptr;
    }
}
}  // namespace opentxs::ui

namespace opentxs::ui::implementation
{
auto ContactActivityItem::qt_data(
    const int column,
    const int role,
    QVariant& out) const noexcept -> void
{
    switch (role) {
        case Qt::DisplayRole: {
            switch (column) {
                case ContactActivityQt::TimeColumn: {
                    qt_data(column, ContactActivityQt::TimeRole, out);
                } break;
                case ContactActivityQt::FromColumn: {
                    qt_data(column, ContactActivityQt::FromRole, out);
                } break;
                case ContactActivityQt::TextColumn: {
                    qt_data(column, ContactActivityQt::TextRole, out);
                } break;
                case ContactActivityQt::AmountColumn: {
                    qt_data(column, ContactActivityQt::AmountRole, out);
                } break;
                case ContactActivityQt::MemoColumn: {
                    qt_data(column, ContactActivityQt::MemoRole, out);
                } break;
                case ContactActivityQt::TxidColumn: {
                    qt_data(column, ContactActivityQt::UUIDRole, out);
                } break;
                case ContactActivityQt::LoadingColumn:
                case ContactActivityQt::PendingColumn:
                default: {
                }
            }
        } break;
        case Qt::CheckStateRole: {
            switch (column) {
                case ContactActivityQt::LoadingColumn: {
                    qt_data(column, ContactActivityQt::LoadingRole, out);
                } break;
                case ContactActivityQt::PendingColumn: {
                    qt_data(column, ContactActivityQt::PendingRole, out);
                } break;
                case ContactActivityQt::TextColumn:
                case ContactActivityQt::AmountColumn:
                case ContactActivityQt::MemoColumn:
                case ContactActivityQt::TimeColumn:
                default: {
                }
            }
        } break;
        case ContactActivityQt::AmountRole: {
            out = DisplayAmount().c_str();
        } break;
        case ContactActivityQt::LoadingRole: {
            out = Loading();
        } break;
        case ContactActivityQt::MemoRole: {
            out = Memo().c_str();
        } break;
        case ContactActivityQt::PendingRole: {
            out = Pending();
        } break;
        case ContactActivityQt::PolarityRole: {
            out = polarity(Amount());
        } break;
        case ContactActivityQt::TextRole: {
            out = Text().c_str();
        } break;
        case ContactActivityQt::TimeRole: {
            auto output = QDateTime{};
            output.setSecsSinceEpoch(Clock::to_time_t(Timestamp()));

            out = output;
        } break;
        case ContactActivityQt::TypeRole: {
            out = static_cast<int>(Type());
        } break;
        case ContactActivityQt::OutgoingRole: {
            out = Outgoing();
        } break;
        case ContactActivityQt::FromRole: {
            out = From().c_str();
        } break;
        case ContactActivityQt::UUIDRole: {
            out = TXID().c_str();
        } break;
        default: {
        }
    }
}
}  // namespace opentxs::ui::implementation
