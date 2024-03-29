// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "interface/ui/blockchainaccountstatus/BlockchainAccountStatus.hpp"  // IWYU pragma: associated

#include <atomic>
#include <chrono>
#include <functional>
#include <memory>
#include <optional>
#include <span>
#include <sstream>
#include <stdexcept>
#include <string_view>
#include <utility>

#include "internal/blockchain/crypto/Subaccount.hpp"
#include "internal/network/zeromq/Pipeline.hpp"
#include "opentxs/api/Network.hpp"
#include "opentxs/api/crypto/Blockchain.hpp"
#include "opentxs/api/network/Blockchain.hpp"
#include "opentxs/api/network/BlockchainHandle.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Endpoints.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/blockchain/crypto/Account.hpp"
#include "opentxs/blockchain/crypto/Subaccount.hpp"
#include "opentxs/blockchain/crypto/SubaccountType.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/crypto/Subchain.hpp"        // IWYU pragma: keep
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/blockchain/node/HeaderOracle.hpp"
#include "opentxs/blockchain/node/Manager.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::factory
{
auto BlockchainAccountStatusModel(
    const api::session::Client& api,
    const ui::implementation::BlockchainAccountStatusPrimaryID& id,
    const blockchain::Type chain,
    const SimpleCallback& cb) noexcept
    -> std::unique_ptr<ui::internal::BlockchainAccountStatus>
{
    using ReturnType = ui::implementation::BlockchainAccountStatus;

    return std::make_unique<ReturnType>(api, id, chain, cb);
}
}  // namespace opentxs::factory

