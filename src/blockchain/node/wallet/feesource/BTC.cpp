// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"  // IWYU pragma: associated
#include "internal/blockchain/node/wallet/Factory.hpp"  // IWYU pragma: associated

#include <boost/json.hpp>
#include <boost/smart_ptr/make_shared.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>
#include <exception>
#include <optional>
#include <string_view>
#include <utility>

#include "blockchain/node/wallet/feesource/FeeSource.hpp"
#include "internal/blockchain/node/wallet/FeeSource.hpp"
#include "internal/network/zeromq/Context.hpp"
#include "internal/network/zeromq/Types.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::blockchain::node::wallet
{
using namespace std::literals;

class Bitcoiner_live final : public FeeSource::Imp
{
public:
    Bitcoiner_live(
        std::shared_ptr<const api::Session> api,
        std::shared_ptr<const node::Manager> node,
        network::zeromq::BatchID batch,
        allocator_type alloc) noexcept
        : Imp(std::move(api),
              std::move(node),
              "bitcoiner.live"sv,
              "/api/fees/estimates/latest"sv,
              true,
              std::move(batch),
              std::move(alloc))
    {
        LogTrace()(OT_PRETTY_CLASS())("My notification endpoint is ")(asio_)
            .Flush();
    }

private:
    auto process(const boost::json::value& data) noexcept
        -> std::optional<Amount> final
    {
        try {
            const auto& rate =
                data.at("estimates").at("30").at("sat_per_vbyte").as_double();
            LogTrace()(OT_PRETTY_CLASS())("Received fee estimate from API: ")(
                rate)
                .Flush();

            return process_double(rate, 1000);
        } catch (const std::exception& e) {
            LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

            return std::nullopt;
        }
    }
};

class BitGo final : public FeeSource::Imp
{
public:
    BitGo(
        std::shared_ptr<const api::Session> api,
        std::shared_ptr<const node::Manager> node,
        network::zeromq::BatchID batch,
        allocator_type alloc) noexcept
        : Imp(std::move(api),
              std::move(node),
              "www.bitgo.com"sv,
              "/api/v2/btc/tx/fee"sv,
              true,
              std::move(batch),
              std::move(alloc))
    {
        LogTrace()(OT_PRETTY_CLASS())("My notification endpoint is ")(asio_)
            .Flush();
    }

private:
    auto process(const boost::json::value& data) noexcept
        -> std::optional<Amount> final
    {
        try {
            const auto& rate = data.at("feePerKb").as_int64();
            LogTrace()(OT_PRETTY_CLASS())("Received fee estimate from API: ")(
                rate)
                .Flush();

            return process_int(rate, 1);
        } catch (const std::exception& e) {
            LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

            return std::nullopt;
        }
    }
};

class Bitpay final : public FeeSource::Imp
{
public:
    Bitpay(
        std::shared_ptr<const api::Session> api,
        std::shared_ptr<const node::Manager> node,
        network::zeromq::BatchID batch,
        allocator_type alloc) noexcept
        : Imp(std::move(api),
              std::move(node),
              "insight.bitpay.com"sv,
              "/api/utils/estimatefee?nbBlocks=2,4,6"sv,
              true,
              std::move(batch),
              std::move(alloc))
    {
        LogTrace()(OT_PRETTY_CLASS())("My notification endpoint is ")(asio_)
            .Flush();
    }

private:
    auto process(const boost::json::value& data) noexcept
        -> std::optional<Amount> final
    {
        try {
            const auto& rate = data.at("2").as_double();
            LogTrace()(OT_PRETTY_CLASS())("Received fee estimate from API: ")(
                rate)
                .Flush();

            return process_double(rate, 100000);
        } catch (const std::exception& e) {
            LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

            return std::nullopt;
        }
    }
};

class Blockchain_info final : public FeeSource::Imp
{
public:
    Blockchain_info(
        std::shared_ptr<const api::Session> api,
        std::shared_ptr<const node::Manager> node,
        network::zeromq::BatchID batch,
        allocator_type alloc) noexcept
        : Imp(std::move(api),
              std::move(node),
              "api.blockchain.info"sv,
              "/mempool/fees"sv,
              true,
              std::move(batch),
              std::move(alloc))
    {
        LogTrace()(OT_PRETTY_CLASS())("My notification endpoint is ")(asio_)
            .Flush();
    }

private:
    auto process(const boost::json::value& data) noexcept
        -> std::optional<Amount> final
    {
        try {
            const auto& rate = data.at("regular").as_int64();
            LogTrace()(OT_PRETTY_CLASS())("Received fee estimate from API: ")(
                rate)
                .Flush();

            return process_int(rate, 1000);
        } catch (const std::exception& e) {
            LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

            return std::nullopt;
        }
    }
};

class Blockchair final : public FeeSource::Imp
{
public:
    Blockchair(
        std::shared_ptr<const api::Session> api,
        std::shared_ptr<const node::Manager> node,
        network::zeromq::BatchID batch,
        allocator_type alloc) noexcept
        : Imp(std::move(api),
              std::move(node),
              "api.blockchair.com"sv,
              "/bitcoin/stats"sv,
              true,
              std::move(batch),
              std::move(alloc))
    {
        LogTrace()(OT_PRETTY_CLASS())("My notification endpoint is ")(asio_)
            .Flush();
    }

private:
    auto process(const boost::json::value& data) noexcept
        -> std::optional<Amount> final
    {
        try {
            const auto& rate = data.at("data")
                                   .at("suggested_transaction_fee_per_byte_sat")
                                   .as_int64();
            LogTrace()(OT_PRETTY_CLASS())("Received fee estimate from API: ")(
                rate)
                .Flush();

            return process_int(rate, 1000);
        } catch (const std::exception& e) {
            LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

            return std::nullopt;
        }
    }
};

class BlockCypher final : public FeeSource::Imp
{
public:
    BlockCypher(
        std::shared_ptr<const api::Session> api,
        std::shared_ptr<const node::Manager> node,
        network::zeromq::BatchID batch,
        allocator_type alloc) noexcept
        : Imp(std::move(api),
              std::move(node),
              "api.blockcypher.com"sv,
              "/v1/btc/main"sv,
              true,
              std::move(batch),
              std::move(alloc))
    {
        LogTrace()(OT_PRETTY_CLASS())("My notification endpoint is ")(asio_)
            .Flush();
    }

private:
    auto process(const boost::json::value& data) noexcept
        -> std::optional<Amount> final
    {
        try {
            const auto& rate = data.at("medium_fee_per_kb").as_int64();
            LogTrace()(OT_PRETTY_CLASS())("Received fee estimate from API: ")(
                rate)
                .Flush();

            return process_int(rate, 1);
        } catch (const std::exception& e) {
            LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

            return std::nullopt;
        }
    }
};

class Blockstream final : public FeeSource::Imp
{
public:
    Blockstream(
        std::shared_ptr<const api::Session> api,
        std::shared_ptr<const node::Manager> node,
        network::zeromq::BatchID batch,
        allocator_type alloc) noexcept
        : Imp(std::move(api),
              std::move(node),
              "blockstream.info"sv,
              "/api/fee-estimates"sv,
              true,
              std::move(batch),
              std::move(alloc))
    {
        LogTrace()(OT_PRETTY_CLASS())("My notification endpoint is ")(asio_)
            .Flush();
    }

private:
    auto process(const boost::json::value& data) noexcept
        -> std::optional<Amount> final
    {
        try {
            const auto& rate = data.at("2").as_double();
            LogTrace()(OT_PRETTY_CLASS())("Received fee estimate from API: ")(
                rate)
                .Flush();

            return process_double(rate, 1000);
        } catch (const std::exception& e) {
            LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

            return std::nullopt;
        }
    }
};

class BTC_com final : public FeeSource::Imp
{
public:
    BTC_com(
        std::shared_ptr<const api::Session> api,
        std::shared_ptr<const node::Manager> node,
        network::zeromq::BatchID batch,
        allocator_type alloc) noexcept
        : Imp(std::move(api),
              std::move(node),
              "btc.com"sv,
              "/service/fees/distribution"sv,
              true,
              std::move(batch),
              std::move(alloc))
    {
        LogTrace()(OT_PRETTY_CLASS())("My notification endpoint is ")(asio_)
            .Flush();
    }

private:
    auto process(const boost::json::value& data) noexcept
        -> std::optional<Amount> final
    {
        try {
            const auto& rate =
                data.at("fees_recommended").at("one_block_fee").as_int64();
            LogTrace()(OT_PRETTY_CLASS())("Received fee estimate from API: ")(
                rate)
                .Flush();

            return process_int(rate, 1000);
        } catch (const std::exception& e) {
            LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

            return std::nullopt;
        }
    }
};

class Earn final : public FeeSource::Imp
{
public:
    Earn(
        std::shared_ptr<const api::Session> api,
        std::shared_ptr<const node::Manager> node,
        network::zeromq::BatchID batch,
        allocator_type alloc) noexcept
        : Imp(std::move(api),
              std::move(node),
              "bitcoinfees.earn.com"sv,
              "/api/v1/fees/recommended"sv,
              true,
              std::move(batch),
              std::move(alloc))
    {
        LogTrace()(OT_PRETTY_CLASS())("My notification endpoint is ")(asio_)
            .Flush();
    }

private:
    auto process(const boost::json::value& data) noexcept
        -> std::optional<Amount> final
    {
        try {
            const auto& rate = data.at("hourFee").as_int64();
            LogTrace()(OT_PRETTY_CLASS())("Received fee estimate from API: ")(
                rate)
                .Flush();

            return process_int(rate, 1000);
        } catch (const std::exception& e) {
            LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

            return std::nullopt;
        }
    }
};
}  // namespace opentxs::blockchain::node::wallet

