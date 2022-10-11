// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"  // IWYU pragma: associated
#include "interface/ui/accountactivity/BalanceItem.hpp"  // IWYU pragma: associated
#include "opentxs/interface/qt/AccountActivity.hpp"  // IWYU pragma: associated

#include <QDateTime>
#include <QList>
#include <QObject>
#include <QString>
#include <algorithm>
#include <cstdint>
#include <iterator>
#include <memory>
#include <utility>

#include "core/Worker.hpp"
#include "interface/qt/SendMonitor.hpp"
#include "interface/ui/accountactivity/AccountActivity.hpp"
#include "internal/core/contract/Unit.hpp"
#include "internal/interface/ui/AccountActivity.hpp"
#include "internal/interface/ui/UI.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/core/display/Definition.hpp"
#include "opentxs/interface/qt/AmountValidator.hpp"
#include "opentxs/interface/qt/DestinationValidator.hpp"
#include "opentxs/interface/qt/DisplayScale.hpp"
#include "opentxs/util/Container.hpp"
#include "util/Polarity.hpp"

namespace opentxs::factory
{
auto AccountActivityQtModel(ui::internal::AccountActivity& parent) noexcept
    -> std::unique_ptr<ui::AccountActivityQt>
{
    using ReturnType = ui::AccountActivityQt;

    return std::make_unique<ReturnType>(parent);
}
}  // namespace opentxs::factory

namespace opentxs::ui
{
struct AccountActivityQt::Imp {
    internal::AccountActivity& parent_;

    Imp(internal::AccountActivity& parent)
        : parent_(parent)
    {
    }
};

AccountActivityQt::AccountActivityQt(internal::AccountActivity& parent) noexcept
    : Model(parent.GetQt())
    , imp_(std::make_unique<Imp>(parent).release())
{
    if (nullptr != internal_) {
        internal_->SetColumnCount(nullptr, 6);
        internal_->SetRoleData({
            {AccountActivityQt::AmountRole, "amount"},
            {AccountActivityQt::TextRole, "description"},
            {AccountActivityQt::MemoRole, "memo"},
            {AccountActivityQt::TimeRole, "timestamp"},
            {AccountActivityQt::UUIDRole, "uuid"},
            {AccountActivityQt::PolarityRole, "polarity"},
            {AccountActivityQt::ContactsRole, "contacts"},
            {AccountActivityQt::WorkflowRole, "workflow"},
            {AccountActivityQt::TypeRole, "type"},
            {AccountActivityQt::ConfirmationsRole, "confirmations"},
        });
    }

    imp_->parent_.SetCallbacks(
        {[this](int current, int max, double percent) {
             emit syncPercentageUpdated(percent);
             emit syncProgressUpdated(current, max);
         },
         [this](UnallocatedCString balance) {
             emit balanceChanged(balance.c_str());
         },
         [this](int polarity) { emit balancePolarityChanged(polarity); }});
}

auto AccountActivityQt::accountID() const noexcept -> QString
{
    return imp_->parent_.AccountID().c_str();
}

auto AccountActivityQt::balancePolarity() const noexcept -> int
{
    return imp_->parent_.BalancePolarity();
}

auto AccountActivityQt::depositChains() const noexcept -> QVariantList
{
    const auto input = imp_->parent_.DepositChains();
    auto output = QVariantList{};
    std::transform(
        std::begin(input), std::end(input), std::back_inserter(output), [
        ](const auto& in) -> auto{ return static_cast<int>(in); });

    return output;
}

auto AccountActivityQt::displayBalance() const noexcept -> QString
{
    return imp_->parent_.DisplayBalance().c_str();
}

auto AccountActivityQt::getAmountValidator() const noexcept -> AmountValidator*
{
    return &imp_->parent_.AmountValidator();
}

auto AccountActivityQt::getDepositAddress(const int chain) const noexcept
    -> QString
{
    return imp_->parent_.DepositAddress(static_cast<blockchain::Type>(chain))
        .c_str();
}

auto AccountActivityQt::getDestValidator() const noexcept
    -> DestinationValidator*
{
    return &imp_->parent_.DestinationValidator();
}

auto AccountActivityQt::getScaleModel() const noexcept -> DisplayScaleQt*
{
    return &imp_->parent_.DisplayScaleQt();
}

auto AccountActivityQt::headerData(int section, Qt::Orientation, int role)
    const noexcept -> QVariant
{
    if (Qt::DisplayRole != role) { return {}; }

    switch (section) {
        case TimeColumn: {
            return "Time";
        }
        case TextColumn: {
            return "Event";
        }
        case AmountColumn: {
            return "Amount";
        }
        case UUIDColumn: {
            return "Transaction ID";
        }
        case MemoColumn: {
            return "Memo";
        }
        case ConfirmationsColumn: {
            return "Confirmations";
        }
        default: {

            return {};
        }
    }
}

auto AccountActivityQt::sendToAddress(
    const QString& address,
    const QString& amount,
    const QString& memo,
    int scale) const noexcept -> int
{
    if (0 > scale) { return false; }

    return imp_->parent_.Send(
        address.toStdString(),
        amount.toStdString(),
        memo.toStdString(),
        static_cast<AccountActivity::Scale>(scale),
        [this](auto key, auto code, auto text) {
            emit transactionSendResult(key, code, text);
        });
}

auto AccountActivityQt::sendToContact(
    const QString& contactID,
    const QString& amount,
    const QString& memo,
    int scale) const noexcept -> int
{
    if (0 > scale) { return -1; }

    try {
        return imp_->parent_.Send(
            imp_->parent_.API().Factory().IdentifierFromBase58(
                contactID.toStdString()),
            amount.toStdString(),
            memo.toStdString(),
            static_cast<AccountActivity::Scale>(scale),
            [this](auto key, auto code, auto text) {
                emit transactionSendResult(key, code, text);
            });
    } catch (...) {

        return -1;
    }
}

auto AccountActivityQt::syncPercentage() const noexcept -> double
{
    return imp_->parent_.SyncPercentage();
}

auto AccountActivityQt::syncProgress() const noexcept -> QVariantList
{
    const auto progress = imp_->parent_.SyncProgress();
    auto out = QVariantList{};
    out.push_back(progress.first);
    out.push_back(progress.second);

    return out;
}

auto AccountActivityQt::validateAddress(const QString& address) const noexcept
    -> bool
{
    return imp_->parent_.ValidateAddress(address.toStdString());
}

auto AccountActivityQt::validateAmount(const QString& amount) const noexcept
    -> QString
{
    return imp_->parent_.ValidateAmount(amount.toStdString()).c_str();
}

AccountActivityQt::~AccountActivityQt()
{
    imp_->parent_.SendMonitor().shutdown();

    if (nullptr != imp_) {
        delete imp_;
        imp_ = nullptr;
    }
}
}  // namespace opentxs::ui

