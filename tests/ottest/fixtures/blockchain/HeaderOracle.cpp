// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "ottest/fixtures/blockchain/HeaderOracle.hpp"  // IWYU pragma: associated

#include <opentxs/opentxs.hpp>
#include <exception>
#include <string_view>
#include <utility>

#include "internal/api/session/FactoryAPI.hpp"
#include "internal/blockchain/Params.hpp"
#include "internal/blockchain/block/Header.hpp"
#include "internal/blockchain/node/headeroracle/HeaderOracle.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "ottest/data/blockchain/Headers.hpp"

namespace ottest
{
using namespace std::literals;
using namespace opentxs::literals;

HeaderOracle_base::HeaderOracle_base(const b::Type type)
    : api_(ot::Context().StartClientSession(
          ot::Options{}.SetBlockchainProfile(ot::BlockchainProfile::server),
          0))
    , type_(type)
    , handle_([&] {
        api_.Network().Blockchain().Start(type);
        auto handle = api_.Network().Blockchain().GetChain(type);

        OT_ASSERT(handle.IsValid());

        return handle;
    }())
    , network_(handle_.get())
    , header_oracle_(const_cast<bc::HeaderOracle&>(network_.HeaderOracle()))
    , test_blocks_()
    , blocks_(api_, "block listener"sv)
{
    header_oracle_.Internal().DeleteCheckpoint();
}

auto HeaderOracle_base::AddCheckpoint(
    const BlockHeaderTag& tag,
    bb::Height target) noexcept -> bool
{
    const auto blockhash = GetBlockHash(tag);

    if (header_oracle_.Internal().AddCheckpoint(target, blockhash)) {
        const auto [height, hash] = header_oracle_.GetCheckpoint();

        EXPECT_EQ(height, target);
        EXPECT_EQ(hash, blockhash);

        return true;
    } else {

        return false;
    }
}

auto HeaderOracle_base::ApplyBlockSequence(
    const BlockHeaderTestSequence& seq) noexcept -> bool
{
    try {
        const auto& data = BitcoinHeaderTestSequences().at(seq);
        const auto stop = get_stop_height(data, 0_uz);
        auto future = blocks_.GetFuture(stop);
        auto headers = ot::Vector<std::unique_ptr<bb::Header>>{};

        for (const auto& [_, tag] : data.create_) {
            headers.emplace_back(GetTestBlock(tag));
        }

        if (header_oracle_.Internal().AddHeaders(headers)) {

            return future.get().height_ == stop;
        } else {

            return false;
        }
    } catch (const std::exception& e) {
        ADD_FAILURE() << e.what();

        return false;
    }
}

auto HeaderOracle_base::CreateBlocks(
    const BlockHeaderTestSequence& seq) noexcept -> bool
{
    try {
        const auto& data = BitcoinHeaderTestSequences().at(seq);

        for (const auto& [parent, child] : data.create_) {
            const bb::Hash previous{
                parent.empty() ? b::params::get(type_).GenesisHash()
                               : GetBlockHash(parent)};

            if (false == MakeTestBlock(child, previous)) { return false; }
        }

        return true;
    } catch (const std::exception& e) {
        ADD_FAILURE() << e.what();

        return false;
    }
}

auto HeaderOracle_base::DeleteCheckpoint() noexcept -> bool
{
    return header_oracle_.Internal().DeleteCheckpoint();
}

auto HeaderOracle_base::GetBlockHash(const BlockHeaderTag& tag) -> bb::Hash
{
    if (tag.empty()) { return b::params::get(type_).GenesisHash(); }

    if (const auto i = test_blocks_.find(tag.value_); test_blocks_.end() != i) {

        return i->second->Hash();
    } else {

        return {};
    }
}

auto HeaderOracle_base::GetStopHeight(
    const BlockHeaderTestSequence& seq,
    std::size_t index) const -> bb::Height
{
    const auto& data = BitcoinHeaderTestSequences().at(seq);

    return get_stop_height(data, index);
}

auto HeaderOracle_base::get_stop_height(
    const BlockHeaderSequence& seq,
    std::size_t index) const -> bb::Height
{
    const auto& snapshot = seq.state_.at(index);

    return get_stop_height(snapshot);
}

auto HeaderOracle_base::get_stop_height(const BlockchainSnapshot& data) const
    -> bb::Height
{
    const auto& bestBlock = data.best_chain_.back();

    return data.status_.at(bestBlock).height_;
}

auto HeaderOracle_base::GetTestBlock(const BlockHeaderTag& tag)
    -> std::unique_ptr<bb::Header>
{
    if (const auto i = test_blocks_.find(tag.value_); test_blocks_.end() != i) {

        return i->second->clone();
    } else {

        return {};
    }
}

auto HeaderOracle_base::MakeTestBlock(
    const BlockHeaderTag& tag,
    const bb::Hash& parent) -> bool
{
    const auto child = ot::blockchain::block::Hash{tag.value_};
    auto pHeader = api_.Factory().InternalSession().BlockHeaderForUnitTests(
        child, parent, -1);

    if (false == bool(pHeader)) { return false; }

    test_blocks_.emplace(tag.value_, std::move(pHeader));

    return true;
}

auto HeaderOracle_base::TearDown() noexcept -> void
{
    // TODO
}

auto HeaderOracle_base::VerifyBestChain(
    const BlockHeaderTestSequence& seq,
    std::size_t state) noexcept -> bool
{
    try {
        const auto& data = BitcoinHeaderTestSequences().at(seq);
        const auto& snapshot = data.state_.at(state);

        for (const auto& tag : snapshot.best_chain_) {
            const auto hash = GetBlockHash(tag);
            const auto pHeader = header_oracle_.LoadHeader(hash);

            if (false == pHeader.operator bool()) {
                ADD_FAILURE() << "failed to load block header " << hash.asHex();

                return false;
            }

            const auto& header = *pHeader;
            const auto& [parent, height, status, parentStatus] =
                snapshot.status_.at(tag);
            using Status = bb::internal::Header::Status;

            EXPECT_EQ(hash, header.Hash());
            EXPECT_EQ(GetBlockHash(parent), header.ParentHash());
            EXPECT_EQ(height, header.Height());
            EXPECT_EQ(
                static_cast<Status>(status), header.Internal().LocalState());
            EXPECT_EQ(
                static_cast<Status>(parentStatus),
                header.Internal().InheritedState());
        }

        const auto siblings = header_oracle_.Siblings();
        const auto expected = [&, this] {
            auto out = decltype(siblings){};

            for (const auto& tag : snapshot.orphans_) {
                out.emplace(GetBlockHash(tag));
            }

            return out;
        }();

        EXPECT_EQ(siblings, expected);

        return true;
    } catch (const std::exception& e) {
        ADD_FAILURE() << e.what();

        return false;
    }
}

HeaderOracle_base::~HeaderOracle_base() = default;
}  // namespace ottest

namespace ottest
{
HeaderOracle_btc::HeaderOracle_btc()
    : HeaderOracle_base(b::Type::Bitcoin)
{
}

HeaderOracle_btc::~HeaderOracle_btc() = default;
}  // namespace ottest

namespace ottest
{
HeaderOracle_bch::HeaderOracle_bch()
    : HeaderOracle_base(b::Type::Bitcoin)
{
}

HeaderOracle_bch::~HeaderOracle_bch() = default;
}  // namespace ottest

namespace ottest
{
HeaderOracle::HeaderOracle()
    : HeaderOracle_base(b::Type::UnitTest)
{
}

HeaderOracle::~HeaderOracle() = default;
}  // namespace ottest
