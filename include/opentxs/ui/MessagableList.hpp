// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_UI_MESSAGABLELIST_HPP
#define OPENTXS_UI_MESSAGABLELIST_HPP

#ifndef Q_MOC_RUN
#include "opentxs/Forward.hpp"

#include "opentxs/ui/List.hpp"

#ifdef SWIG
// clang-format off
%rename(UIMessagableList) opentxs::ui::MessagableList;
// clang-format on
#endif  // SWIG

namespace opentxs
{
namespace ui
{
namespace implementation
{
class MessagableList;
}  // namespace implementation

class MessagableList : virtual public List
{
public:
#if OT_QT
    // Data method needs this. And it needs to match the parent type for the Qt
    using qt_super = QAbstractItemModel;  // class that appears below this one.
#endif

    EXPORT virtual opentxs::SharedPimpl<opentxs::ui::ContactListItem> First()
        const noexcept = 0;
    EXPORT virtual opentxs::SharedPimpl<opentxs::ui::ContactListItem> Next()
        const noexcept = 0;

    EXPORT ~MessagableList() override = default;

protected:
    MessagableList() noexcept = default;

private:
    MessagableList(const MessagableList&) = delete;
    MessagableList(MessagableList&&) = delete;
    MessagableList& operator=(const MessagableList&) = delete;
    MessagableList& operator=(MessagableList&&) = delete;
};
}  // namespace ui
}  // namespace opentxs
#endif

#if OT_QT || defined(Q_MOC_RUN)
class opentxs::ui::MessagableListQt final : public QAbstractItemModel
{
    Q_OBJECT

public:
    using ConstructorCallback = std::function<implementation::MessagableList*(
        RowCallbacks insert,
        RowCallbacks remove)>;

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

    const MessagableList& operator*() const noexcept;

    // Throws std::runtime_error if callback returns invalid pointer
    MessagableListQt(ConstructorCallback cb) noexcept(false);
    ~MessagableListQt() final;

signals:
    void updated() const;

private:
    std::unique_ptr<implementation::MessagableList> parent_;

    void notify() const noexcept;
    void finish_row_add() noexcept;
    void finish_row_delete() noexcept;
    void start_row_add(const QModelIndex& parent, int first, int last) noexcept;
    void start_row_delete(
        const QModelIndex& parent,
        int first,
        int last) noexcept;

    MessagableListQt() = delete;
    MessagableListQt(const MessagableListQt&) = delete;
    MessagableListQt(MessagableListQt&&) = delete;
    MessagableListQt& operator=(const MessagableListQt&) = delete;
    MessagableListQt& operator=(MessagableListQt&&) = delete;
};
#endif
#endif
