// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "interface/ui/blockchainselection/BlockchainSelection.hpp"  // IWYU pragma: associated

#include <future>
#include <memory>
#include <string_view>
#include <utility>

#include "internal/api/network/Blockchain.hpp"
#include "internal/network/zeromq/Pipeline.hpp"
#include "internal/network/zeromq/socket/Types.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/network/Blockchain.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Endpoints.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/interface/ui/Blockchains.hpp"  // IWYU pragma: keep
#include "opentxs/interface/ui/Types.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/message/FrameSection.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/network/zeromq/message/Message.tpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"

namespace zmq = opentxs::network::zeromq;

namespace opentxs::factory
{
auto BlockchainSelectionModel(
    const api::session::Client& api,
    const ui::Blockchains type,
    const SimpleCallback& cb) noexcept
    -> std::unique_ptr<ui::internal::BlockchainSelection>
{
    using ReturnType = ui::implementation::BlockchainSelection;

    return std::make_unique<ReturnType>(api, type, cb);
}
}  // namespace opentxs::factory

namespace opentxs::ui::implementation
{
BlockchainSelection::BlockchainSelection(
    const api::session::Client& api,
    const ui::Blockchains type,
    const SimpleCallback& cb) noexcept
    : BlockchainSelectionList(api, {}, cb, false)
    , Worker(api, {}, "ui::BlockchainSelection")
    , filter_(filter(type))
    , chain_state_([&] {
        auto out = UnallocatedMap<blockchain::Type, bool>{};

        for (const auto chain : filter_) { out[chain] = false; }

        return out;
    }())
    , enabled_count_(0)
    , enabled_callback_()
{
    init_executor(
        {UnallocatedCString{api.Endpoints().BlockchainStateChange()}});
    pipeline_.Push(MakeWork(Work::init));
}

auto BlockchainSelection::construct_row(
    const BlockchainSelectionRowID& id,
    const BlockchainSelectionSortKey& index,
    CustomData& custom) const noexcept -> RowPointer
{
    return factory::BlockchainSelectionItem(*this, api_, id, index, custom);
}

auto BlockchainSelection::Disable(const blockchain::Type type) const noexcept
    -> bool
{
    pipeline_.Push([&] {
        auto out = network::zeromq::tagged_message(Work::disable);
        out.AddFrame(type);

        return out;
    }());

    return true;
}

auto BlockchainSelection::disable(const Message& in) noexcept -> void
{
    const auto body = in.Body();

    OT_ASSERT(1 < body.size());

    const auto chain = body.at(1).as<blockchain::Type>();
    process_state(chain, false);
    api_.Network().Blockchain().Disable(chain);
}

auto BlockchainSelection::Enable(const blockchain::Type type) const noexcept
    -> bool
{
    pipeline_.Push([&] {
        auto out = network::zeromq::tagged_message(Work::enable);
        out.AddFrame(type);

        return out;
    }());

    return true;
}

auto BlockchainSelection::enable(const Message& in) noexcept -> void
{
    const auto body = in.Body();

    OT_ASSERT(1 < body.size());

    const auto chain = body.at(1).as<blockchain::Type>();
    process_state(chain, true);
    api_.Network().Blockchain().Enable(chain);
}

auto BlockchainSelection::EnabledCount() const noexcept -> std::size_t
{
    return enabled_count_.load();
}

auto BlockchainSelection::filter(const ui::Blockchains type) noexcept
    -> UnallocatedSet<blockchain::Type>
{
    auto complete = blockchain::SupportedChains();

    switch (type) {
        case Blockchains::Main: {
            auto output = decltype(complete){};

            for (const auto& chain : complete) {
                if (false == blockchain::IsTestnet(chain)) {
                    output.emplace(chain);
                }
            }

            return output;
        }
        case Blockchains::Test: {
            auto output = decltype(complete){};

            for (const auto& chain : complete) {
                if (blockchain::IsTestnet(chain)) { output.emplace(chain); }
            }

            return output;
        }
        case Blockchains::All:
        default: {

            return complete;
        }
    }
}

auto BlockchainSelection::pipeline(const Message& in) noexcept -> void
{
    if (false == running_.load()) { return; }

    const auto body = in.Body();

    if (1 > body.size()) {
        LogError()(OT_PRETTY_CLASS())("Invalid message").Flush();

        OT_FAIL;
    }

    const auto work = [&] {
        try {

            return body.at(0).as<Work>();
        } catch (...) {

            OT_FAIL;
        }
    }();

    switch (work) {
        case Work::shutdown: {
            if (auto previous = running_.exchange(false); previous) {
                shutdown(shutdown_promise_);
            }
        } break;
        case Work::statechange: {
            process_state(in);
        } break;
        case Work::enable: {
            enable(in);
        } break;
        case Work::disable: {
            disable(in);
        } break;
        case Work::init: {
            startup();
        } break;
        case Work::statemachine: {
            do_work();
        } break;
        default: {
            LogError()(OT_PRETTY_CLASS())("Unhandled type: ")(
                static_cast<OTZMQWorkType>(work))
                .Flush();

            OT_FAIL;
        }
    }
}

auto BlockchainSelection::process_state(const Message& in) noexcept -> void
{
    const auto body = in.Body();

    OT_ASSERT(2 < body.size());

    process_state(body.at(1).as<blockchain::Type>(), body.at(2).as<bool>());
}

auto BlockchainSelection::process_state(
    const blockchain::Type chain,
    const bool enabled) const noexcept -> void
{
    if (0 == filter_.count(chain)) { return; }

    auto& isEnabled = chain_state_.at(chain);

    if (isEnabled) {
        if (enabled) {
            // Do nothing
        } else {
            isEnabled = false;
            --enabled_count_;
            enabled_callback_.run(chain, enabled, enabled_count_.load());
        }
    } else {
        if (enabled) {
            isEnabled = true;
            ++enabled_count_;
            enabled_callback_.run(chain, enabled, enabled_count_.load());
        } else {
            // Do nothing
        }
    }

    auto custom = CustomData{};
    custom.emplace_back(new bool{enabled});
    const_cast<BlockchainSelection&>(*this).add_item(
        chain,
        {UnallocatedCString{print(chain)}, blockchain::IsTestnet(chain)},
        custom);
}

auto BlockchainSelection::Set(EnabledCallback&& cb) const noexcept -> void
{
    enabled_callback_.set(std::move(cb));
}

auto BlockchainSelection::startup() noexcept -> void
{
    const auto& api = api_.Network().Blockchain().Internal();

    for (const auto& chain : filter_) {
        process_state(chain, api.IsEnabled(chain));
    }

    finish_startup();
}

BlockchainSelection::~BlockchainSelection()
{
    wait_for_startup();
    signal_shutdown().get();
}
}  // namespace opentxs::ui::implementation
