// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <compare>
#include <cstddef>
#include <string_view>

#include "opentxs/Export.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/util/Allocated.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace blockchain
{
namespace bitcoin
{
namespace block
{
class Header;
}  // namespace block
}  // namespace bitcoin

namespace block
{
namespace internal
{
class Header;
}  // namespace internal

class Hash;
class Header;
class HeaderPrivate;
class NumericHash;
class Position;
}  // namespace block

class Work;
}  // namespace blockchain

class Writer;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace std
{
template <>
struct OPENTXS_EXPORT hash<opentxs::blockchain::block::Header> {
    using is_transparent = void;
    using is_avalanching = void;

    auto operator()(const opentxs::blockchain::block::Header& data)
        const noexcept -> std::size_t;
};
}  // namespace std

namespace opentxs::blockchain::block
{
OPENTXS_EXPORT auto operator==(const Header&, const Header&) noexcept -> bool;
OPENTXS_EXPORT auto operator<=>(const Header&, const Header&) noexcept
    -> std::strong_ordering;
OPENTXS_EXPORT auto swap(Header&, Header&) noexcept -> void;
}  // namespace opentxs::blockchain::block

namespace opentxs::blockchain::block
{
class OPENTXS_EXPORT Header : virtual public opentxs::Allocated
{
public:
    OPENTXS_NO_EXPORT static auto Blank() noexcept -> Header&;

    [[nodiscard]] operator bool() const noexcept { return IsValid(); }

    auto asBitcoin() const& noexcept -> const bitcoin::block::Header&;
    auto Difficulty() const noexcept -> blockchain::Work;
    auto get_allocator() const noexcept -> allocator_type final;
    auto Hash() const noexcept -> const block::Hash&;
    auto Height() const noexcept -> block::Height;
    auto IncrementalWork() const noexcept -> blockchain::Work;
    OPENTXS_NO_EXPORT auto Internal() const noexcept -> const internal::Header&;
    [[nodiscard]] auto IsValid() const noexcept -> bool;
    auto NumericHash() const noexcept -> block::NumericHash;
    auto ParentHash() const noexcept -> const block::Hash&;
    auto ParentWork() const noexcept -> blockchain::Work;
    auto Position() const noexcept -> block::Position;
    auto Print() const noexcept -> UnallocatedCString;
    auto Print(allocator_type alloc) const noexcept -> CString;
    auto Serialize(Writer&& destination, const bool bitcoinformat = true)
        const noexcept -> bool;
    auto Target() const noexcept -> block::NumericHash;
    auto Type() const noexcept -> blockchain::Type;
    auto Valid() const noexcept -> bool;
    auto Work() const noexcept -> blockchain::Work;

    auto asBitcoin() & noexcept -> bitcoin::block::Header&;
    auto asBitcoin() && noexcept -> bitcoin::block::Header;
    OPENTXS_NO_EXPORT auto Internal() noexcept -> internal::Header&;
    auto swap(Header& rhs) noexcept -> void;

    OPENTXS_NO_EXPORT Header(HeaderPrivate* imp) noexcept;
    Header(allocator_type alloc = {}) noexcept;
    Header(const Header& rhs, allocator_type alloc = {}) noexcept;
    Header(Header&& rhs) noexcept;
    Header(Header&& rhs, allocator_type alloc) noexcept;
    auto operator=(const Header& rhs) noexcept -> Header&;
    auto operator=(Header&& rhs) noexcept -> Header&;

    ~Header() override;

protected:
    friend HeaderPrivate;

    HeaderPrivate* imp_;
};
}  // namespace opentxs::blockchain::block
