// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/blockchain/Work.hpp"  // IWYU pragma: associated

#include <boost/multiprecision/cpp_int.hpp>
#include <utility>

#include "blockchain/work/WorkPrivate.hpp"
#include "internal/blockchain/Params.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "internal/util/PMR.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/NumericHash.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/core/Types.hpp"
#include "opentxs/util/Allocator.hpp"
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
    : Work([&] {
        // TODO c++20 alloc.new_object<WorkPrivate>()
        auto pmr = alloc::PMR<WorkPrivate>{alloc};
        auto* out = pmr.allocate(1_uz);
        pmr.construct(out);

        return out;
    }())
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

            // TODO c++20 alloc.new_object<WorkPrivate>(out, std::move(value))
            auto pmr = alloc::PMR<WorkPrivate>{alloc};
            auto* out = pmr.allocate(1_uz);
            pmr.construct(out, std::move(value));

            return out;
        } catch (...) {
            LogError()(OT_PRETTY_CLASS())("failed to calculate difficulty for")(
                print(chain))
                .Flush();
            // TODO c++20 alloc.new_object<WorkPrivate>()
            auto pmr = alloc::PMR<WorkPrivate>{alloc};
            auto* out = pmr.allocate(1_uz);
            pmr.construct(out);

            return out;
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
                // TODO c++20 alloc.new_object<WorkPrivate>()
                auto pmr = alloc::PMR<WorkPrivate>{alloc};
                auto* out = pmr.allocate(1_uz);
                pmr.construct(out);

                return out;
            }

            const auto bytes = ByteArray{IsHex, hex};
            auto i = TargetType{};
            boost::multiprecision::import_bits(
                i, bytes.begin(), bytes.end(), 8, true);
            auto value = ValueType{i};
            // TODO c++20 alloc.new_object<WorkPrivate>(out, std::move(value))
            auto pmr = alloc::PMR<WorkPrivate>{alloc};
            auto* out = pmr.allocate(1_uz);
            pmr.construct(out, std::move(value));

            return out;
        } catch (...) {
            LogError()(OT_PRETTY_CLASS())("failed to decode work").Flush();
            // TODO c++20 alloc.new_object<WorkPrivate>()
            auto pmr = alloc::PMR<WorkPrivate>{alloc};
            auto* out = pmr.allocate(1_uz);
            pmr.construct(out);

            return out;
        }
    }())
{
}

Work::Work(const Work& rhs, allocator_type alloc) noexcept
    : Work([&] {
        // TODO c++20 alloc.new_object<WorkPrivate>(*rhs.imp_)
        auto pmr = alloc::PMR<WorkPrivate>{alloc};
        auto* out = pmr.allocate(1_uz);
        pmr.construct(out, *rhs.imp_);

        return out;
    }())
{
}

Work::Work(Work&& rhs) noexcept
    : Work(rhs.imp_)
{
    rhs.imp_ = nullptr;
}

Work::Work(Work&& rhs, allocator_type alloc) noexcept
    : imp_(nullptr)
{
    if (rhs.get_allocator() == alloc) {
        swap(rhs);
    } else {
        operator=(rhs);
    }

    OT_ASSERT(nullptr != imp_);
}

auto Work::asHex(allocator_type alloc) const noexcept -> CString
{
    return imp_->asHex(alloc);
}

auto Work::asHex() const noexcept -> UnallocatedCString
{
    return imp_->asHex({}).c_str();
}

auto Work::cleanup(WorkPrivate* imp) noexcept -> void
{
    if (nullptr != imp) {
        // TODO c++20 get_allocator().delete_object<WorkPrivate>(imp)
        auto pmr = alloc::PMR<WorkPrivate>{get_allocator()};
        pmr.destroy(imp);
        pmr.deallocate(imp, 1_uz);
    }
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
    return make_deleter(this);
}

auto Work::IsNull() const noexcept -> bool { return imp_->IsNull(); }

auto Work::operator=(const Work& rhs) noexcept -> Work&
{
    auto* old{imp_};
    // TODO c++20 get_allocator().new_object<WorkPrivate>(*rhs.imp_)
    auto pmr = alloc::PMR<WorkPrivate>{get_allocator()};
    imp_ = pmr.allocate(1_uz);
    pmr.construct(imp_, *rhs.imp_);
    cleanup(old);

    return *this;
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
    // TODO c++20 get_allocator().new_object<WorkPrivate>(...)
    auto pmr = alloc::PMR<WorkPrivate>{get_allocator()};
    auto* out = pmr.allocate(1_uz);
    pmr.construct(out, imp_->operator+(*rhs.imp_));

    return out;
}

auto Work::swap(Work& rhs) noexcept -> void
{
    pmr_swap(*this, rhs, imp_, rhs.imp_);
}

auto Work::operator=(Work&& rhs) noexcept -> Work&
{
    if (get_allocator() == rhs.get_allocator()) {
        swap(rhs);

        return *this;
    } else {

        return operator=(rhs);
    }
}

Work::~Work()
{
    cleanup(imp_);
    imp_ = nullptr;
}
}  // namespace opentxs::blockchain
