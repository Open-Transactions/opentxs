// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/network/otdht/Data.hpp"  // IWYU pragma: associated

#include <opentxs/protobuf/P2PBlockchainSync.pb.h>
#include <memory>
#include <stdexcept>
#include <utility>

#include "internal/network/otdht/Factory.hpp"
#include "network/otdht/messages/Base.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/Session.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/blockchain/block/Header.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/network/otdht/Block.hpp"
#include "opentxs/network/otdht/MessageType.hpp"  // IWYU pragma: keep
#include "opentxs/network/otdht/State.hpp"
#include "opentxs/protobuf/Types.internal.tpp"
#include "opentxs/protobuf/syntax/P2PBlockchainSync.hpp"  // IWYU pragma: keep
#include "opentxs/protobuf/syntax/Types.internal.tpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::factory
{
auto BlockchainSyncData() noexcept -> network::otdht::Data
{
    using ReturnType = network::otdht::Data;

    return {std::make_unique<ReturnType::Imp>().release()};
}

auto BlockchainSyncData(
    WorkType type,
    network::otdht::State state,
    network::otdht::SyncData blocks,
    ReadView cfheader) noexcept -> network::otdht::Data
{
    using ReturnType = network::otdht::Data;

    return {std::make_unique<ReturnType::Imp>(
                type, std::move(state), std::move(blocks), cfheader)
                .release()};
}

auto BlockchainSyncData_p(
    WorkType type,
    network::otdht::State state,
    network::otdht::SyncData blocks,
    ReadView cfheader) noexcept -> std::unique_ptr<network::otdht::Data>
{
    using ReturnType = network::otdht::Data;

    return std::make_unique<ReturnType>(
        std::make_unique<ReturnType::Imp>(
            type, std::move(state), std::move(blocks), cfheader)
            .release());
}
}  // namespace opentxs::factory

namespace opentxs::network::otdht
{
class Data::Imp final : public Base::Imp
{
public:
    Data* parent_;

    auto asData() const noexcept -> const Data& final
    {
        if (nullptr != parent_) {

            return *parent_;
        } else {

            return Base::Imp::asData();
        }
    }

    Imp() noexcept
        : Base::Imp()
        , parent_(nullptr)
    {
    }
    Imp(WorkType type,
        otdht::State state,
        SyncData blocks,
        ReadView cfheader) noexcept(false)
        : Base::Imp(
              Imp::default_version_,
              translate(type),
              [&] {
                  auto out = StateData{};
                  out.emplace_back(std::move(state));

                  return out;
              }(),
              UnallocatedCString{cfheader},
              std::move(blocks))
        , parent_(nullptr)
    {
        switch (type_) {
            case MessageType::sync_reply: {
            } break;
            case MessageType::new_block_header: {
                if (0 == blocks_.size()) {
                    throw std::runtime_error{"Missing blocks"};
                }
            } break;
            default: {

                throw std::runtime_error{"Incorrect type"};
            }
        }
    }
    Imp(const Imp&) = delete;
    Imp(Imp&&) = delete;
    auto operator=(const Imp&) -> Imp& = delete;
    auto operator=(Imp&&) -> Imp& = delete;
};

Data::Data(Imp* imp) noexcept
    : Base(imp)
    , imp_(imp)
{
    imp_->parent_ = this;
}

auto Data::Add(ReadView data) noexcept -> bool
{
    const auto proto = protobuf::Factory<protobuf::P2PBlockchainSync>(data);

    if (false == protobuf::syntax::check(LogError(), proto)) { return false; }

    auto& blocks = const_cast<SyncData&>(imp_->blocks_);

    if (0 < blocks.size()) {
        const auto expected = blocks.back().Height() + 1;
        const auto height =
            static_cast<opentxs::blockchain::block::Height>(proto.height());

        if (height != expected) {
            LogError()()("Non-contiguous sync data").Flush();

            return false;
        }
    }

    try {
        blocks.emplace_back(proto);

        return true;
    } catch (const std::exception& e) {
        LogError()()(e.what()).Flush();

        return false;
    }
}

auto Data::Blocks() const noexcept -> const SyncData& { return imp_->blocks_; }

auto Data::FirstPosition(const api::Session& api) const noexcept
    -> opentxs::blockchain::block::Position
{
    const auto& blocks = imp_->blocks_;

    if (blocks.empty()) { return {}; }

    const auto& first = blocks.front();
    const auto header = api.Factory().BlockHeaderFromNative(
        first.Chain(), first.Header(), {});  // TODO allocator

    if (false == header.IsValid()) { return {}; }

    return {first.Height(), header.Hash()};
}

auto Data::LastPosition(const api::Session& api) const noexcept
    -> opentxs::blockchain::block::Position
{
    const auto& blocks = imp_->blocks_;

    if (blocks.empty()) { return {}; }

    const auto& last = blocks.back();
    const auto header = api.Factory().BlockHeaderFromNative(
        last.Chain(), last.Header(), {});  // TODO allocator

    if (false == header.IsValid()) { return {}; }

    return {last.Height(), header.Hash()};
}

auto Data::PreviousCfheader() const noexcept -> ReadView
{
    return imp_->endpoint_;
}

auto Data::State() const noexcept -> const otdht::State&
{
    assert_true(0 < imp_->state_.size());

    return imp_->state_.front();
}

Data::~Data()
{
    if (nullptr != Data::imp_) {
        delete Data::imp_;
        Data::imp_ = nullptr;
        Base::imp_ = nullptr;
    }
}
}  // namespace opentxs::network::otdht
