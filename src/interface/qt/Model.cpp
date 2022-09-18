// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                    // IWYU pragma: associated
#include "1_Internal.hpp"                  // IWYU pragma: associated
#include "opentxs/interface/qt/Model.hpp"  // IWYU pragma: associated

#include <QByteArray>
#include <QString>
#include <algorithm>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <memory>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <type_traits>
#include <utility>

#include "internal/interface/ui/UI.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/Mutex.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::ui::qt::internal
{
struct Model::Imp {
    using RowID = std::ptrdiff_t;

    static auto ID(const ui::internal::Row* ptr) noexcept -> RowID
    {
        if (nullptr == ptr) { return root_row_id_; }

        return ptr->index();
    }

    auto ChangeRow(ui::internal::Row* parent, ui::internal::Row* row) noexcept
        -> void
    {
        auto lock = Lock{parent_lock_};

        if (nullptr != parent_) { get_helper().requestChangeRow(parent, row); }
    }
    auto ClearParent() noexcept -> void
    {
        auto lock = Lock{parent_lock_};
        parent_ = nullptr;
    }
    auto DeleteRow(ui::internal::Row* row) noexcept -> void
    {
        auto lock = Lock{parent_lock_};

        if (nullptr != parent_) {
            get_helper().requestDeleteRow(row);
        } else {
            do_delete_row(lock, row);
        }
    }
    auto do_delete_row(ui::internal::Row* row) noexcept -> void
    {
        auto lock = Lock{data_lock_};
        do_delete_row(lock, row);
    }
    auto do_insert_row(
        ui::internal::Row* parent,
        ui::internal::Row* after,
        std::shared_ptr<ui::internal::Row> row) noexcept -> void
    {
        auto lock = Lock{data_lock_};
        do_insert_row(lock, parent, after, std::move(row));
    }
    auto do_move_row(
        ui::internal::Row* newParent,
        ui::internal::Row* newBefore,
        ui::internal::Row* row) noexcept -> void
    {
        auto lock = Lock{data_lock_};
        do_move_row(lock, newParent, newBefore, row);
    }
    auto GetChild(const ui::internal::Row* parent, int index) const noexcept
        -> ui::internal::Row*
    {
        try {
            if (0 > index) { throw std::runtime_error{"negative index"}; }

            const auto pos = static_cast<std::size_t>(index);
            auto lock = Lock{data_lock_};
            const auto& data = map_.at(ID(parent));

            if (pos >= data.children_.size()) {
                throw std::out_of_range{"child does not exist"};
            }

            const auto& child =
                map_.at(ID(*std::next(data.children_.begin(), pos)));

            return const_cast<ui::internal::Row*>(child.pointer_.get());
        } catch (const std::exception& e) {
            LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

            return nullptr;
        }
    }

    auto GetColumnCount(const ui::internal::Row* row) const noexcept -> int
    {
        auto lock = Lock{data_lock_};

        return get_column_count(lock, row);
    }

    auto GetIndex(const ui::internal::Row* item) const noexcept -> Index
    {
        try {
            auto lock = Lock{data_lock_};
            const auto& child = map_.at(ID(item));
            auto row = int{invalid_index_};
            const auto root = child.isRootNode();

            if (root) {
                row = 0;
            } else {
                const auto& parent = map_.at(ID(child.parent_));
                row = parent.FindChild(item);
            }

            if (invalid_index_ == row) {
                throw std::runtime_error{"child not found"};
            }

            return {!root, row, 0, child.pointer_.get()};
        } catch (const std::exception& e) {
            LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

            return {};
        }
    }

    auto GetParent(const ui::internal::Row* row) const noexcept -> Index
    {
        try {
            const auto& parent = [&]() -> auto&
            {
                auto lock = Lock{data_lock_};

                return map_.at(ID(row)).parent_;
            }
            ();

            return GetIndex(parent);
        } catch (const std::exception& e) {
            LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

            return {};
        }
    }

    auto GetRoleData() const noexcept -> RoleData
    {
        auto lock = Lock{data_lock_};

        return role_data_;
    }

    auto GetRoot() const noexcept -> QObject* { return qt_parent_; }

    auto GetRowCount(ui::internal::Row* row) const noexcept -> int
    {
        auto lock = Lock{data_lock_};

        try {

            return static_cast<int>(map_.at(ID(row)).children_.size());
        } catch (const std::exception& e) {
            LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

            return invalid_index_;
        }
    }
    auto InsertRow(
        ui::internal::Row* parent,
        ui::internal::Row* after,
        std::shared_ptr<ui::internal::Row> row) noexcept -> void
    {
        auto lock = Lock{parent_lock_};

        if (nullptr != parent_) {
            get_helper().requestInsertRow(parent, after, row);
        } else {
            do_insert_row(lock, parent, after, row);
        }
    }
    auto MoveRow(
        ui::internal::Row* newParent,
        ui::internal::Row* newBefore,
        ui::internal::Row* row) noexcept -> void
    {
        auto lock = Lock{parent_lock_};

        if (nullptr != parent_) {
            get_helper().requestMoveRow(newParent, newBefore, row);
        } else {
            do_move_row(lock, newParent, newBefore, row);
        }
    }
    auto SetColumnCount(Row* parent, int count) noexcept -> void
    {
        auto lock = Lock{data_lock_};

        SetColumnCount(lock, parent, count);
    }
    auto SetColumnCount(const Lock&, Row* parent, int count) noexcept -> void
    {
        try {
            map_.at(ID(parent)).column_count_ = count;
        } catch (const std::exception& e) {
            LogError()(OT_PRETTY_CLASS())(e.what()).Flush();
        }
    }
    auto SetParent(qt::Model& parent) noexcept -> void
    {
        auto lock = Lock{parent_lock_};
        parent_ = &parent;
    }
    auto SetRoleData(RoleData&& data) noexcept -> void
    {
        auto lock = Lock{data_lock_};
        role_data_ = std::move(data);
    }

    Imp(QObject* parent) noexcept
        : parent_lock_()
        , data_lock_()
        , qt_parent_(parent)
        , parent_(nullptr)
        , role_data_()
        , map_()
    {
        map_[ID(nullptr)];
    }
    Imp() = delete;
    Imp(const Imp&) = delete;
    Imp(Imp&&) = delete;
    auto operator=(const Imp&) -> Imp& = delete;
    auto operator=(Imp&&) -> Imp& = delete;

private:
    struct RowData {
        const ui::internal::Row* parent_;
        const std::shared_ptr<ui::internal::Row> pointer_;
        std::optional<int> column_count_;
        UnallocatedDeque<const ui::internal::Row*> children_;

        auto FindChild(const ui::internal::Row* item) const noexcept -> int
        {
            auto found{false};
            auto pos{invalid_index_};

            for (const auto& child : children_) {
                ++pos;

                if (child == item) {
                    found = true;

                    break;
                }
            }

            if (found) {

                return pos;
            } else {

                return invalid_index_;
            }
        }
        auto isRootNode() const noexcept -> bool { return !pointer_; }

        auto AddAfter(
            const ui::internal::Row* before,
            const ui::internal::Row* item) noexcept -> std::ptrdiff_t
        {
            auto pos = 0_z;

            if ((nullptr == before) || (0_uz == children_.size())) {
                children_.emplace_front(item);

                return pos;
            }

            for (auto i{children_.begin()}; i != children_.end(); ++i, ++pos) {
                if (*i == before) {
                    children_.insert(std::next(i), item);

                    return ++pos;
                }
            }

            LogError()(OT_PRETTY_CLASS())("insert position for row ")(
                ID(before))(" not found")
                .Flush();
            pos = static_cast<std::ptrdiff_t>(children_.size());
            children_.emplace_back(item);

            return pos;
        }
        auto Remove(const ui::internal::Row* item) noexcept -> void
        {
            auto match = [&](const auto in) { return in == item; };
            children_.erase(
                std::remove_if(children_.begin(), children_.end(), match));
        }

        RowData(
            const ui::internal::Row* parent,
            std::shared_ptr<ui::internal::Row> pointer) noexcept
            : parent_(parent)
            , pointer_(std::move(pointer))
            , column_count_(std::nullopt)
            , children_()
        {
        }
        RowData() noexcept
            : RowData(nullptr, std::shared_ptr<ui::internal::Row>{})
        {
        }
        RowData(const RowData&) = delete;
        RowData(RowData&&) = delete;
        auto operator=(const RowData&) -> RowData& = delete;
        auto operator=(RowData&&) -> RowData& = delete;
    };

    static constexpr auto root_row_id_ = RowID{-1};
    static constexpr auto invalid_index_{-1};

    mutable std::mutex parent_lock_;
    mutable std::mutex data_lock_;
    QObject* qt_parent_;
    std::atomic<qt::Model*> parent_;
    RoleData role_data_;
    UnallocatedMap<RowID, RowData> map_;

    auto do_delete_row(const Lock&, const ui::internal::Row* item) noexcept
        -> void
    {
        try {
            const auto id = ID(item);
            auto& child = map_.at(id);
            auto& parent = map_.at(ID(child.parent_));
            parent.Remove(item);
            map_.erase(id);
            LogTrace()(OT_PRETTY_CLASS())("row ")(id)(" deleted").Flush();
        } catch (const std::exception& e) {
            LogError()(OT_PRETTY_CLASS())(e.what())(" in model ")(
                reinterpret_cast<std::uintptr_t>(this))
                .Flush();
        }
    }
    auto do_insert_row(
        const Lock& lock,
        const ui::internal::Row* parent,
        const ui::internal::Row* after,
        std::shared_ptr<ui::internal::Row> row) noexcept -> void
    {
        const auto parentID = ID(parent);
        auto* ptr = row.get();
        const auto id = ID(ptr);

        try {
            if (!row) { throw std::runtime_error{"Invalid row"}; }

            auto& ancestor = map_.at(parentID);
            auto [it, added] = map_.try_emplace(id, parent, std::move(row));

            if (false == added) {
                throw std::runtime_error{"row already exists"};
            }

            OT_ASSERT(1 == map_.count(id));

            it->second.pointer_->InitAfterAdd(lock);
            const auto pos = ancestor.AddAfter(after, ptr);
            LogTrace()(OT_PRETTY_CLASS())("row ")(id)(" with parent ")(
                parentID)(" added to ")(reinterpret_cast<std::uintptr_t>(this))(
                " at position ")(pos)
                .Flush();
        } catch (const std::out_of_range&) {
            LogError()(OT_PRETTY_CLASS())("parent ")(
                parentID)(" does not exist in ")(
                reinterpret_cast<std::uintptr_t>(this))
                .Flush();
        } catch (const std::runtime_error& e) {
            LogError()(OT_PRETTY_CLASS())(e.what())(" in model ")(
                reinterpret_cast<std::uintptr_t>(this))
                .Flush();
        }
    }
    auto do_move_row(
        const Lock&,
        const ui::internal::Row* newParent,
        const ui::internal::Row* newBefore,
        const ui::internal::Row* row) noexcept -> void
    {
        try {
            auto& child = map_.at(ID(row));
            auto& from = map_.at(ID(child.parent_));
            auto& to = map_.at(ID(newParent));
            from.Remove(row);
            to.AddAfter(newBefore, row);
            child.parent_ = newParent;
        } catch (const std::exception& e) {
            LogError()(OT_PRETTY_CLASS())(e.what())(" in model ")(
                reinterpret_cast<std::uintptr_t>(this))
                .Flush();
        }
    }
    auto get_column_count(const Lock& lock, const ui::internal::Row* item)
        const noexcept -> int
    {
        try {
            const auto& child = map_.at(ID(item));

            if (child.column_count_.has_value()) {

                return child.column_count_.value();
            }

            if (child.isRootNode()) {

                throw std::runtime_error{"root column count not set"};
            }

            return get_column_count(lock, child.parent_);
        } catch (const std::exception& e) {
            LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

            return invalid_index_;
        }
    }

    auto get_helper() noexcept -> ModelHelper&
    {
        static thread_local auto map =
            UnallocatedMap<std::uintptr_t, ModelHelper>{};
        auto [it, added] = map.try_emplace(
            reinterpret_cast<std::uintptr_t>(this), parent_.load());

        return it->second;
    }
};

Model::Model(QObject* parent) noexcept
    : imp_(std::make_unique<Imp>(parent).release())
{
    static const auto wrapperType = qRegisterMetaType<RowWrapper>();
    static const auto pointerType = qRegisterMetaType<ui::internal::Row*>();
    LogInsane()(OT_PRETTY_CLASS())("wrapperType: ")(wrapperType).Flush();
    LogInsane()(OT_PRETTY_CLASS())("pointerType: ")(pointerType).Flush();
}

auto Model::ChangeRow(
    ui::internal::Row* parent,
    ui::internal::Row* row) noexcept -> void
{
    imp_->ChangeRow(parent, row);
}

auto Model::ClearParent() noexcept -> void { imp_->ClearParent(); }

auto Model::DeleteRow(ui::internal::Row* row) noexcept -> void
{
    imp_->DeleteRow(row);
}

auto Model::do_delete_row(ui::internal::Row* row) noexcept -> void
{
    imp_->do_delete_row(row);
}

auto Model::do_insert_row(
    ui::internal::Row* parent,
    ui::internal::Row* after,
    std::shared_ptr<ui::internal::Row> row) noexcept -> void
{
    imp_->do_insert_row(parent, after, row);
}

auto Model::do_move_row(
    ui::internal::Row* newParent,
    ui::internal::Row* newBefore,
    ui::internal::Row* row) noexcept -> void
{
    imp_->do_move_row(newParent, newBefore, row);
}

auto Model::GetChild(ui::internal::Row* parent, int index) const noexcept
    -> ui::internal::Row*
{
    return imp_->GetChild(parent, index);
}

auto Model::GetColumnCount(ui::internal::Row* row) const noexcept -> int
{
    return imp_->GetColumnCount(row);
}

auto Model::GetIndex(ui::internal::Row* row) const noexcept -> Index
{
    return imp_->GetIndex(row);
}

auto Model::GetID(const ui::internal::Row* ptr) noexcept -> std::ptrdiff_t
{
    return Imp::ID(ptr);
}

auto Model::GetParent(ui::internal::Row* row) const noexcept -> Index
{
    return imp_->GetParent(row);
}

auto Model::GetRoot() const noexcept -> QObject* { return imp_->GetRoot(); }

auto Model::GetRoleData() const noexcept -> RoleData
{
    return imp_->GetRoleData();
}

auto Model::GetRowCount(ui::internal::Row* row) const noexcept -> int
{
    return imp_->GetRowCount(row);
}

auto Model::InsertRow(
    ui::internal::Row* parent,
    ui::internal::Row* after,
    std::shared_ptr<ui::internal::Row> row) noexcept -> void
{
    imp_->InsertRow(parent, after, row);
}

auto Model::MoveRow(
    ui::internal::Row* newParent,
    ui::internal::Row* newBefore,
    ui::internal::Row* row) noexcept -> void
{
    imp_->MoveRow(newParent, newBefore, row);
}

auto Model::SetColumnCount(Row* parent, int count) noexcept -> void
{
    imp_->SetColumnCount(parent, count);
}

auto Model::SetColumnCount(const Lock& lock, Row* parent, int count) noexcept
    -> void
{
    imp_->SetColumnCount(lock, parent, count);
}

auto Model::SetParent(qt::Model& parent) noexcept -> void
{
    imp_->SetParent(parent);
}

auto Model::SetRoleData(RoleData&& data) noexcept -> void
{
    imp_->SetRoleData(std::move(data));
}

Model::~Model()
{
    if (nullptr != imp_) {
        delete imp_;
        imp_ = nullptr;
    }
}
}  // namespace opentxs::ui::qt::internal

