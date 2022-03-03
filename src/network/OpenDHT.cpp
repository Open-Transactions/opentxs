// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"         // IWYU pragma: associated
#include "1_Internal.hpp"       // IWYU pragma: associated
#include "network/OpenDHT.hpp"  // IWYU pragma: associated

#include <opendht.h>
#include <cstdint>
#include <iostream>
#include <stdexcept>

#include "internal/network/Factory.hpp"
#include "internal/util/LogMacros.hpp"
#include "network/DhtConfig.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::factory
{
auto OpenDHT(const network::DhtConfig& config) noexcept
    -> std::unique_ptr<network::OpenDHT>
{
    using ReturnType = network::implementation::OpenDHT;

    return std::make_unique<ReturnType>(config);
}
}  // namespace opentxs::factory

namespace opentxs::network::implementation
{
OpenDHT::OpenDHT(const DhtConfig& config) noexcept
    : config_(config)
    , node_()
    , loaded_(Flag::Factory(false))
    , ready_(Flag::Factory(false))
    , init_()
{
}

auto OpenDHT::Init() const -> bool
{
    if (ready_.get()) { return true; }
    if (false == config_.enable_dht_) { return false; }

    Lock lock(init_);

    if (false == bool(node_)) {
        try {
            const_cast<Pointer&>(node_) = std::make_unique<dht::DhtRunner>();
        } catch (...) {

            return false;
        }
    }

    OT_ASSERT(node_);

    auto listenPort = config_.listen_port_;

    if ((listenPort <= 1000) || (listenPort >= 65535)) {
        listenPort = config_.default_port_;
    }

    if (false == loaded_.get()) {
        try {
            node_->run(
                static_cast<unsigned short>(listenPort),
                dht::crypto::generateIdentity(),
                true);
        } catch (const std::exception& e) {
            std::cout << e.what() << std::endl;

            return false;
        } catch (...) {
            std::cout << "opendht error" << std::endl;

            return false;
        }

        loaded_->On();
    }

    try {
        node_->bootstrap(
            config_.bootstrap_url_.c_str(), config_.bootstrap_port_.c_str());
        ready_->On();
    } catch (const std::exception& e) {
        std::cout << e.what() << std::endl;

        return false;
    } catch (...) {
        std::cout << "opendht error" << std::endl;

        return false;
    }

    return true;
}

auto OpenDHT::Insert(
    const UnallocatedCString& key,
    const UnallocatedCString& value,
    DhtDoneCallback cb) const noexcept -> void
{
    if (false == Init()) { return; }

    try {
        auto infoHash = dht::InfoHash::get(
            reinterpret_cast<const std::uint8_t*>(key.c_str()), key.size());
        auto pValue = std::make_shared<dht::Value>(
            reinterpret_cast<const std::uint8_t*>(value.c_str()), value.size());

        if (false == bool(pValue)) { return; }

        if (value.size() > dht::MAX_VALUE_SIZE) {
            LogError()(OT_PRETTY_CLASS())(
                "Error: data size exceeds DHT limits.")
                .Flush();
            return;
        }

        node_->put(infoHash, pValue, cb);
    } catch (const std::exception& e) {
        std::cout << e.what() << std::endl;
    } catch (...) {
        std::cout << "opendht error" << std::endl;
    }
}

auto OpenDHT::Retrieve(
    const UnallocatedCString& key,
    DhtResultsCallback vcb,
    DhtDoneCallback dcb) const noexcept -> void
{
    if (false == Init()) { return; }

    try {
        // The OpenDHT get method wants a lambda function that accepts an
        // OpenDHT-specific type as an argument. I don't consumers of this class
        // to need to include OpenDHT headers. Solution: this lambda performs
        // the translation
        dht::GetCallback cb(
            [vcb](const UnallocatedVector<std::shared_ptr<dht::Value>> results)
                -> bool {
                DhtResults input;

                for (const auto& it : results) {
                    if (nullptr == it) { continue; }

                    auto& bytes = input.emplace_back(
                        std::make_shared<UnallocatedCString>());

                    OT_ASSERT(bytes);

                    copy(reader(it->data), writer(*bytes));
                }

                return vcb(input);
            });
        auto infoHash = dht::InfoHash::get(
            reinterpret_cast<const std::uint8_t*>(key.c_str()), key.size());
        node_->get(infoHash, cb, dcb, dht::Value::AllFilter());
    } catch (const std::exception& e) {
        std::cout << e.what() << std::endl;
    } catch (...) {
        std::cout << "opendht error" << std::endl;
    }
}

OpenDHT::~OpenDHT()
{
    if (node_) { node_->join(); }
}
}  // namespace opentxs::network::implementation
