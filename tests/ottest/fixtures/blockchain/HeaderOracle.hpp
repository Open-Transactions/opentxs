// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <cstddef>
#include <iosfwd>
#include <memory>
#include <string_view>
#include <tuple>
#include <utility>

#include "ottest/Basic.hpp"
#include "ottest/fixtures/blockchain/BlockHeaderListener.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace session
{
class Client;
}  // namespace session
}  // namespace api

namespace blockchain
{
namespace node
{
class HeaderOracle;
class Manager;
}  // namespace node
}  // namespace blockchain
}  // namespace opentxs

namespace ottest
{
struct BlockHeaderSequence;
struct BlockHeaderTag;
struct BlockHeaderTestSequence;
struct BlockchainSnapshot;
}  // namespace ottest
// NOLINTEND(modernize-concat-nested-namespaces)

namespace b = ot::blockchain;
namespace bb = b::block;
namespace bc = b::node;

namespace ottest
{
class OPENTXS_EXPORT HeaderOracle_base : public ::testing::Test
{
public:
    const ot::api::session::Client& api_;
    const b::Type type_;
    ot::api::network::BlockchainHandle handle_;
    const bc::Manager& network_;
    bc::HeaderOracle& header_oracle_;
    ot::UnallocatedMap<ot::UnallocatedCString, std::unique_ptr<bb::Header>>
        test_blocks_;
    BlockHeaderListener blocks_;

    auto AddCheckpoint(const BlockHeaderTag& tag, bb::Height height) noexcept
        -> bool;
    auto ApplyBlockSequence(const BlockHeaderTestSequence& seq) noexcept
        -> bool;
    auto CreateBlocks(const BlockHeaderTestSequence& seq) noexcept -> bool;
    auto DeleteCheckpoint() noexcept -> bool;
    auto GetBlockHash(const BlockHeaderTag& tag) -> bb::Hash;
    auto GetStopHeight(const BlockHeaderTestSequence& seq, std::size_t index)
        const -> bb::Height;
    auto GetTestBlock(const BlockHeaderTag& tag) -> std::unique_ptr<bb::Header>;
    auto MakeTestBlock(const BlockHeaderTag& tag, const bb::Hash& parent)
        -> bool;
    auto TearDown() noexcept -> void final;
    auto VerifyBestChain(
        const BlockHeaderTestSequence& seq,
        std::size_t state) noexcept -> bool;

    HeaderOracle_base(const b::Type type);

    ~HeaderOracle_base() override;

private:
    auto get_stop_height(const BlockHeaderSequence& seq, std::size_t index)
        const -> bb::Height;
    auto get_stop_height(const BlockchainSnapshot& data) const -> bb::Height;
};

struct OPENTXS_EXPORT HeaderOracle_btc : public HeaderOracle_base {
    HeaderOracle_btc();

    ~HeaderOracle_btc() override;
};

struct OPENTXS_EXPORT HeaderOracle_bch : public HeaderOracle_base {
    HeaderOracle_bch();

    ~HeaderOracle_bch() override;
};

struct OPENTXS_EXPORT HeaderOracle : public HeaderOracle_base {
    HeaderOracle();

    ~HeaderOracle() override;
};
}  // namespace ottest
