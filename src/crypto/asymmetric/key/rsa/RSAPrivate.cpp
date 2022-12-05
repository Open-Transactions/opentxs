// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "crypto/asymmetric/key/rsa/RSAPrivate.hpp"  // IWYU pragma: associated

#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/util/Allocator.hpp"

namespace opentxs::crypto::asymmetric::internal::key
{
auto RSA::Blank() noexcept -> RSA&
{
    static auto blank = RSA{};

    return blank;
}
}  // namespace opentxs::crypto::asymmetric::internal::key

namespace opentxs::crypto::asymmetric::key
{
RSAPrivate::RSAPrivate(allocator_type alloc) noexcept
    : KeyPrivate(alloc)
{
}

RSAPrivate::RSAPrivate(const RSAPrivate& rhs, allocator_type alloc) noexcept
    : KeyPrivate(rhs, alloc)
{
}

auto RSAPrivate::Blank(allocator_type alloc) noexcept -> RSAPrivate*
{
    auto pmr = alloc::PMR<RSAPrivate>{alloc};
    auto* out = pmr.allocate(1_uz);

    OT_ASSERT(nullptr != out);

    pmr.construct(out);

    return out;
}

auto RSAPrivate::clone(allocator_type alloc) const noexcept -> RSAPrivate*
{
    auto pmr = alloc::PMR<RSAPrivate>{alloc};
    auto* out = pmr.allocate(1_uz);

    OT_ASSERT(nullptr != out);

    pmr.construct(out, *this);

    return out;
}

auto RSAPrivate::get_deleter() const noexcept
    -> std::function<void(KeyPrivate*)>
{
    return [alloc = alloc::PMR<RSAPrivate>{get_allocator()}](
               KeyPrivate* in) mutable {
        auto* p = dynamic_cast<RSAPrivate*>(in);

        OT_ASSERT(nullptr != p);

        alloc.destroy(p);
        alloc.deallocate(p, 1_uz);
    };
}

RSAPrivate::~RSAPrivate() = default;
}  // namespace opentxs::crypto::asymmetric::key
