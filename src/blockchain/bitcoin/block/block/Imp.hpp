// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <functional>
#include <optional>
#include <utility>

#include "blockchain/bitcoin/block/block/BlockPrivate.hpp"
#include "blockchain/block/block/Imp.hpp"
#include "internal/blockchain/bitcoin/block/Types.hpp"
#include "internal/blockchain/block/Types.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/bitcoin/block/Header.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/Types.hpp"
#include "opentxs/network/blockchain/bitcoin/CompactSize.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Session;
}  // namespace api
class Log;
class WriteBuffer;
class Writer;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::bitcoin::block::implementation
{
class Block : public blockchain::block::implementation::Block,
              public BlockPrivate
{
public:
    static constexpr auto header_bytes_ = 80_uz;

    auto CalculateSize() const noexcept -> std::size_t final
    {
        return get_or_calculate_size().first;
    }
    [[nodiscard]] auto clone(allocator_type alloc) const noexcept
        -> bitcoin::block::BlockPrivate* override;
    auto ExtractElements(const cfilter::Type style, alloc::Default alloc)
        const noexcept -> Elements final;
    auto FindMatches(
        const api::Session& api,
        const cfilter::Type type,
        const Patterns& outpoints,
        const Patterns& scripts,
        const Log& log,
        alloc::Default alloc,
        alloc::Default monotonic) const noexcept -> Matches final;
    auto IsValid() const noexcept -> bool final { return true; }
    auto Print() const noexcept -> UnallocatedCString override;
    auto Print(allocator_type alloc) const noexcept -> CString override;
    auto Serialize(Writer&& bytes) const noexcept -> bool final;

    [[nodiscard]] auto get_deleter() noexcept -> std::function<void()> override;

    Block(
        const blockchain::Type chain,
        bitcoin::block::Header header,
        TxidIndex&& ids,
        TxidIndex&& hashes,
        TransactionMap&& transactions,
        std::optional<CalculatedSize> size,
        allocator_type alloc) noexcept(false);
    Block() = delete;
    Block(const Block& rhs, allocator_type alloc) noexcept;
    Block(const Block&) = delete;
    Block(Block&&) = delete;
    auto operator=(const Block&) -> Block& = delete;
    auto operator=(Block&&) -> Block& = delete;

    ~Block() override;

private:
    mutable std::optional<CalculatedSize> size_;

    auto calculate_size() const noexcept -> CalculatedSize;
    auto calculate_size(const network::blockchain::bitcoin::CompactSize& cs)
        const noexcept -> std::size_t;
    virtual auto extra_bytes() const noexcept -> std::size_t { return 0; }
    auto get_or_calculate_size() const noexcept -> CalculatedSize;
    virtual auto serialize_aux_pow(WriteBuffer& out) const noexcept -> bool;
};
}  // namespace opentxs::blockchain::bitcoin::block::implementation
