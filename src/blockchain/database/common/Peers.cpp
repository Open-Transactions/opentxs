// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                          // IWYU pragma: associated
#include "blockchain/database/common/Peers.hpp"  // IWYU pragma: associated

#include <BlockchainPeerAddress.pb.h>
#include <algorithm>
#include <chrono>
#include <cstring>
#include <iterator>
#include <optional>
#include <random>
#include <stdexcept>
#include <utility>

#include "internal/blockchain/p2p/P2P.hpp"
#include "internal/serialization/protobuf/Check.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "internal/serialization/protobuf/Proto.tpp"
#include "internal/serialization/protobuf/verify/BlockchainPeerAddress.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "internal/util/Time.hpp"
#include "internal/util/storage/lmdb/Database.hpp"
#include "internal/util/storage/lmdb/Transaction.hpp"
#include "internal/util/storage/lmdb/Types.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/p2p/Address.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::blockchain::database::common
{
template <typename Index, typename Map>
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
    map[static_cast<Index>(input)].emplace(
        api_.Factory().IdentifierFromBase58(value));

    return true;
}
}  // namespace opentxs::blockchain::database::common

namespace opentxs::blockchain::database::common
{
Peers::Peers(const api::Session& api, storage::lmdb::Database& lmdb) noexcept(
    false)
    : api_(api)
    , lmdb_(lmdb)
    , lock_()
    , chains_()
    , protocols_()
    , services_()
    , networks_()
    , connected_()
{
    using Dir = storage::lmdb::Dir;

    auto chain = [this](const auto key, const auto value) {
        return read_index<Chain>(key, value, chains_);
    };
    auto protocol = [this](const auto key, const auto value) {
        return read_index<Protocol>(key, value, protocols_);
    };
    auto service = [this](const auto key, const auto value) {
        return read_index<Service>(key, value, services_);
    };
    auto type = [this](const auto key, const auto value) {
        return read_index<Type>(key, value, networks_);
    };
    auto last = [this](const auto key, const auto value) {
        auto input = 0_uz;

        if (sizeof(input) != key.size()) {
            throw std::runtime_error("Invalid key");
        }

        std::memcpy(&input, key.data(), key.size());
        connected_.emplace(
            api_.Factory().IdentifierFromBase58(value), convert_stime(input));

        return true;
    };

    lmdb_.Read(PeerChainIndex, chain, Dir::Forward);
    lmdb_.Read(PeerProtocolIndex, protocol, Dir::Forward);
    lmdb_.Read(PeerServiceIndex, service, Dir::Forward);
    lmdb_.Read(PeerNetworkIndex, type, Dir::Forward);
    lmdb_.Read(PeerConnectedIndex, last, Dir::Forward);
}

auto Peers::Find(
    const blockchain::Type chain,
    const p2p::Protocol protocol,
    const Set<p2p::Network>& onNetworks,
    const Set<p2p::Service>& withServices,
    const Set<identifier::Generic>& exclude) const noexcept -> p2p::Address
{
    Lock lock(lock_);

    try {
        auto candidates = Addresses{};
        const auto& protocolSet = protocols_.at(protocol);
        const auto& chainSet = chains_.at(chain);

        if (protocolSet.empty()) { return {}; }
        if (chainSet.empty()) { return {}; }

        for (const auto& network : onNetworks) {
            try {
                for (const auto& id : networks_.at(network)) {
                    if (false == chainSet.contains(id)) { continue; }
                    if (false == protocolSet.contains(id)) { continue; }
                    if (exclude.contains(id)) { continue; }

                    candidates.emplace(id);
                }
            } catch (...) {
            }
        }

        if (candidates.empty()) {
            LogTrace()(OT_PRETTY_CLASS())(
                "No peers available for specified chain/protocol")
                .Flush();

            return {};
        }

        auto haveServices = Addresses{};

        if (withServices.empty()) {
            haveServices = candidates;
        } else {
            for (const auto& id : candidates) {
                bool haveAllServices{true};

                for (const auto& service : withServices) {
                    try {
                        if (0 == services_.at(service).count(id)) {
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
            LogTrace()(OT_PRETTY_CLASS())(
                "No peers available with specified services")
                .Flush();

            return {};
        } else {
            LogTrace()(OT_PRETTY_CLASS())("Choosing from ")(
                haveServices.size())(" candidates")
                .Flush();
        }

        auto weighted = Vector<AddressID>{};
        const auto now = Clock::now();

        for (const auto& id : haveServices) {
            auto weight = 1_uz;

            try {
                const auto& last = connected_.at(id);
                const auto since =
                    std::chrono::duration_cast<std::chrono::hours>(now - last);

                if (since.count() <= 1) {
                    weight = 10;
                } else if (since.count() <= 24) {
                    weight = 5;
                }
            } catch (...) {
            }

            weighted.insert(weighted.end(), weight, id);
        }

        auto output = Vector<AddressID>{};
        constexpr auto count = 1_uz;
        std::sample(
            weighted.begin(),
            weighted.end(),
            std::back_inserter(output),
            count,
            std::mt19937{std::random_device{}()});

        OT_ASSERT(count == output.size());

        LogTrace()(OT_PRETTY_CLASS())("Loading peer ")(output.front()).Flush();

        return load_address(output.front());
    } catch (...) {

        return {};
    }
}

auto Peers::Import(Vector<p2p::Address>&& peers) noexcept -> bool
{
    auto newPeers = Vector<p2p::Address>{};

    for (auto& peer : peers) {
        if (false == peer.IsValid()) { continue; }

        if (false ==
            lmdb_.Exists(
                Table::PeerDetails, peer.ID().asBase58(api_.Crypto()))) {
            newPeers.emplace_back(std::move(peer));
        }
    }

    Lock lock(lock_);

    return insert(lock, newPeers);
}

auto Peers::Insert(p2p::Address address) noexcept -> bool
{
    if (false == address.IsValid()) { return false; }

    auto peers = Vector<p2p::Address>{};
    peers.emplace_back(std::move(address));
    Lock lock(lock_);

    return insert(lock, peers);
}

auto Peers::insert(const Lock& lock, const Vector<p2p::Address>& peers) noexcept
    -> bool
{
    auto parentTxn = lmdb_.TransactionRW();

    for (const auto& address : peers) {
        OT_ASSERT(address.IsValid());

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
                LogError()(OT_PRETTY_CLASS())("Failed to save peer address")
                    .Flush();

                return false;
            }

            result = lmdb_.Store(
                Table::PeerChainIndex,
                static_cast<std::size_t>(address.Chain()),
                encodedID,
                parentTxn);

            if (false == result.first) {
                LogError()(OT_PRETTY_CLASS())("Failed to save peer chain index")
                    .Flush();

                return false;
            }

            result = lmdb_.Store(
                Table::PeerProtocolIndex,
                static_cast<std::size_t>(address.Style()),
                encodedID,
                parentTxn);

            if (false == result.first) {
                LogError()(OT_PRETTY_CLASS())(
                    "Failed to save peer protocol index")
                    .Flush();

                return false;
            }

            for (const auto& service : address.Services()) {
                result = lmdb_.Store(
                    Table::PeerServiceIndex,
                    static_cast<std::size_t>(service),
                    encodedID,
                    parentTxn);

                if (false == result.first) {
                    LogError()(OT_PRETTY_CLASS())(
                        "Failed to save peer service index")
                        .Flush();

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
                LogError()(OT_PRETTY_CLASS())(
                    "Failed to save peer network index")
                    .Flush();

                return false;
            }

            result = lmdb_.Store(
                Table::PeerConnectedIndex,
                static_cast<std::size_t>(
                    Clock::to_time_t(address.LastConnected())),
                encodedID,
                parentTxn);

            if (false == result.first) {
                LogError()(OT_PRETTY_CLASS())(
                    "Failed to save peer network index")
                    .Flush();

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
            chains_[address.Chain()].emplace(id);
            protocols_[address.Style()].emplace(id);
            networks_[address.Type()].emplace(id);

            for (const auto& service : address.Services()) {
                services_[service].emplace(id);
            }

            for (const auto& service : deleteServices) {
                services_[service].erase(id);
            }

            connected_[id] = address.LastConnected();
        }
    }

    if (false == parentTxn.Finalize(true)) {
        LogError()(OT_PRETTY_CLASS())("Database error").Flush();

        return false;
    }

    return true;
}

auto Peers::load_address(const AddressID& id) const noexcept(false)
    -> p2p::Address
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
        LogError()(OT_PRETTY_CLASS())("Peer ")(id)(" not found").Flush();

        throw std::out_of_range("Address not found");
    }

    const auto& serialized = output.value();

    if (false == proto::Validate(serialized, SILENT)) {
        LogError()(OT_PRETTY_CLASS())("Peer ")(id)(" invalid").Flush();

        throw std::out_of_range("Invalid address");
    }

    return factory::BlockchainAddress(api_, serialized);
}
}  // namespace opentxs::blockchain::database::common