namespace opentxs::factory
{
auto BTCFeeSources(
    std::shared_ptr<const api::Session> api,
    std::shared_ptr<const blockchain::node::Manager> node) noexcept -> void
{
    OT_ASSERT(api);
    OT_ASSERT(node);

    using Source = blockchain::node::wallet::FeeSource;
    const auto& asio = api->Network().ZeroMQ().Internal();
    // TODO the version of libc++ present in android ndk 23.0.7599858 has a
    // broken std::allocate_shared function so we're using boost::shared_ptr
    // instead of std::shared_ptr
    // clang-format off
    Source{[&]() -> boost::shared_ptr<Source::Imp> {
        using Imp = blockchain::node::wallet::Bitcoiner_live;
        const auto batchID = asio.PreallocateBatch();

        return boost::allocate_shared<Imp>(
            alloc::PMR<Imp>{asio.Alloc(batchID)}, api, node, batchID);
    }()}.Init();
    Source{[&]() -> boost::shared_ptr<Source::Imp> {
        using Imp = blockchain::node::wallet::BitGo;
        const auto batchID = asio.PreallocateBatch();

        return boost::allocate_shared<Imp>(
            alloc::PMR<Imp>{asio.Alloc(batchID)}, api, node, batchID);
    }()}.Init();
    Source{[&]() -> boost::shared_ptr<Source::Imp> {
        using Imp = blockchain::node::wallet::Bitpay;
        const auto batchID = asio.PreallocateBatch();

        return boost::allocate_shared<Imp>(
            alloc::PMR<Imp>{asio.Alloc(batchID)}, api, node, batchID);
    }()}.Init();
    Source{[&]() -> boost::shared_ptr<Source::Imp> {
        using Imp = blockchain::node::wallet::Blockchain_info;
        const auto batchID = asio.PreallocateBatch();

        return boost::allocate_shared<Imp>(
            alloc::PMR<Imp>{asio.Alloc(batchID)}, api, node, batchID);
    }()}.Init();
    Source{[&]() -> boost::shared_ptr<Source::Imp> {
        using Imp = blockchain::node::wallet::Blockchair;
        const auto batchID = asio.PreallocateBatch();

        return boost::allocate_shared<Imp>(
            alloc::PMR<Imp>{asio.Alloc(batchID)}, api, node, batchID);
    }()}.Init();
    Source{[&]() -> boost::shared_ptr<Source::Imp> {
        using Imp = blockchain::node::wallet::BlockCypher;
        const auto batchID = asio.PreallocateBatch();

        return boost::allocate_shared<Imp>(
            alloc::PMR<Imp>{asio.Alloc(batchID)}, api, node, batchID);
    }()}.Init();
    Source{[&]() -> boost::shared_ptr<Source::Imp> {
        using Imp = blockchain::node::wallet::Blockstream;
        const auto batchID = asio.PreallocateBatch();

        return boost::allocate_shared<Imp>(
            alloc::PMR<Imp>{asio.Alloc(batchID)}, api, node, batchID);
    }()}.Init();
    Source{[&]() -> boost::shared_ptr<Source::Imp> {
        using Imp = blockchain::node::wallet::BTC_com;
        const auto batchID = asio.PreallocateBatch();

        return boost::allocate_shared<Imp>(
            alloc::PMR<Imp>{asio.Alloc(batchID)}, api, node, batchID);
    }()}.Init();
    Source{[&]() -> boost::shared_ptr<Source::Imp> {
        using Imp = blockchain::node::wallet::Earn;
        const auto batchID = asio.PreallocateBatch();

        return boost::allocate_shared<Imp>(
            alloc::PMR<Imp>{asio.Alloc(batchID)}, api, node, batchID);
    }()}.Init();
    // clang-format on
}
}  // namespace opentxs::factory