namespace opentxs::ui::implementation
{
BlockchainAccountStatus::BlockchainAccountStatus(
    const api::session::Client& api,
    const BlockchainAccountStatusPrimaryID& id,
    const blockchain::Type chain,
    const SimpleCallback& cb) noexcept
    : BlockchainAccountStatusType(api, id, cb, false)
    , Worker(api, 100ms, "ui::BlockchainAccountStatus")
    , chain_(chain)
{
    init_executor({
        UnallocatedCString{api.Endpoints().BlockchainAccountCreated()},
        UnallocatedCString{api.Endpoints().BlockchainReorg()},
        UnallocatedCString{api.Endpoints().BlockchainScanProgress()},
    });
    pipeline_.Push(MakeWork(Work::init));
}

auto BlockchainAccountStatus::add_children(ChildMap&& map) noexcept -> void
{
    add_items([&] {
        auto rows = ChildDefinitions{};

        for (auto& [sourceType, sourceMap] : map) {
            for (auto& it : sourceMap) {
                auto& [sourceID, data] = it;
                auto& [sourceName, custom] = data;
                rows.emplace_back(
                    std::move(sourceID),
                    std::make_pair(sourceType, std::move(sourceName)),
                    CustomData{},
                    std::move(custom));
            }
        }

        return rows;
    }());
}

auto BlockchainAccountStatus::construct_row(
    const BlockchainAccountStatusRowID& id,
    const BlockchainAccountStatusSortKey& index,
    CustomData& custom) const noexcept -> RowPointer
{
    return factory::BlockchainSubaccountSourceWidget(
        *this, api_, id, index, custom);
}

auto BlockchainAccountStatus::load() noexcept -> void
{
    try {
        auto map = [&] {
            auto out = ChildMap{};
            const auto& api = api_;
            const auto& account =
                api.Crypto().Blockchain().Account(primary_id_, chain_);

            for (const auto& subaccount : account.GetSubaccounts()) {
                populate(
                    account,
                    subaccount.ID(),
                    subaccount.Type(),
                    blockchain::crypto::Subchain::Error,  // NOTE: all subchains
                    out);
            }

            return out;
        }();
        add_children(std::move(map));
    } catch (const std::exception& e) {
        LogError()()(e.what()).Flush();
    }
}

auto BlockchainAccountStatus::pipeline(Message&& in) noexcept -> void
{
    if (false == running_.load()) { return; }

    const auto body = in.Payload();

    if (1 > body.size()) {
        LogError()()("Invalid message").Flush();

        LogAbort()().Abort();
    }

    const auto work = [&] {
        try {

            return body[0].as<Work>();
        } catch (...) {

            LogAbort()().Abort();
        }
    }();

    if ((false == startup_complete()) && (Work::init != work)) {
        pipeline_.Push(std::move(in));

        return;
    }

    switch (work) {
        case Work::shutdown: {
            if (auto previous = running_.exchange(false); previous) {
                shutdown(shutdown_promise_);
            }
        } break;
        case Work::newaccount: {
            process_account(in);
        } break;
        case Work::header:
        case Work::reorg: {
            process_reorg(in);
        } break;
        case Work::progress: {
            process_progress(in);
        } break;
        case Work::init: {
            startup();
        } break;
        case Work::statemachine: {
            do_work();
        } break;
        default: {
            LogError()()("Unhandled type").Flush();

            LogAbort()().Abort();
        }
    }
}

auto BlockchainAccountStatus::populate(
    const blockchain::crypto::Account& account,
    const identifier::Account& subaccountID,
    const blockchain::crypto::SubaccountType type,
    const blockchain::crypto::Subchain subchain,
    ChildMap& out) const noexcept -> void
{
    const auto& subaccount = account.Subaccount(subaccountID);
    const auto& internal = subaccount.Internal();
    populate(
        subaccount,
        internal.Source(),
        internal.SourceDescription(),
        internal.DisplayName(),
        subchain,
        out[internal.DisplayType()]);
}

auto BlockchainAccountStatus::populate(
    const blockchain::crypto::Subaccount& node,
    const identifier::Generic& sourceID,
    std::string_view sourceDescription,
    std::string_view subaccountName,
    blockchain::crypto::Subchain subchain,
    SubaccountMap& out) const noexcept -> void
{
    auto& data = out[sourceID];
    auto& [sourceText, cantCapture] = data;

    if (sourceText.empty()) { sourceText = sourceDescription; }

    auto& subaccounts = [&]() -> auto& {
        using Subaccounts =
            UnallocatedVector<BlockchainSubaccountSourceRowData>;
        auto* ptr = [&] {
            auto& custom = data.second;

            if (0u == custom.size()) {

                return custom.emplace_back(
                    std::make_unique<Subaccounts>().release());
            } else {
                assert_true(1u == custom.size());

                return custom.front();
            }
        }();

        assert_false(nullptr == ptr);

        return *reinterpret_cast<Subaccounts*>(ptr);
    }();
    using Subchains = UnallocatedVector<BlockchainSubaccountRowData>;
    auto& subaccount = subaccounts.emplace_back(
        node.ID(),
        UnallocatedCString{subaccountName},
        CustomData{},
        CustomData{});
    auto& subchainData = [&]() -> auto& {
        auto& children = subaccount.children_;

        assert_true(0u == children.size());

        auto& ptr =
            children.emplace_back(std::make_unique<Subchains>().release());

        assert_true(1u == children.size());
        assert_false(nullptr == ptr);

        return *reinterpret_cast<Subchains*>(ptr);
    }();
    const auto subchainList = [&]() -> Set<blockchain::crypto::Subchain> {
        if (blockchain::crypto::Subchain::Error == subchain) {

            return node.AllowedSubchains();
        }

        return {subchain};
    }();

    for (const auto subtype : subchainList) {
        auto [name, progress] = subchain_display_name(node, subtype);
        subchainData.emplace_back(
            subtype, std::move(name), std::move(progress), CustomData{});
    }
}

auto BlockchainAccountStatus::process_account(const Message& in) noexcept
    -> void
{
    const auto& api = api_;
    auto body = in.Payload();

    assert_true(4 < body.size());

    const auto chain = body[1].as<blockchain::Type>();

    if (chain != chain_) { return; }

    const auto owner = api.Factory().IdentifierFromHash(body[2].Bytes());

    if (owner != primary_id_) { return; }

    const auto type = body[3].as<blockchain::crypto::SubaccountType>();
    const auto subaccountID = api.Factory().AccountIDFromZMQ(body[4]);
    const auto& account =
        api.Crypto().Blockchain().Account(primary_id_, chain_);

    try {
        auto map = [&] {
            auto out = ChildMap{};
            populate(
                account,
                subaccountID,
                type,
                blockchain::crypto::Subchain::Error,  // NOTE: all subchains
                out);

            return out;
        }();
        add_children(std::move(map));
    } catch (const std::exception& e) {
        LogError()()(e.what()).Flush();
    }
}

auto BlockchainAccountStatus::process_progress(const Message& in) noexcept
    -> void
{
    const auto& api = api_;
    auto body = in.Payload();

    assert_true(5 < body.size());

    const auto chain = body[1].as<blockchain::Type>();

    if (chain != chain_) { return; }

    const auto owner = api.Factory().IdentifierFromHash(body[2].Bytes());

    if (owner != primary_id_) { return; }

    const auto type = body[3].as<blockchain::crypto::SubaccountType>();
    const auto subaccountID = api.Factory().AccountIDFromZMQ(body[4]);
    const auto subchain = body[5].as<blockchain::crypto::Subchain>();
    const auto& account =
        api.Crypto().Blockchain().Account(primary_id_, chain_);

    try {
        auto map = [&] {
            auto out = ChildMap{};
            populate(account, subaccountID, type, subchain, out);

            return out;
        }();
        add_children(std::move(map));
    } catch (const std::exception& e) {
        LogError()()(e.what()).Flush();
    }
}

auto BlockchainAccountStatus::process_reorg(const Message& in) noexcept -> void
{
    const auto body = in.Payload();

    assert_true(1 < body.size());

    const auto chain = body[1].as<blockchain::Type>();

    if (chain != chain_) { return; }

    load();
}

auto BlockchainAccountStatus::startup() noexcept -> void
{
    load();
    finish_startup();
    trigger();
}

auto BlockchainAccountStatus::subchain_display_name(
    const blockchain::crypto::Subaccount& node,
    BlockchainSubaccountRowID subchain) const noexcept
    -> std::pair<BlockchainSubaccountSortKey, CustomData>
{
    auto output = std::pair<BlockchainSubaccountSortKey, CustomData>{};
    auto& nameOut = output.first;
    auto& progressOut = *static_cast<UnallocatedCString*>(
        output.second.emplace_back(new UnallocatedCString{}));
    auto name = std::stringstream{};
    auto progress = std::stringstream{};
    using Height = blockchain::block::Height;
    const auto target = [&]() -> std::optional<Height> {
        try {
            const auto& api = api_;
            const auto handle = api.Network().Blockchain().GetChain(
                blockchain::crypto::base_chain(node.Parent().Target()));

            if (false == handle.IsValid()) {
                throw std::runtime_error{"invalid chain"};
            }

            const auto& chain = handle.get();

            return chain.HeaderOracle().BestChain().height_;
        } catch (...) {

            return std::nullopt;
        }
    }();
    const auto scanned = [&]() -> std::optional<Height> {
        try {

            return node.ScanProgress(subchain).height_;
        } catch (...) {

            return std::nullopt;
        }
    }();
    auto actual = scanned.value_or(0);
    auto eTarget = target.value_or(1);
    const auto eProgress = internal::make_progress(actual, eTarget);
    const auto percent = [&] {
        auto out = std::stringstream{};

        if (target.has_value()) {
            out << std::to_string(eProgress);
        } else {
            out << "?";
        }

        out << " %";

        return out.str();
    }();
    name << print(subchain) << " subchain";
    progress << std::to_string(actual);
    progress << " of ";

    if (target.has_value()) {
        progress << std::to_string(eTarget);
    } else {
        progress << "?";
    }

    progress << " (";
    progress << percent;
    progress << ')';
    name << ": " << progress.str();
    nameOut = name.str();
    progressOut = progress.str();

    return output;
}

BlockchainAccountStatus::~BlockchainAccountStatus() = default;
}  // namespace opentxs::ui::implementation
