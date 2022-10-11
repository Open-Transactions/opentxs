// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "ottest/fixtures/blockchain/ScanListener.hpp"  // IWYU pragma: associated

#include <opentxs/opentxs.hpp>
#include <chrono>
#include <mutex>
#include <string_view>
#include <utility>

#include "internal/network/zeromq/Context.hpp"
#include "internal/network/zeromq/ListenCallback.hpp"
#include "internal/network/zeromq/socket/Subscribe.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/Mutex.hpp"

namespace ottest
{
struct ScanListener::Imp {
    using Chain = ot::blockchain::Type;
    using Position = ot::blockchain::block::Position;
    using Promise = std::promise<void>;

    struct Data {
        Position pos_;
        Height target_;
        Promise promise_;

        auto reset(Height target) noexcept -> Future
        {
            target_ = target;
            promise_ = {};

            return promise_.get_future();
        }
        auto test() noexcept -> void
        {
            if (pos_.height_ == target_) {
                try {
                    promise_.set_value();
                } catch (...) {
                }
            }
        }

        Data(Height height, ot::blockchain::block::Hash&& hash) noexcept
            : pos_(height, std::move(hash))
            , target_()
            , promise_()
        {
        }
        Data() = delete;
        Data(Data&&) = default;
        Data(const Data&) = delete;
        auto operator=(const Data&) -> Data& = delete;
        auto operator=(Data&&) -> Data& = delete;
    };

    using SubchainMap = ot::UnallocatedMap<Subchain, Data>;
    using AccountMap = ot::UnallocatedMap<ot::identifier::Generic, SubchainMap>;
    using ChainMap = ot::UnallocatedMap<Chain, AccountMap>;
    using Map = ot::UnallocatedMap<ot::identifier::Nym, ChainMap>;

    const ot::api::Session& api_;
    const ot::OTZMQListenCallback cb_;
    const ot::OTZMQSubscribeSocket socket_;
    mutable std::mutex lock_;
    Map map_;

    auto cb(ot::network::zeromq::Message&& in) noexcept -> void
    {
        const auto body = in.Body();

        OT_ASSERT(body.size() == 8u);

        const auto chain = body.at(1).as<Chain>();
        auto nymID = [&] {
            auto out = ot::identifier::Nym{};
            out.Assign(body.at(2).Bytes());

            OT_ASSERT(false == out.empty());

            return out;
        }();
        auto accountID = [&] {
            auto out = ot::identifier::Generic{};
            out.Assign(body.at(4).Bytes());

            OT_ASSERT(false == out.empty());

            return out;
        }();
        const auto sub = body.at(5).as<Subchain>();
        const auto height = body.at(6).as<Height>();
        auto hash = ot::blockchain::block::Hash{body.at(7).Bytes()};
        auto lock = ot::Lock{lock_};
        auto& map = map_[std::move(nymID)][chain][std::move(accountID)];
        auto it = [&] {
            if (auto i = map.find(sub); i != map.end()) {
                if (height > i->second.pos_.height_) {
                    i->second.pos_ = Position{height, std::move(hash)};
                }

                return i;
            } else {
                return map.try_emplace(sub, height, std::move(hash)).first;
            }
        }();

        it->second.test();
    }

    Imp(const ot::api::Session& api) noexcept
        : api_(api)
        , cb_(Callback::Factory([&](auto&& msg) { cb(std::move(msg)); }))
        , socket_([&] {
            auto out = api_.Network().ZeroMQ().Internal().SubscribeSocket(cb_);
            const auto rc =
                out->Start(api_.Endpoints().BlockchainScanProgress().data());

            OT_ASSERT(rc);

            return out;
        }())
        , lock_()
        , map_()
    {
    }
};
}  // namespace ottest

namespace ottest
{
ScanListener::ScanListener(const ot::api::Session& api) noexcept
    : imp_(std::make_unique<Imp>(api))
{
}

auto ScanListener::get_future(
    const Subaccount& account,
    Subchain subchain,
    Height target) noexcept -> Future
{
    auto lock = ot::Lock{imp_->lock_};
    const auto& nym = account.Parent().NymID();
    const auto chain = account.Parent().Parent().Chain();
    const auto& id = account.ID();
    auto& map = imp_->map_[nym][chain][id];
    auto it = [&] {
        if (auto i = map.find(subchain); i != map.end()) {

            return i;
        } else {
            return map.try_emplace(subchain, -1, ot::blockchain::block::Hash{})
                .first;
        }
    }();

    return it->second.reset(target);
}

auto ScanListener::wait(const Future& future) const noexcept -> bool
{
    constexpr auto limit = std::chrono::minutes{5};
    using Status = std::future_status;

    if (Status::ready == future.wait_for(limit)) {

        return true;
    } else {

        return false;
    }
}

ScanListener::~ScanListener() = default;
}  // namespace ottest
