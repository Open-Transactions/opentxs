// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "blockchain/database/common/Peers.hpp"  // IWYU pragma: associated

#include <BlockchainPeerAddress.pb.h>
#include <algorithm>
#include <chrono>
#include <compare>
#include <cstring>
#include <iterator>
#include <memory>
#include <optional>
#include <random>
#include <ratio>
#include <stdexcept>
#include <utility>

#include "internal/network/blockchain/Address.hpp"
#include "internal/serialization/protobuf/Check.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "internal/serialization/protobuf/Proto.tpp"
#include "internal/serialization/protobuf/verify/BlockchainPeerAddress.hpp"
#include "internal/util/Future.hpp"
#include "internal/util/P0330.hpp"
#include "internal/util/Time.hpp"
#include "internal/util/storage/lmdb/Database.hpp"
#include "internal/util/storage/lmdb/Transaction.hpp"
#include "internal/util/storage/lmdb/Types.hpp"
#include "opentxs/Context.hpp"
#include "opentxs/api/Factory.internal.hpp"
#include "opentxs/api/Session.hpp"
#include "opentxs/api/Session.internal.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/network/blockchain/Address.hpp"
#include "opentxs/network/blockchain/Transport.hpp"  // IWYU pragma: keep
#include "opentxs/util/Log.hpp"
#include "util/ScopeGuard.hpp"

namespace opentxs::blockchain::database::common
{
template <typename Key, typename Map>
auto Peers::read_index(
    const ReadView key,
    const ReadView value,
    Map& map) noexcept(false) -> bool
{
    auto input = 0_uz;

    if (sizeof(input) != key.size()) {
        throw std::runtime_error("Invalid key");
    }

    std::memcpy(&input, key.data(), key.size());
    map[static_cast<Key>(input)].emplace(
        api_.Factory().IdentifierFromBase58(value));

    return true;
}
}  // namespace opentxs::blockchain::database::common

