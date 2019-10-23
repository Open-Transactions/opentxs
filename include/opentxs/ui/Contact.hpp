// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_UI_CONTACT_HPP
#define OPENTXS_UI_CONTACT_HPP

#ifndef Q_MOC_RUN

#include "opentxs/Forward.hpp"

#include "opentxs/ui/List.hpp"
#include "opentxs/Proto.hpp"

#include <string>

#ifdef SWIG
// clang-format off
%rename(UIContact) opentxs::ui::Contact;
// clang-format on
#endif  // SWIG

namespace opentxs
{
namespace ui
{
namespace implementation
{
class Contact;
}  // namespace implementation

class Contact : virtual public List
{
public:
#if OT_QT
    // Data method needs this. And it needs to match the parent type for the Qt
    using qt_super = QAbstractItemModel;  // class that appears below this one.
#endif

    EXPORT virtual std::string ContactID() const noexcept = 0;
    EXPORT virtual std::string DisplayName() const noexcept = 0;
    EXPORT virtual opentxs::SharedPimpl<opentxs::ui::ContactSection> First()
        const noexcept = 0;
    EXPORT virtual opentxs::SharedPimpl<opentxs::ui::ContactSection> Next()
        const noexcept = 0;
    EXPORT virtual std::string PaymentCode() const noexcept = 0;

    EXPORT ~Contact() override = default;

protected:
    Contact() noexcept = default;

private:
    Contact(const Contact&) = delete;
    Contact(Contact&&) = delete;
    Contact& operator=(const Contact&) = delete;
    Contact& operator=(Contact&&) = delete;
};
}  // namespace ui
}  // namespace opentxs
#endif

#if OT_QT || defined(Q_MOC_RUN)
class opentxs::ui::ContactQt final : public QAbstractItemModel
{
    Q_OBJECT

public:
    using ConstructorCallback = std::function<
        implementation::Contact*(RowCallbacks insert, RowCallbacks remove)>;

    QString displayName() const noexcept;
    QString contactID() const noexcept;
    QString paymentCode() const noexcept;

    int columnCount(const QModelIndex& parent = QModelIndex()) const
        noexcept final;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const
        noexcept final;
    QModelIndex index(
        int row,
        int column,
        const QModelIndex& parent = QModelIndex()) const noexcept final;
    QModelIndex parent(const QModelIndex& index) const noexcept final;
    QHash<int, QByteArray> roleNames() const noexcept final;
    int rowCount(const QModelIndex& parent = QModelIndex()) const
        noexcept final;

    const Contact& operator*() const noexcept;

    // Throws std::runtime_error if callback returns invalid pointer
    ContactQt(ConstructorCallback cb) noexcept(false);
    ~ContactQt() final;

signals:
    void updated() const;

private:
    Q_PROPERTY(QString displayName READ displayName NOTIFY updated)
    Q_PROPERTY(QString contactID READ contactID NOTIFY updated)
    Q_PROPERTY(QString paymentCode READ paymentCode NOTIFY updated)

    std::unique_ptr<implementation::Contact> parent_;

    void notify() const noexcept;
    void finish_row_add() noexcept;
    void finish_row_delete() noexcept;
    void start_row_add(const QModelIndex& parent, int first, int last) noexcept;
    void start_row_delete(
        const QModelIndex& parent,
        int first,
        int last) noexcept;

    ContactQt() = delete;
    ContactQt(const ContactQt&) = delete;
    ContactQt(ContactQt&&) = delete;
    ContactQt& operator=(const ContactQt&) = delete;
    ContactQt& operator=(ContactQt&&) = delete;
};
#endif
#endif
