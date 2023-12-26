// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/Types.hpp"

#include "ottest/fixtures/blockchain/BlockListener.hpp"  // IWYU pragma: associated

#include <cs_plain_guarded.h>
#include <opentxs/opentxs.hpp>
#include <span>
#include <utility>

#include "internal/util/P0330.hpp"

namespace ottest
{
using namespace opentxs::literals;
using namespace std::literals;

class BlockListener::Imp
{
public:
    using Message = opentxs::network::zeromq::Message;

    struct Data {
        std::promise<Position> promise_{};
        Height target_{-1};
    };

    const ot::api::Session& api_;
    libguarded::plain_guarded<Data> data_;

    auto GetFuture(const Height height) noexcept -> Future
    {
        auto handle = data_.lock();
        auto& data = *handle;
        data.target_ = height;

        try {
            data.promise_ = {};
        } catch (...) {
        }

        return data.promise_.get_future();
    }
    auto Process(
        const opentxs::WorkType work,
        Message&& msg,
        ot::alloc::Strategy alloc) noexcept
        -> opentxs::network::zeromq::actor::Replies
    {
        using enum opentxs::WorkType;

        switch (work) {
            case BlockchainBlockOracleProgress: {
                process_block(std::move(msg));
            } break;
            default: {
            }
        }

        return opentxs::network::zeromq::actor::Replies{alloc.work_};
    }

    Imp(const ot::api::Session& api) noexcept
        : api_(api)
        , data_()
    {
    }

private:
    auto process_block(Message&& msg) noexcept -> void
    {
        const auto body = msg.Payload();

        opentxs::assert_true(3_uz < body.size());

        using BlockHeight = ot::blockchain::block::Height;
        process_position({body[2].as<BlockHeight>(), body[3].Bytes()});
    }
    auto process_position(ot::blockchain::block::Position&& position) noexcept
        -> void
    {
        auto handle = data_.lock();
        auto& data = *handle;

        if (position.height_ == data.target_) {
            try {
                data.promise_.set_value(position);
            } catch (...) {
            }
        }
    }
};
}  // namespace ottest

namespace ottest
{
BlockListener::BlockListener(
    const ot::api::Session& api,
    std::string_view name) noexcept
    : imp_(std::make_shared<Imp>(api))
{
    using namespace opentxs::network::zeromq;
    using enum socket::Direction;
    api.Network().ZeroMQ().Context().SpawnActor(
        api,
        name,
        DefaultStartup(),
        DefaultShutdown(),
        [me = imp_](auto, auto type, auto&& message, auto alloc) {
            return me->Process(
                static_cast<opentxs::WorkType>(type),
                std::move(message),
                alloc);
        },
        DefaultStateMachine(),
        {
            {api.Endpoints().Shutdown(), Connect},
            {api.Endpoints().BlockchainBlockOracleProgress(), Connect},
        });
}

auto BlockListener::GetFuture(const Height height) noexcept -> Future
{
    return imp_->GetFuture(height);
}

BlockListener::~BlockListener() = default;
}  // namespace ottest