namespace opentxs::blockchain::database::common
{
Peers::Peers(const api::Session& api, storage::lmdb::Database& lmdb) noexcept(
    false)
    : log_(LogTrace())
    , api_(api)
    , lmdb_(lmdb)
    , data_()
    , future_([&] {
        auto promise = std::make_shared<std::promise<GuardedData&>>();
        RunJob([this, promise] { init(api_.Internal().GetShared(), promise); });

        return promise->get_future();
    }())
{
}

auto Peers::Confirm(
    const blockchain::Type chain,
    const network::blockchain::AddressID& id) noexcept -> void
{
    auto handle = get().lock()->chain_index_[chain].lock();
    auto& data = *handle;
    data.failed_.erase(id);
    data.untested_.erase(id);
    data.next_timeout_.erase(id);
    data.known_good_.emplace(id);
}

auto Peers::delete_peer(
    const blockchain::Type chain,
    const network::blockchain::AddressID& id) noexcept -> void
{
    {
        auto handle = [&] {
            auto lock = get().lock();
            auto& g = *lock;

            for (auto& [_, addresses] : g.chains_) { addresses.erase(id); }

            for (auto& [_, addresses] : g.protocols_) { addresses.erase(id); }

            for (auto& [_, addresses] : g.services_) { addresses.erase(id); }

            for (auto& [_, addresses] : g.networks_) { addresses.erase(id); }

            g.connected_.erase(id);

            return g.chain_index_[chain].lock();
        }();
        auto& data = *handle;
        data.in_use_.erase(id);
        data.untested_.erase(id);
        data.known_good_.erase(id);
        data.failed_.erase(id);
        data.next_timeout_.erase(id);

        for (auto& [_, addresses] : data.retry_) { addresses.erase(id); }
    }

    lmdb_.Delete(Table::PeerDetails, id.asBase58(api_.Crypto()));
    log_()("deleted stale ")(print(chain))(" peer ")(id, api_.Crypto()).Flush();
}

auto Peers::exists(
    const Index& data,
    const network::blockchain::AddressID& id) noexcept -> bool
{
    return data.known_good_.contains(id) || data.untested_.contains(id) ||
           data.failed_.contains(id);
}

auto Peers::Fail(
    const blockchain::Type chain,
    const network::blockchain::AddressID& id) noexcept -> void
{
    constexpr auto limit = 24h * 7;
    using namespace std::chrono;
    const auto last = duration_cast<hours>(Clock::now() - last_connected(id));

    if (last >= limit) {
        delete_peer(chain, id);
    } else {
        auto handle = get().lock()->chain_index_[chain].lock();
        auto& data = *handle;
        data.known_good_.erase(id);
        data.untested_.erase(id);
        data.failed_.emplace(id);
        data.retry_[next_timeout(data, id)].emplace(id);
    }
}

auto Peers::Find(
    const blockchain::Type chain,
    const network::blockchain::Protocol protocol,
    const Set<network::blockchain::Transport>& onNetworks,
    const Set<network::blockchain::bitcoin::Service>& withServices,
    const Set<network::blockchain::AddressID>& exclude) noexcept
    -> network::blockchain::Address
{
    const auto& log = log_;
    log()("loading a ")(print(chain))(" peer").Flush();
    const auto [candidates, haveServices] =
        get_candidates(chain, protocol, onNetworks, withServices, exclude);
    auto handle = get().lock()->chain_index_[chain].lock();
    auto& data = *handle;
    retry_peers(data);
    log()(print(chain))(" has ")(data.in_use_.size())(" in-use addresses")
        .Flush();
    log()(print(chain))(" has ")(data.known_good_.size())(
        " known good addresses")
        .Flush();
    log()(print(chain))(" has ")(data.failed_.size())(" failed addresses")
        .Flush();
    log()(print(chain))(" has ")(data.untested_.size())(" untested addresses")
        .Flush();
    const auto use = [&](const auto& in) {
        auto output = Vector<network::blockchain::AddressID>{};
        constexpr auto count = 1_uz;
        std::ranges::sample(
            in,
            std::back_inserter(output),
            count,
            std::mt19937{std::random_device{}()});

        assert_true(count == output.size());

        auto& peer = output.front();
        log()("Loading peer ")(peer, api_.Crypto()).Flush();
        data.in_use_.emplace(peer);

        return peer;
    };
    const auto getType = [&](const auto& lhs, const auto& rhs) {
        auto first = Vector<network::blockchain::AddressID>{};
        first.reserve(std::min(lhs.size(), rhs.size()));
        first.clear();
        std::ranges::set_intersection(lhs, rhs, std::back_inserter(first));
        auto second = Set<network::blockchain::AddressID>{};
        second.clear();
        std::ranges::set_difference(
            first, data.in_use_, std::inserter(second, second.end()));

        return second;
    };

    if (auto p = getType(haveServices, data.known_good_); p.empty()) {
        log()("No known good peers available with specified services").Flush();
    } else {
        log()(p.size())(
            " candidates with matching services in the known good set")
            .Flush();

        while (false == p.empty()) {
            const auto id = use(p);

            try {

                return load_address(id);
            } catch (...) {
                data.in_use_.erase(id);
                p.erase(id);
            }
        }

        log()(" all candidates failed to load").Flush();
    }

    if (auto p = getType(haveServices, data.untested_); p.empty()) {
        log()("No known untested peers available with specified services")
            .Flush();
    } else {
        log()(p.size())(
            " candidates with matching services in the untested set")
            .Flush();

        while (false == p.empty()) {
            const auto id = use(p);

            try {

                return load_address(id);
            } catch (...) {
                data.in_use_.erase(id);
                p.erase(id);
            }
        }

        log()(" all candidates failed to load").Flush();
    }

    if (auto p = getType(candidates, data.untested_); p.empty()) {
        log()("No peer candidates available for ")(print(chain)).Flush();
    } else {
        log()(p.size())(" untested candidates with unknown services").Flush();

        while (false == p.empty()) {
            const auto id = use(p);

            try {

                return load_address(id);
            } catch (...) {
                data.in_use_.erase(id);
                p.erase(id);
            }
        }

        log()(" all candidates failed to load").Flush();
    }

    return {};
}

auto Peers::get_candidates(
    const blockchain::Type chain,
    const network::blockchain::Protocol protocol,
    const Set<network::blockchain::Transport>& onNetworks,
    const Set<network::blockchain::bitcoin::Service>& withServices,
    const Set<network::blockchain::AddressID>& exclude) const noexcept
    -> std::pair<Addresses, Addresses>
{
    auto handle = get().lock_shared();
    const auto& data = *handle;
    auto out = std::make_pair(Addresses{}, Addresses{});
    auto& [candidates, haveServices] = out;

    if (false == data.chains_.contains(chain)) {
        log_()(" no known addresses for ")(print(chain)).Flush();

        return out;
    }

    const auto& chainAddresses = data.chains_.at(chain);
    log_()(chainAddresses.size())(" addresses for ")(print(chain)).Flush();

    if (false == data.protocols_.contains(protocol)) {
        log_()(" no known addresses for ")(print(protocol)).Flush();

        return out;
    }

    const auto& protocolAddresses = data.protocols_.at(protocol);
    log_()(protocolAddresses.size())(" addresses for ")(print(protocol))
        .Flush();

    for (const auto& network : onNetworks) {
        if (false == data.networks_.contains(network)) {
            log_()(" no known addresses for ")(print(network)).Flush();

            continue;
        }

        for (const auto& id : data.networks_.at(network)) {
            if (false == chainAddresses.contains(id)) { continue; }
            if (false == protocolAddresses.contains(id)) { continue; }
            if (exclude.contains(id)) { continue; }

            candidates.emplace(id);
        }
    }

    if (candidates.empty()) {
        log_()("No available peers match the requested "
               "chain, protocol, and transport")
            .Flush();

        return out;
    } else {
        log_()(candidates.size())(
            " available peers match the requested chain, protocol, and "
            "transport")
            .Flush();
    }

    if (withServices.empty()) {
        haveServices = candidates;
    } else {
        for (const auto& id : candidates) {
            bool haveAllServices{true};

            for (const auto& service : withServices) {
                try {
                    if (0 == data.services_.at(service).count(id)) {
                        haveAllServices = false;
                        break;
                    }
                } catch (...) {
                    haveAllServices = false;
                    break;
                }
            }

            if (haveAllServices) { haveServices.emplace(id); }
        }
    }

    if (haveServices.empty()) {
        log_()("No peers available with specified services").Flush();
    } else {
        log_()(haveServices.size())(
            " candidates advertise the requested services")
            .Flush();
    }

    return out;
}

auto Peers::Good(
    const blockchain::Type chain,
    alloc::Default alloc,
    alloc::Default monotonic) noexcept -> Vector<network::blockchain::Address>
{
    const auto ids = Addresses{
        get().lock()->chain_index_[chain].lock()->known_good_, monotonic};
    auto out = Vector<network::blockchain::Address>{alloc};
    out.reserve(ids.size());
    out.clear();

    for (const auto& id : ids) {
        try {
            out.emplace_back(load_address(id));
        } catch (const std::exception& e) {
            log_()(e.what()).Flush();
        }
    }

    return out;
}

auto Peers::Import(Vector<network::blockchain::Address>&& peers) noexcept
    -> bool
{
    auto newPeers = Vector<network::blockchain::Address>{};

    for (auto& peer : peers) {
        if (false == peer.IsValid()) { continue; }

        if (false ==
            lmdb_.Exists(
                Table::PeerDetails, peer.ID().asBase58(api_.Crypto()))) {
            newPeers.emplace_back(std::move(peer));
        }
    }

    auto handle = get().lock();

    return insert(*handle, newPeers);
}

// NOLINTBEGIN(clang-analyzer-cplusplus.NewDeleteLeaks)
auto Peers::init(
    std::shared_ptr<const api::internal::Session> p,
    std::shared_ptr<std::promise<GuardedData&>> promise) noexcept -> void
{
    assert_false(nullptr == p);
    assert_false(nullptr == promise);

    const auto post = ScopeGuard{[&] { promise->set_value(data_); }};
    const auto& api = *p;

    {
        auto handle = data_.lock();
        auto& data = *handle;
        using namespace storage::lmdb;
        using enum Dir;
        static const auto work = {
            std::make_pair<Table, ReadCallback>(
                PeerChainIndex,
                [&, this](const auto key, const auto value) {
                    if (api.ShuttingDown()) {
                        throw std::runtime_error{
                            "database read interrupted for shutdown"};
                    }

                    return read_index<Chain>(key, value, data.chains_);
                }),
            std::make_pair<Table, ReadCallback>(
                PeerProtocolIndex,
                [&, this](const auto key, const auto value) {
                    if (api.ShuttingDown()) {
                        throw std::runtime_error{
                            "database read interrupted for shutdown"};
                    }

                    return read_index<Protocol>(key, value, data.protocols_);
                }),
            std::make_pair<Table, ReadCallback>(
                PeerServiceIndex,
                [&, this](const auto key, const auto value) {
                    if (api.ShuttingDown()) {
                        throw std::runtime_error{
                            "database read interrupted for shutdown"};
                    }

                    return read_index<Service>(key, value, data.services_);
                }),
            std::make_pair<Table, ReadCallback>(
                PeerNetworkIndex,
                [&, this](const auto key, const auto value) {
                    if (api.ShuttingDown()) {
                        throw std::runtime_error{
                            "database read interrupted for shutdown"};
                    }

                    return read_index<Transport>(key, value, data.networks_);
                }),
            std::make_pair<Table, ReadCallback>(
                PeerConnectedIndex,
                [&, this](const auto key, const auto value) {
                    if (api.ShuttingDown()) {
                        throw std::runtime_error{
                            "database read interrupted for shutdown"};
                    }

                    auto input = 0_uz;

                    if (sizeof(input) != key.size()) {
                        throw std::runtime_error("Invalid key");
                    }

                    std::memcpy(&input, key.data(), key.size());
                    data.connected_.emplace(
                        api_.Factory().IdentifierFromBase58(value),
                        convert_stime(input));

                    return true;
                }),
        };
        init_tables(api.Self(), work);

        if (api.ShuttingDown()) { return; }

        const auto chains = [&] {
            // TODO monotonic allocator
            auto out = Vector<std::pair<const Addresses*, GuardedIndex*>>{};
            out.reserve(data.chains_.size());
            out.clear();

            for (const auto& [c, addresses] : data.chains_) {
                out.emplace_back(
                    std::addressof(addresses),
                    std::addressof(data.chain_index_[c]));
            }

            return out;
        }();
        init_chains(api.Self(), Clock::now(), data, chains);
    }
}
// NOLINTEND(clang-analyzer-cplusplus.NewDeleteLeaks)

auto Peers::Insert(network::blockchain::Address address) noexcept -> bool
{
    if (false == address.IsValid()) { return false; }

    auto peers = Vector<network::blockchain::Address>{};
    peers.emplace_back(std::move(address));
    auto handle = get().lock();

    return insert(*handle, peers);
}

auto Peers::insert(
    Data& data,
    const Vector<network::blockchain::Address>& peers) noexcept -> bool
{
    auto parentTxn = lmdb_.TransactionRW();

    for (const auto& address : peers) {
        assert_true(address.IsValid());

        const auto& id = address.ID();
        auto deleteServices = address.Internal().PreviousServices();

        for (const auto& service : address.Services()) {
            deleteServices.erase(service);
        }

        // write to database
        {
            const auto encodedID = id.asBase58(api_.Crypto());
            auto result = lmdb_.Store(
                Table::PeerDetails,
                encodedID,
                [&] {
                    auto proto = proto::BlockchainPeerAddress{};
                    address.Internal().Serialize(proto);

                    return proto::ToString(proto);
                }(),
                parentTxn);

            if (false == result.first) {
                LogError()()("Failed to save peer address").Flush();

                return false;
            }

            result = lmdb_.Store(
                Table::PeerChainIndex,
                static_cast<std::size_t>(address.Chain()),
                encodedID,
                parentTxn);

            if (false == result.first) {
                LogError()()("Failed to save peer chain index").Flush();

                return false;
            }

            result = lmdb_.Store(
                Table::PeerProtocolIndex,
                static_cast<std::size_t>(address.Style()),
                encodedID,
                parentTxn);

            if (false == result.first) {
                LogError()()("Failed to save peer protocol index").Flush();

                return false;
            }

            for (const auto& service : address.Services()) {
                result = lmdb_.Store(
                    Table::PeerServiceIndex,
                    static_cast<std::size_t>(service),
                    encodedID,
                    parentTxn);

                if (false == result.first) {
                    LogError()()("Failed to save peer service index").Flush();

                    return false;
                }
            }

            for (const auto& service : deleteServices) {
                result.first = lmdb_.Delete(
                    Table::PeerServiceIndex,
                    static_cast<std::size_t>(service),
                    encodedID,
                    parentTxn);
            }

            result = lmdb_.Store(
                Table::PeerNetworkIndex,
                static_cast<std::size_t>(address.Type()),
                encodedID,
                parentTxn);

            if (false == result.first) {
                LogError()()("Failed to save peer network index").Flush();

                return false;
            }

            result = lmdb_.Store(
                Table::PeerConnectedIndex,
                static_cast<std::size_t>(
                    Clock::to_time_t(address.LastConnected())),
                encodedID,
                parentTxn);

            if (false == result.first) {
                LogError()()("Failed to save peer network index").Flush();

                return false;
            }

            lmdb_.Delete(
                Table::PeerConnectedIndex,
                static_cast<std::size_t>(Clock::to_time_t(
                    address.Internal().PreviousLastConnected())),
                encodedID,
                parentTxn);
        }

        // Update in-memory indices to match database
        {
            data.chains_[address.Chain()].emplace(id);
            data.protocols_[address.Style()].emplace(id);
            data.networks_[address.Type()].emplace(id);

            for (const auto& service : address.Services()) {
                data.services_[service].emplace(id);
            }

            for (const auto& service : deleteServices) {
                data.services_[service].erase(id);
            }

            data.connected_[id] = address.LastConnected();
            auto handle = data.chain_index_[address.Chain()].lock();
            auto& index = *handle;

            if (false == exists(index, id)) { index.untested_.emplace(id); }
        }
    }

    if (false == parentTxn.Finalize(true)) {
        LogError()()("Database error").Flush();

        return false;
    }

    return true;
}

auto Peers::IsReady() const noexcept -> bool
{
    return opentxs::IsReady(future_);
}

auto Peers::last_connected(
    const network::blockchain::AddressID& id) const noexcept -> Time
{
    auto handle = get().lock_shared();
    const auto& data = *handle;
    const auto& map = data.connected_;

    if (auto i = map.find(id); map.end() != i) {

        return i->second;
    } else {

        return {};
    }
}

auto Peers::load_address(const network::blockchain::AddressID& id) noexcept(
    false) -> network::blockchain::Address
{
    auto output = std::optional<proto::BlockchainPeerAddress>{};
    lmdb_.Load(
        Table::PeerDetails,
        id.asBase58(api_.Crypto()),
        [&](const auto data) -> void {
            output = proto::Factory<proto::BlockchainPeerAddress>(
                data.data(), data.size());
        });

    if (false == output.has_value()) {

        throw std::out_of_range("Address not found");
    }

    const auto& serialized = output.value();

    if (false == proto::Validate(serialized, SILENT)) {

        throw std::out_of_range("Invalid address");
    }

    auto out = api_.Factory().Internal().BlockchainAddress(serialized);
    using enum network::blockchain::Transport;

    if ((out.Type() == zmq) && (out.Subtype() == invalid)) {

        throw std::out_of_range("invalid subtype");
    }

    return out;
}

auto Peers::next_timeout(
    Index& data,
    network::blockchain::AddressID id) noexcept -> sTime
{
    auto& timeout = [&]() -> auto& {
        auto& map = data.next_timeout_;

        if (auto i = map.find(id); map.end() != i) {

            return i->second;
        } else {
            constexpr auto initial = 30s;
            auto [j, _] = map.try_emplace(id, initial);

            return j->second;
        }
    }();
    const auto out = sClock::now() + timeout;
    constexpr auto max = 6h;
    timeout = std::min<std::chrono::seconds>(max, timeout * 2);

    return out;
}

auto Peers::Release(
    const blockchain::Type chain,
    const network::blockchain::AddressID& id) noexcept -> void
{
    auto handle = get().lock()->chain_index_[chain].lock();
    auto& data = *handle;
    data.in_use_.erase(id);
}

auto Peers::retry_peers(Index& data) noexcept -> void
{
    auto& retry = data.retry_;
    const auto stop = retry.lower_bound(sClock::now());

    for (auto i = retry.begin(); i != stop;) {
        for (const auto& id : i->second) {
            data.untested_.insert(data.failed_.extract(id));
        }

        i = retry.erase(i);
    }
}

Peers::~Peers() { get(); }
}  // namespace opentxs::blockchain::database::common
