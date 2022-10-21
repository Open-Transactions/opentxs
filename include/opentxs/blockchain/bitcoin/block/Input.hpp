// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "opentxs/Export.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace blockchain
{
namespace bitcoin
{
namespace block
{
namespace internal
{
class Input;
}  // namespace internal

class Script;
}  // namespace block
}  // namespace bitcoin

namespace block
{
class Outpoint;
}  // namespace block
}  // namespace blockchain

namespace proto
{
class BlockchainTransactionInput;
}  // namespace proto
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::bitcoin::block
{
class OPENTXS_EXPORT Input
{
public:
    virtual auto Coinbase() const noexcept -> Space = 0;
    OPENTXS_NO_EXPORT virtual auto Internal() const noexcept
        -> const internal::Input& = 0;
    virtual auto Keys() const noexcept -> UnallocatedVector<crypto::Key> = 0;
    virtual auto PreviousOutput() const noexcept
        -> const blockchain::block::Outpoint& = 0;
    virtual auto Print() const noexcept -> UnallocatedCString = 0;
    virtual auto Script() const noexcept -> const block::Script& = 0;
    virtual auto Sequence() const noexcept -> std::uint32_t = 0;
    virtual auto Witness() const noexcept
        -> const UnallocatedVector<Space>& = 0;

    OPENTXS_NO_EXPORT virtual auto Internal() noexcept -> internal::Input& = 0;

    Input(const Input&) = delete;
    Input(Input&&) = delete;
    auto operator=(const Input&) -> Input& = delete;
    auto operator=(Input&&) -> Input& = delete;

    virtual ~Input() = default;

protected:
    Input() noexcept = default;
};
}  // namespace opentxs::blockchain::bitcoin::block
