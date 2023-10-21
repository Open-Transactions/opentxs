// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "blockchain/node/wallet/subchain/statemachine/Elements.hpp"  // IWYU pragma: associated

#include "opentxs/blockchain/block/Outpoint.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/protocol/bitcoin/base/block/Output.hpp"  // IWYU pragma: keep

namespace opentxs::blockchain::node::wallet
{
Elements::Elements(allocator_type alloc) noexcept
    : elements_20_(alloc)
    , elements_32_(alloc)
    , elements_33_(alloc)
    , elements_64_(alloc)
    , elements_65_(alloc)
    , txos_(alloc)
{
}

Elements::Elements(const Elements& rhs, allocator_type alloc) noexcept
    : elements_20_(rhs.elements_20_, alloc)
    , elements_32_(rhs.elements_32_, alloc)
    , elements_33_(rhs.elements_33_, alloc)
    , elements_64_(rhs.elements_64_, alloc)
    , elements_65_(rhs.elements_65_, alloc)
    , txos_(rhs.txos_, alloc)
{
}

Elements::Elements(Elements&& rhs, allocator_type alloc) noexcept
    : elements_20_(std::move(rhs.elements_20_), alloc)
    , elements_32_(std::move(rhs.elements_32_), alloc)
    , elements_33_(std::move(rhs.elements_33_), alloc)
    , elements_64_(std::move(rhs.elements_64_), alloc)
    , elements_65_(std::move(rhs.elements_65_), alloc)
    , txos_(std::move(rhs.txos_), alloc)
{
}

Elements::Elements(Elements&& rhs) noexcept
    : Elements(std::move(rhs), rhs.get_allocator())
{
}

auto Elements::get_allocator() const noexcept -> allocator_type
{
    return elements_20_.get_allocator();
}

auto Elements::operator=(const Elements& rhs) noexcept -> Elements&
{
    elements_20_ = rhs.elements_20_;
    elements_32_ = rhs.elements_32_;
    elements_33_ = rhs.elements_33_;
    elements_64_ = rhs.elements_64_;
    elements_65_ = rhs.elements_65_;
    txos_ = rhs.txos_;

    return *this;
}

auto Elements::operator=(Elements&& rhs) noexcept -> Elements&
{
    elements_20_ = std::move(rhs.elements_20_);
    elements_32_ = std::move(rhs.elements_32_);
    elements_33_ = std::move(rhs.elements_33_);
    elements_64_ = std::move(rhs.elements_64_);
    elements_65_ = std::move(rhs.elements_65_);
    txos_ = std::move(rhs.txos_);

    return *this;
}

auto Elements::size() const noexcept -> std::size_t
{
    return elements_20_.size() + elements_32_.size() + elements_33_.size() +
           elements_64_.size() + elements_65_.size() + txos_.size();
}
}  // namespace opentxs::blockchain::node::wallet
