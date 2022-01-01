// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "opentxs/blockchain/Types.hpp"

namespace opentxs
{
namespace blockchain
{
namespace block
{
namespace bitcoin
{
namespace internal
{
struct Input;
}  // namespace internal

class Script;
}  // namespace bitcoin

struct Outpoint;
}  // namespace block
}  // namespace blockchain

namespace proto
{
class BlockchainTransactionInput;
}  // namespace proto
}  // namespace opentxs

namespace opentxs
{
namespace blockchain
{
namespace block
{
namespace bitcoin
{
class OPENTXS_EXPORT Input
{
public:
    virtual auto Coinbase() const noexcept -> Space = 0;
    OPENTXS_NO_EXPORT virtual auto Internal() const noexcept
        -> const internal::Input& = 0;
    virtual auto Keys() const noexcept -> std::pmr::vector<crypto::Key> = 0;
    virtual auto PreviousOutput() const noexcept -> const Outpoint& = 0;
    virtual auto Print() const noexcept -> std::string = 0;
    virtual auto Script() const noexcept -> const bitcoin::Script& = 0;
    virtual auto Sequence() const noexcept -> std::uint32_t = 0;
    virtual auto Witness() const noexcept -> const std::pmr::vector<Space>& = 0;

    OPENTXS_NO_EXPORT virtual auto Internal() noexcept -> internal::Input& = 0;

    virtual ~Input() = default;

protected:
    Input() noexcept = default;

private:
    Input(const Input&) = delete;
    Input(Input&&) = delete;
    auto operator=(const Input&) -> Input& = delete;
    auto operator=(Input&&) -> Input& = delete;
};
}  // namespace bitcoin
}  // namespace block
}  // namespace blockchain
}  // namespace opentxs
