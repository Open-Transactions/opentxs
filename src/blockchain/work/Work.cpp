// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/blockchain/Work.hpp"  // IWYU pragma: associated

#include <boost/multiprecision/cpp_int.hpp>
#include <span>
#include <utility>

#include "blockchain/work/WorkPrivate.hpp"
#include "internal/blockchain/Params.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/PMR.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/NumericHash.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/core/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::blockchain
{
auto swap(Work& lhs, Work& rhs) noexcept -> void { lhs.swap(rhs); }
}  // namespace opentxs::blockchain

namespace opentxs::blockchain
{
Work::Work(WorkPrivate* imp) noexcept
    : imp_(imp)
{
    OT_ASSERT(nullptr != imp_);
}

Work::Work(allocator_type alloc) noexcept
    : Work(pmr::default_construct<WorkPrivate>(alloc))
{
}

Work::Work(
    const blockchain::block::NumericHash& target,
    blockchain::Type chain,
    allocator_type alloc) noexcept
    : Work([&] {
        using TargetType = boost::multiprecision::checked_cpp_int;
        using ValueType = WorkPrivate::Type;

        try {
            const auto maxTarget =
                block::NumericHash{params::get(chain).Difficulty()};
            const auto max = TargetType{maxTarget.Decimal()};
            const auto incoming = TargetType{target.Decimal()};
            auto value = ValueType{};

            if (incoming > max) {
                value = ValueType{1};
            } else {
                value = ValueType{max} / ValueType{incoming};
            }

            return pmr::construct<WorkPrivate>(alloc, std::move(value));
        } catch (...) {
            LogError()(OT_PRETTY_CLASS())("failed to calculate difficulty for")(
                print(chain))
                .Flush();

            return pmr::default_construct<WorkPrivate>(alloc);
        }
    }())
{
}

Work::Work(const HexType&, std::string_view hex, allocator_type alloc) noexcept
    : Work([&] {
        using TargetType = boost::multiprecision::checked_cpp_int;
        using ValueType = WorkPrivate::Type;

        try {
            if (hex.empty()) {

                return pmr::default_construct<WorkPrivate>(alloc);
            }

            const auto bytes = ByteArray{IsHex, hex};
            auto i = TargetType{};
            auto d = bytes.get();
            boost::multiprecision::import_bits(i, d.begin(), d.end(), 8, true);
            auto value = ValueType{i};

            return pmr::construct<WorkPrivate>(alloc, std::move(value));
        } catch (...) {
            LogError()(OT_PRETTY_CLASS())("failed to decode work").Flush();

            return pmr::default_construct<WorkPrivate>(alloc);
        }
    }())
{
}

Work::Work(const Work& rhs, allocator_type alloc) noexcept
    : Work(rhs.imp_->clone(alloc))
{
}

Work::Work(Work&& rhs) noexcept
    : Work(std::exchange(rhs.imp_, nullptr))
{
}

Work::Work(Work&& rhs, allocator_type alloc) noexcept
    : imp_(nullptr)
{
    pmr::move_construct(imp_, rhs.imp_, alloc);
}

auto Work::asHex(allocator_type alloc) const noexcept -> CString
{
    return imp_->asHex(alloc);
}

auto Work::asHex() const noexcept -> UnallocatedCString
{
    return imp_->asHex({}).c_str();
}

auto Work::Decimal(allocator_type alloc) const noexcept -> CString
{
    return imp_->Decimal(alloc);
}

auto Work::Decimal() const noexcept -> UnallocatedCString
{
    return imp_->Decimal({}).c_str();
}

auto Work::get_allocator() const noexcept -> allocator_type
{
    return imp_->get_allocator();
}

auto Work::get_deleter() noexcept -> delete_function
{
    return pmr::make_deleter(this);
}

auto Work::IsNull() const noexcept -> bool { return imp_->IsNull(); }

auto Work::operator=(const Work& rhs) noexcept -> Work&
{
    return pmr::copy_assign_base(this, imp_, rhs.imp_);
}

auto Work::operator=(Work&& rhs) noexcept -> Work&
{
    return pmr::move_assign_base(*this, rhs, imp_, rhs.imp_);
}

auto Work::operator<=>(const blockchain::Work& rhs) const noexcept
    -> std::strong_ordering
{
    return imp_->operator<=>(*rhs.imp_);
}

auto Work::operator==(const blockchain::Work& rhs) const noexcept -> bool
{
    return imp_->operator==(*rhs.imp_);
}

auto Work::operator+(const Work& rhs) const noexcept -> Work
{
    return pmr::construct<WorkPrivate>(
        get_allocator(), imp_->operator+(*rhs.imp_));
}

auto Work::swap(Work& rhs) noexcept -> void { pmr::swap(imp_, rhs.imp_); }

Work::~Work() { pmr::destroy(imp_); }
}  // namespace opentxs::blockchain
