// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated
#include "internal/blockchain/node/wallet/Factory.hpp"  // IWYU pragma: associated

#include <boost/json.hpp>
#include <exception>
#include <new>
#include <optional>
#include <utility>

#include "blockchain/node/wallet/feeoracle/FeeSource.hpp"
#include "internal/blockchain/node/wallet/FeeSource.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/util/Allocated.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::blockchain::node::wallet
{
class Bitcoiner_live final : public FeeSource::Imp
{
public:
    Bitcoiner_live(
        const api::Session& api,
        const std::string_view endpoint,
        allocator_type alloc) noexcept
        : Imp(api,
              CString{endpoint, alloc},
              CString{"bitcoiner.live", alloc},
              CString{"/api/fees/estimates/latest", alloc},
              true,
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
        const api::Session& api,
        const std::string_view endpoint,
        allocator_type alloc) noexcept
        : Imp(api,
              CString{endpoint, alloc},
              CString{"www.bitgo.com", alloc},
              CString{"/api/v2/btc/tx/fee", alloc},
              true,
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
        const api::Session& api,
        const std::string_view endpoint,
        allocator_type alloc) noexcept
        : Imp(api,
              CString{endpoint, alloc},
              CString{"insight.bitpay.com", alloc},
              CString{"/api/utils/estimatefee?nbBlocks=2,4,6", alloc},
              true,
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
        const api::Session& api,
        const std::string_view endpoint,
        allocator_type alloc) noexcept
        : Imp(api,
              CString{endpoint, alloc},
              CString{"api.blockchain.info", alloc},
              CString{"/mempool/fees", alloc},
              true,
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
        const api::Session& api,
        const std::string_view endpoint,
        allocator_type alloc) noexcept
        : Imp(api,
              CString{endpoint, alloc},
              CString{"api.blockchair.com", alloc},
              CString{"/bitcoin/stats", alloc},
              true,
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
        const api::Session& api,
        const std::string_view endpoint,
        allocator_type alloc) noexcept
        : Imp(api,
              CString{endpoint, alloc},
              CString{"api.blockcypher.com", alloc},
              CString{"/v1/btc/main", alloc},
              true,
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
        const api::Session& api,
        const std::string_view endpoint,
        allocator_type alloc) noexcept
        : Imp(api,
              CString{endpoint, alloc},
              CString{"blockstream.info", alloc},
              CString{"/api/fee-estimates", alloc},
              true,
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
        const api::Session& api,
        const std::string_view endpoint,
        allocator_type alloc) noexcept
        : Imp(api,
              CString{endpoint, alloc},
              CString{"btc.com", alloc},
              CString{"/service/fees/distribution", alloc},
              true,
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
        const api::Session& api,
        const std::string_view endpoint,
        allocator_type alloc) noexcept
        : Imp(api,
              CString{endpoint, alloc},
              CString{"bitcoinfees.earn.com", alloc},
              CString{"/api/v1/fees/recommended", alloc},
              true,
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
    const api::Session& api,
    const std::string_view endpoint,
    alloc::Resource* mr) noexcept
    -> ForwardList<blockchain::node::wallet::FeeSource>
{
    if (nullptr == mr) { mr = alloc::System(); }

    using ReturnType = blockchain::node::wallet::FeeSource;
    auto alloc = ReturnType::allocator_type{mr};
    auto* res = alloc.resource();
    auto sources = ForwardList<ReturnType>{alloc};
    sources.emplace_front([&] {
        using Imp = blockchain::node::wallet::Bitcoiner_live;
        auto* out = res->allocate(sizeof(Imp), alignof(Imp));

        return new (out) Imp{api, endpoint, alloc};
    }());
    sources.emplace_front([&] {
        using Imp = blockchain::node::wallet::BitGo;
        auto* out = res->allocate(sizeof(Imp), alignof(Imp));

        return new (out) Imp{api, endpoint, alloc};
    }());
    sources.emplace_front([&] {
        using Imp = blockchain::node::wallet::Bitpay;
        auto* out = res->allocate(sizeof(Imp), alignof(Imp));

        return new (out) Imp{api, endpoint, alloc};
    }());
    sources.emplace_front([&] {
        using Imp = blockchain::node::wallet::Blockchain_info;
        auto* out = res->allocate(sizeof(Imp), alignof(Imp));

        return new (out) Imp{api, endpoint, alloc};
    }());
    sources.emplace_front([&] {
        using Imp = blockchain::node::wallet::Blockchair;
        auto* out = res->allocate(sizeof(Imp), alignof(Imp));

        return new (out) Imp{api, endpoint, alloc};
    }());
    sources.emplace_front([&] {
        using Imp = blockchain::node::wallet::BlockCypher;
        auto* out = res->allocate(sizeof(Imp), alignof(Imp));

        return new (out) Imp{api, endpoint, alloc};
    }());
    sources.emplace_front([&] {
        using Imp = blockchain::node::wallet::Blockstream;
        auto* out = res->allocate(sizeof(Imp), alignof(Imp));

        return new (out) Imp{api, endpoint, alloc};
    }());
    sources.emplace_front([&] {
        using Imp = blockchain::node::wallet::BTC_com;
        auto* out = res->allocate(sizeof(Imp), alignof(Imp));

        return new (out) Imp{api, endpoint, alloc};
    }());
    sources.emplace_front([&] {
        using Imp = blockchain::node::wallet::Earn;
        auto* out = res->allocate(sizeof(Imp), alignof(Imp));

        return new (out) Imp{api, endpoint, alloc};
    }());

    return sources;
}
}  // namespace opentxs::factory
