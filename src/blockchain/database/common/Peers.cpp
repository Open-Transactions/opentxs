// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                          // IWYU pragma: associated
#include "1_Internal.hpp"                        // IWYU pragma: associated
#include "blockchain/database/common/Peers.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <chrono>
#include <iterator>
#include <memory>
#include <optional>
#include <random>
#include <utility>

#include "Proto.tpp"
#include "internal/blockchain/p2p/P2P.hpp"
#include "internal/protobuf/Check.hpp"
#include "internal/protobuf/verify/BlockchainPeerAddress.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/blockchain/p2p/Address.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/util/Log.hpp"
#include "serialization/protobuf/BlockchainPeerAddress.pb.h"
#include "util/LMDB.hpp"

namespace opentxs::blockchain::database::common
{
Peers::Peers(const api::Session& api, storage::lmdb::LMDB& lmdb) noexcept(false)
    : api_(api)
    , lmdb_(lmdb)
    , lock_()
    , chains_()
    , protocols_()
    , services_()
    , networks_()
    , connected_()
{
    using Dir = storage::lmdb::LMDB::Dir;

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
        auto input = std::size_t{};

        if (sizeof(input) != key.size()) {
            throw std::runtime_error("Invalid key");
        }

        std::memcpy(&input, key.data(), key.size());
        connected_.emplace(value, Clock::from_time_t(input));

        return true;
    };

    lmdb_.Read(PeerChainIndex, chain, Dir::Forward);
    lmdb_.Read(PeerProtocolIndex, protocol, Dir::Forward);
    lmdb_.Read(PeerServiceIndex, service, Dir::Forward);
    lmdb_.Read(PeerNetworkIndex, type, Dir::Forward);
    lmdb_.Read(PeerConnectedIndex, last, Dir::Forward);
}

auto Peers::Find(
    const Chain chain,
    const Protocol protocol,
    const std::pmr::set<Type> onNetworks,
    const std::pmr::set<Service> withServices) const noexcept -> Address_p
{
    Lock lock(lock_);

    try {
        auto candidates = std::pmr::set<std::string>{};
        const auto& protocolSet = protocols_.at(protocol);
        const auto& chainSet = chains_.at(chain);

        if (protocolSet.empty()) { return {}; }
        if (chainSet.empty()) { return {}; }

        for (const auto& network : onNetworks) {
            try {
                for (const auto& id : networks_.at(network)) {
                    if ((1 == chainSet.count(id)) &&
                        (1 == protocolSet.count(id))) {
                        candidates.emplace(id);
                    }
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

        auto haveServices = std::pmr::set<std::string>{};

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

        auto weighted = std::pmr::vector<std::string>{};
        const auto now = Clock::now();

        for (const auto& id : haveServices) {
            auto weight = std::size_t{1};

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

        std::pmr::vector<std::string> output;
        const std::size_t count{1};
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

auto Peers::Import(std::pmr::vector<Address_p> peers) noexcept -> bool
{
    auto newPeers = std::pmr::vector<Address_p>{};

    for (auto& peer : peers) {
        if (false == lmdb_.Exists(Table::PeerDetails, peer->ID().str())) {
            newPeers.emplace_back(std::move(peer));
        }
    }

    Lock lock(lock_);

    return insert(lock, std::move(newPeers));
}

auto Peers::Insert(Address_p pAddress) noexcept -> bool
{
    auto peers = std::pmr::vector<Address_p>{};
    peers.emplace_back(std::move(pAddress));
    Lock lock(lock_);

    return insert(lock, std::move(peers));
}

auto Peers::insert(const Lock& lock, std::pmr::vector<Address_p> peers) noexcept
    -> bool
{
    auto parentTxn = lmdb_.TransactionRW();

    for (auto& pAddress : peers) {
        if (false == bool(pAddress)) {
            LogError()(OT_PRETTY_CLASS())("Invalid peer").Flush();

            return false;
        }

        auto& address = *pAddress;
        const auto id = address.ID().str();
        auto deleteServices = address.PreviousServices();

        for (const auto& service : address.Services()) {
            deleteServices.erase(service);
        }

        // write to database
        {
            auto result = lmdb_.Store(
                Table::PeerDetails,
                id,
                [&] {
                    auto proto =
                        opentxs::blockchain::p2p::Address::SerializedType{};
                    address.Serialize(proto);

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
                id,
                parentTxn);

            if (false == result.first) {
                LogError()(OT_PRETTY_CLASS())("Failed to save peer chain index")
                    .Flush();

                return false;
            }

            result = lmdb_.Store(
                Table::PeerProtocolIndex,
                static_cast<std::size_t>(address.Style()),
                id,
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
                    id,
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
                    id,
                    parentTxn);
            }

            result = lmdb_.Store(
                Table::PeerNetworkIndex,
                static_cast<std::size_t>(address.Type()),
                id,
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
                id,
                parentTxn);

            if (false == result.first) {
                LogError()(OT_PRETTY_CLASS())(
                    "Failed to save peer network index")
                    .Flush();

                return false;
            }

            lmdb_.Delete(
                Table::PeerConnectedIndex,
                static_cast<std::size_t>(
                    Clock::to_time_t(address.PreviousLastConnected())),
                id,
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

auto Peers::load_address(const std::string& id) const noexcept(false)
    -> Address_p
{
    auto output = std::optional<Address::SerializedType>{};
    lmdb_.Load(Table::PeerDetails, id, [&](const auto data) -> void {
        output =
            proto::Factory<Address::SerializedType>(data.data(), data.size());
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