namespace opentxs::ui::qt
{
RowWrapper::RowWrapper(std::shared_ptr<opentxs::ui::internal::Row> row) noexcept
    : row_(std::move(row))
{
}

RowWrapper::RowWrapper() noexcept
    : RowWrapper(std::shared_ptr<opentxs::ui::internal::Row>{})
{
}

RowWrapper::RowWrapper(const RowWrapper& rhs) noexcept
    : RowWrapper(rhs.row_)
{
}

auto RowWrapper::operator=(const RowWrapper& rhs) noexcept -> RowWrapper&
{
    if (&rhs != this) { row_ = rhs.row_; }

    return *this;
}

RowWrapper::~RowWrapper() = default;
}  // namespace opentxs::ui::qt

namespace opentxs::ui::qt
{
ModelHelper::ModelHelper(Model* model) noexcept
{
    claim_ownership(this);

    OT_ASSERT(nullptr != model);

    connect(
        this,
        &ModelHelper::changeRow,
        model,
        &Model::changeRow,
        Qt::QueuedConnection);
    connect(
        this,
        &ModelHelper::deleteRow,
        model,
        &Model::deleteRow,
        Qt::QueuedConnection);
    connect(
        this,
        &ModelHelper::insertRow,
        model,
        &Model::insertRow,
        Qt::QueuedConnection);
    connect(
        this,
        &ModelHelper::moveRow,
        model,
        &Model::moveRow,
        Qt::QueuedConnection);
}

auto ModelHelper::requestChangeRow(
    ui::internal::Row* parent,
    ui::internal::Row* row) noexcept -> void
{
    emit changeRow(parent, row);
}

auto ModelHelper::requestDeleteRow(ui::internal::Row* row) noexcept -> void
{
    emit deleteRow(row);
}

auto ModelHelper::requestInsertRow(
    ui::internal::Row* parent,
    ui::internal::Row* after,
    std::shared_ptr<ui::internal::Row> row) noexcept -> void
{
    emit insertRow(parent, after, std::move(row));
}

auto ModelHelper::requestMoveRow(
    ui::internal::Row* newParent,
    ui::internal::Row* newBefore,
    ui::internal::Row* row) noexcept -> void
{
    emit moveRow(newParent, newBefore, row);
}

ModelHelper::~ModelHelper() { disconnect(); }
}  // namespace opentxs::ui::qt