namespace opentxs::ui::implementation
{
struct AccountActivity::QT {
    ui::DisplayScaleQt scales_qt_;
    ui::AmountValidator amount_validator_;
    ui::DestinationValidator destination_validator_;
    mutable implementation::SendMonitor send_monitor_;

    QT(AccountActivity& parent)
    noexcept
        : scales_qt_(display::GetDefinition(parent.Contract().UnitOfAccount()))
        , amount_validator_(parent)
        , destination_validator_(
              parent.Worker::api_,
              static_cast<std::int8_t>(parent.type_),
              parent.account_id_,
              parent)
        , send_monitor_()
    {
    }
};

auto AccountActivity::DisplayScaleQt() noexcept -> ui::DisplayScaleQt&
{
    return qt_->scales_qt_;
}

auto AccountActivity::AmountValidator() noexcept -> ui::AmountValidator&
{
    return qt_->amount_validator_;
}

auto AccountActivity::DestinationValidator() noexcept
    -> ui::DestinationValidator&
{
    return qt_->destination_validator_;
}

auto AccountActivity::init_qt() noexcept -> void
{
    qt_ = std::make_unique<QT>(*this).release();

    OT_ASSERT(qt_);
}

auto AccountActivity::SendMonitor() const noexcept
    -> implementation::SendMonitor&
{
    return qt_->send_monitor_;
}

auto AccountActivity::SendMonitor() noexcept -> implementation::SendMonitor&
{
    return qt_->send_monitor_;
}

auto AccountActivity::shutdown_qt() noexcept -> void
{
    if (nullptr != qt_) {
        SendMonitor().shutdown();
        delete qt_;
        qt_ = nullptr;
    }
}

auto BalanceItem::qt_data(const int column, const int role, QVariant& out)
    const noexcept -> void
{
    using Parent = AccountActivityQt;

    switch (role) {
        case Qt::TextAlignmentRole: {
            switch (column) {
                case Parent::TextColumn:
                case Parent::MemoColumn: {
                    out = Qt::AlignLeft;
                } break;
                default: {
                    out = Qt::AlignHCenter;
                } break;
            }
        } break;
        case Qt::DisplayRole: {
            switch (column) {
                case Parent::TimeColumn: {
                    qt_data(column, Parent::TimeRole, out);
                } break;
                case Parent::TextColumn: {
                    qt_data(column, Parent::TextRole, out);
                } break;
                case Parent::AmountColumn: {
                    qt_data(column, Parent::AmountRole, out);
                } break;
                case Parent::UUIDColumn: {
                    qt_data(column, Parent::UUIDRole, out);
                } break;
                case Parent::MemoColumn: {
                    qt_data(column, Parent::MemoRole, out);
                } break;
                case Parent::ConfirmationsColumn: {
                    qt_data(column, Parent::ConfirmationsRole, out);
                } break;
                default: {
                }
            }
        } break;
        case Parent::AmountRole: {
            out = DisplayAmount().c_str();
        } break;
        case Parent::TextRole: {
            out = Text().c_str();
        } break;
        case Parent::MemoRole: {
            out = Memo().c_str();
        } break;
        case Parent::TimeRole: {
            auto qdatetime = QDateTime{};
            qdatetime.setSecsSinceEpoch(Clock::to_time_t(Timestamp()));

            out = qdatetime;
        } break;
        case Parent::UUIDRole: {
            out = UUID().c_str();
        } break;
        case Parent::PolarityRole: {
            out = polarity(Amount());
        } break;
        case Parent::ContactsRole: {
            auto output = QStringList{};

            for (const auto& contact : Contacts()) {
                output << contact.c_str();
            }

            out = output;
        } break;
        case Parent::WorkflowRole: {
            out = Workflow().c_str();
        } break;
        case Parent::TypeRole: {
            out = static_cast<int>(Type());
        } break;
        case Parent::ConfirmationsRole: {
            out = Confirmations();
        } break;
        default: {
        }
    }
}
}  // namespace opentxs::ui::implementation
