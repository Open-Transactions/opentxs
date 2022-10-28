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

#include "internal/blockchain/block/Header.hpp"
#include "ottest/Basic.hpp"

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
// NOLINTEND(modernize-concat-nested-namespaces)

namespace b = ot::blockchain;
namespace bb = b::block;
namespace bc = b::node;

#define BLOCK_1 "block 01_XXXXXXXXXXXXXXXXXXXXXXX"
#define BLOCK_2 "block 02_XXXXXXXXXXXXXXXXXXXXXXX"
#define BLOCK_3 "block 03_XXXXXXXXXXXXXXXXXXXXXXX"
#define BLOCK_4 "block 04_XXXXXXXXXXXXXXXXXXXXXXX"
#define BLOCK_5 "block 05_XXXXXXXXXXXXXXXXXXXXXXX"
#define BLOCK_6 "block 06_XXXXXXXXXXXXXXXXXXXXXXX"
#define BLOCK_7 "block 07_XXXXXXXXXXXXXXXXXXXXXXX"
#define BLOCK_8 "block 08_XXXXXXXXXXXXXXXXXXXXXXX"
#define BLOCK_9 "block 09_XXXXXXXXXXXXXXXXXXXXXXX"
#define BLOCK_10 "block 10_XXXXXXXXXXXXXXXXXXXXXXX"
#define BLOCK_11 "block 11_XXXXXXXXXXXXXXXXXXXXXXX"
#define BLOCK_12 "block 12_XXXXXXXXXXXXXXXXXXXXXXX"
#define BLOCK_13 "block 13_XXXXXXXXXXXXXXXXXXXXXXX"
#define BLOCK_14 "block 14_XXXXXXXXXXXXXXXXXXXXXXX"

namespace ottest
{
class OPENTXS_EXPORT Test_HeaderOracle_base : public ::testing::Test
{
public:
    using Block = std::pair<ot::UnallocatedCString, ot::UnallocatedCString>;
    using Position = std::pair<bb::Height, ot::UnallocatedCString>;
    using BestChainVector = ot::UnallocatedVector<ot::UnallocatedCString>;
    using Test = std::tuple<ot::UnallocatedCString, Position, BestChainVector>;
    using Status = bb::internal::Header::Status;
    using HeaderData = std::tuple<
        ot::UnallocatedCString,
        ot::UnallocatedCString,
        bb::Height,
        Status,
        Status>;
    using PostStateVector = ot::UnallocatedVector<HeaderData>;
    using ExpectedSiblings = ot::UnallocatedSet<ot::UnallocatedCString>;

    static const ot::UnallocatedVector<Block> create_1_;
    static const ot::UnallocatedVector<Test> sequence_1_;
    static const PostStateVector post_state_1_;
    static const BestChainVector best_chain_1_;
    static const ExpectedSiblings siblings_1_;
    static const ot::UnallocatedVector<Block> create_2_;
    static const ot::UnallocatedVector<Test> sequence_2_;
    static const PostStateVector post_state_2_;
    static const BestChainVector best_chain_2_;
    static const ExpectedSiblings siblings_2_;
    static const ot::UnallocatedVector<Block> create_3_;
    static const ot::UnallocatedVector<Test> sequence_3_;
    static const PostStateVector post_state_3_;
    static const BestChainVector best_chain_3_;
    static const ExpectedSiblings siblings_3_;
    static const ot::UnallocatedVector<Block> create_4_;
    static const ot::UnallocatedVector<Test> sequence_4_;
    static const PostStateVector post_state_4_;
    static const BestChainVector best_chain_4_;
    static const ExpectedSiblings siblings_4_;
    static const ot::UnallocatedVector<Block> create_5_;
    static const ot::UnallocatedVector<Test> sequence_5_;
    static const PostStateVector post_state_5_;
    static const BestChainVector best_chain_5_;
    static const ExpectedSiblings siblings_5_;
    static const ot::UnallocatedVector<Block> create_6_;
    static const ot::UnallocatedVector<Test> sequence_6_;
    static const PostStateVector post_state_6_;
    static const BestChainVector best_chain_6_;
    static const ExpectedSiblings siblings_6_;
    static const ot::UnallocatedVector<Block> create_7_;
    static const ot::UnallocatedVector<Test> sequence_7_;
    static const PostStateVector post_state_7_;
    static const BestChainVector best_chain_7_;
    static const ExpectedSiblings siblings_7_;
    static const ot::UnallocatedVector<Block> create_8_;
    static const ot::UnallocatedVector<Test> sequence_8_;
    static const PostStateVector post_state_8a_;
    static const PostStateVector post_state_8b_;
    static const BestChainVector best_chain_8a_;
    static const BestChainVector best_chain_8b_;
    static const ExpectedSiblings siblings_8a_;
    static const ExpectedSiblings siblings_8b_;
    static const ot::UnallocatedVector<Block> create_9_;
    static const ot::UnallocatedVector<Test> sequence_9_;
    static const PostStateVector post_state_9_;
    static const BestChainVector best_chain_9_;
    static const ExpectedSiblings siblings_9_;
    static const ot::UnallocatedVector<Block> create_10_;
    static const ot::UnallocatedVector<Test> sequence_10_;
    static const ot::UnallocatedVector<ot::UnallocatedCString> bitcoin_;

    const ot::api::session::Client& api_;
    const b::Type type_;
    ot::api::network::BlockchainHandle handle_;
    const bc::Manager& network_;
    bc::HeaderOracle& header_oracle_;
    ot::UnallocatedMap<ot::UnallocatedCString, std::unique_ptr<bb::Header>>
        test_blocks_;

    auto apply_blocks(const ot::UnallocatedVector<Test>& vector) -> bool;
    auto apply_blocks_batch(const ot::UnallocatedVector<Test>& vector) -> bool;
    auto create_blocks(const ot::UnallocatedVector<Block>& vector) -> bool;
    auto get_block_hash(const ot::UnallocatedCString& hash) -> bb::Hash;
    auto get_test_block(const ot::UnallocatedCString& hash)
        -> std::unique_ptr<bb::Header>;
    auto make_position(const bb::Height height, const std::string_view hash)
        -> bb::Position;
    auto make_test_block(
        const ot::UnallocatedCString& hash,
        const bb::Hash& parent) -> bool;
    auto verify_best_chain(const BestChainVector& vector) -> bool;
    auto verify_hash_sequence(
        const bb::Height start,
        const std::size_t stop,
        const ot::UnallocatedVector<ot::UnallocatedCString>& expected) -> bool;
    auto verify_post_state(const PostStateVector& vector) -> bool;
    auto verify_siblings(const ExpectedSiblings& vector) -> bool;

    auto Shutdown() noexcept -> void;

    Test_HeaderOracle_base(const b::Type type);

    ~Test_HeaderOracle_base() override;
};

struct OPENTXS_EXPORT Test_HeaderOracle_btc : public Test_HeaderOracle_base {
    Test_HeaderOracle_btc();

    ~Test_HeaderOracle_btc() override;
};

struct OPENTXS_EXPORT Test_HeaderOracle : public Test_HeaderOracle_base {
    Test_HeaderOracle();

    ~Test_HeaderOracle() override;
};
}  // namespace ottest