namespace opentxs::ui::qt
{
Model::Model(internal::Model* internal) noexcept
    : QAbstractItemModel(nullptr)
    , internal_(internal)
{
    if (nullptr != internal_) {
        internal_->SetParent(*this);

        if (auto* root = internal_->GetRoot(); nullptr != root) {
            moveToThread(root->thread());
        }
    }

    claim_ownership(this);
}

auto Model::columnCount(const QModelIndex& parent) const noexcept -> int
{
    if (nullptr != internal_) {

        return internal_->GetColumnCount(
            static_cast<ui::internal::Row*>(parent.internalPointer()));
    }

    return -1;
}

auto Model::data(const QModelIndex& index, int role) const noexcept -> QVariant
{
    auto output = QVariant{};

    if (index.isValid()) {
        auto* row = static_cast<ui::internal::Row*>(index.internalPointer());

        OT_ASSERT(nullptr != row);

        row->qt_data(index.column(), role, output);
    }

    return output;
}

auto Model::changeRow(
    ui::internal::Row* parent,
    ui::internal::Row* item) noexcept -> void
{
    if (nullptr != internal_) {
        const auto topLeft = make_index(internal_->GetIndex(item));
        const auto bottomRight = createIndex(
            topLeft.row(),
            internal_->GetColumnCount(item) - 1,
            topLeft.internalPointer());
        emit dataChanged(topLeft, bottomRight, {});
    }
}

auto Model::deleteRow(ui::internal::Row* item) noexcept -> void
{
    if (nullptr != internal_) {
        const auto parent = make_index(internal_->GetParent(item));
        const auto start = internal_->GetIndex(item).row_;
        beginRemoveRows(parent, start, start);
        internal_->do_delete_row(item);
        endRemoveRows();
    }
}

auto Model::insertRow(
    ui::internal::Row* parent,
    ui::internal::Row* after,
    RowWrapper wrapper) noexcept -> void
{
    auto& row = wrapper.row_;

    if (nullptr != internal_) {
        const auto ancestor = make_index(internal_->GetIndex(parent));
        const auto pos = [&] {
            if (nullptr == after) { return 0; }

            return internal_->GetIndex(after).row_ + 1;
        }();
        beginInsertRows(ancestor, pos, pos);
        internal_->do_insert_row(parent, after, row);
        endInsertRows();
    }
}

auto Model::moveRow(
    ui::internal::Row* newParent,
    ui::internal::Row* newBefore,
    ui::internal::Row* item) noexcept -> void
{
    if (nullptr != internal_) {
        const auto from = make_index(internal_->GetParent(item));
        const auto to = make_index(internal_->GetIndex(newParent));
        const auto start = internal_->GetIndex(item).row_;
        const auto end = [&] {
            if (nullptr == newBefore) { return 0; }

            return internal_->GetIndex(newBefore).row_ + 1;
        }();

        if (beginMoveRows(from, start, start, to, end)) {
            internal_->do_move_row(newParent, newBefore, item);
            endMoveRows();
        } else {
            OT_FAIL;
        }
    }
}

auto Model::hasChildren(const QModelIndex& parent) const noexcept -> bool
{
    if (nullptr != internal_) {

        return 0 < internal_->GetRowCount(static_cast<ui::internal::Row*>(
                       parent.internalPointer()));
    }

    return false;
}

auto Model::headerData(int section, Qt::Orientation orientation, int role)
    const noexcept -> QVariant
{
    return {};
}

auto Model::index(int row, int column, const QModelIndex& ancestor)
    const noexcept -> QModelIndex
{
    if ((0 > row) || (0 > column)) { return {}; }

    if (nullptr != internal_) {
        auto* parent =
            static_cast<ui::internal::Row*>(ancestor.internalPointer());
        auto* child = internal_->GetChild(parent, row);

        if (nullptr != child) {
            if (internal_->GetColumnCount(child) > column) {

                return createIndex(row, column, child);
            }
        }
    }

    return {};
}

auto Model::make_index(const internal::Index& in) const noexcept -> QModelIndex
{
    if (false == in.valid_) {

        return {};
    } else {

        return createIndex(in.row_, in.column_, in.ptr_);
    }
}

auto Model::parent(const QModelIndex& index) const noexcept -> QModelIndex
{
    if (nullptr != internal_) {

        return make_index(internal_->GetParent(
            static_cast<ui::internal::Row*>(index.internalPointer())));
    }

    return {};
}

auto Model::roleNames() const noexcept -> QHash<int, QByteArray>
{
    auto out = QHash<int, QByteArray>{};

    if (nullptr != internal_) {
        for (const auto& [key, value] : internal_->GetRoleData()) {
            out.insert(key, QString{value.c_str()}.toUtf8());
        }
    }

    return out;
}

auto Model::rowCount(const QModelIndex& parent) const noexcept -> int
{
    if (nullptr != internal_) {

        return internal_->GetRowCount(
            static_cast<ui::internal::Row*>(parent.internalPointer()));
    }

    return -1;
}

Model::~Model()
{
    setParent(nullptr);

    if (nullptr != internal_) {
        internal_->ClearParent();
        internal_ = nullptr;
    }
}
}  // namespace opentxs::ui::qt

namespace opentxs::ui::internal
{
auto List::MakeQT(const api::Session& api) noexcept -> ui::qt::internal::Model*
{
    return std::make_unique<ui::qt::internal::Model>(api.QtRootObject())
        .release();
}
}  // namespace opentxs::ui::internal
