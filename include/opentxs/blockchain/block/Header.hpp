// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Export.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/util/Bytes.hpp"
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
class NumericHash;
}  // namespace block
}  // namespace bitcoin

namespace block
{
namespace internal
{
class Header;
}  // namespace internal

class Hash;
class NumericHash;
class Position;
}  // namespace block

class Work;
}  // namespace blockchain
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::block
{
class OPENTXS_EXPORT Header
{
public:
    class Imp;

    virtual auto as_Bitcoin() const noexcept
        -> const blockchain::bitcoin::block::Header&;
    auto clone() const noexcept -> std::unique_ptr<Header>;
    auto Difficulty() const noexcept -> blockchain::Work;
    auto Hash() const noexcept -> const block::Hash&;
    auto Height() const noexcept -> block::Height;
    auto IncrementalWork() const noexcept -> blockchain::Work;
    auto Internal() const noexcept -> const internal::Header&;
    auto NumericHash() const noexcept -> block::NumericHash;
    auto ParentHash() const noexcept -> const block::Hash&;
    auto ParentWork() const noexcept -> blockchain::Work;
    auto Position() const noexcept -> block::Position;
    auto Print() const noexcept -> UnallocatedCString;
    auto Serialize(
        const AllocateOutput destination,
        const bool bitcoinformat = true) const noexcept -> bool;
    auto Target() const noexcept -> block::NumericHash;
    auto Type() const noexcept -> blockchain::Type;
    auto Valid() const noexcept -> bool;
    auto Work() const noexcept -> blockchain::Work;

    auto Internal() noexcept -> internal::Header&;

    Header() noexcept;
    OPENTXS_NO_EXPORT Header(Imp*) noexcept;
    Header(const Header&) noexcept;
    Header(Header&&) = delete;
    auto operator=(const Header&) -> Header& = delete;
    auto operator=(Header&&) -> Header& = delete;

    virtual ~Header();

protected:
    Imp* imp_;

    auto swap_header(Header& rhs) noexcept -> void;
};
}  // namespace opentxs::blockchain::block
