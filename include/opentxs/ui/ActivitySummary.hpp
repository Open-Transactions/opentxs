// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_UI_ACTIVITYSUMMARY_HPP
#define OPENTXS_UI_ACTIVITYSUMMARY_HPP

#ifndef Q_MOC_RUN

#include "opentxs/Forward.hpp"

#include "opentxs/ui/List.hpp"

#ifdef SWIG
// clang-format off
%rename(UIActivitySummary) opentxs::ui::ActivitySummary;
// clang-format on
#endif  // SWIG

namespace opentxs
{
namespace ui
{
namespace implementation
{
class ActivitySummary;
}  // namespace implementation

class ActivitySummary : virtual public List
{
public:
#if OT_QT
    // Data method needs this. And it needs to match the parent type for the Qt
    using qt_super = QAbstractItemModel;  // class that appears below this one.
#endif
    EXPORT virtual opentxs::SharedPimpl<opentxs::ui::ActivitySummaryItem>
    First() const noexcept = 0;
    EXPORT virtual opentxs::SharedPimpl<opentxs::ui::ActivitySummaryItem> Next()
        const noexcept = 0;

    EXPORT ~ActivitySummary() override = default;

protected:
    ActivitySummary() = default;

private:
    ActivitySummary(const ActivitySummary&) = delete;
    ActivitySummary(ActivitySummary&&) = delete;
    ActivitySummary& operator=(const ActivitySummary&) = delete;
    ActivitySummary& operator=(ActivitySummary&&) = delete;
};
}  // namespace ui
}  // namespace opentxs
#endif

#if OT_QT || defined(Q_MOC_RUN)
class opentxs::ui::ActivitySummaryQt final : public QAbstractItemModel
{
    Q_OBJECT

public:
    using ConstructorCallback = std::function<implementation::ActivitySummary*(
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

    const ActivitySummary& operator*() const noexcept;

    // Throws std::runtime_error if callback returns invalid pointer
    ActivitySummaryQt(ConstructorCallback cb) noexcept(false);
    ~ActivitySummaryQt() final;

signals:
    void updated() const;

private:
    std::unique_ptr<implementation::ActivitySummary> parent_;

    void notify() const noexcept;
    void finish_row_add() noexcept;
    void finish_row_delete() noexcept;
    void start_row_add(const QModelIndex& parent, int first, int last) noexcept;
    void start_row_delete(
        const QModelIndex& parent,
        int first,
        int last) noexcept;

    ActivitySummaryQt() = delete;
    ActivitySummaryQt(const ActivitySummaryQt&) = delete;
    ActivitySummaryQt(ActivitySummaryQt&&) = delete;
    ActivitySummaryQt& operator=(const ActivitySummaryQt&) = delete;
    ActivitySummaryQt& operator=(ActivitySummaryQt&&) = delete;
};
#endif
#endif
