// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/api/NetworkPrivate.hpp"  // IWYU pragma: associated

#include <memory>
#include <utility>

#include "internal/api/network/Blockchain.hpp"
#include "internal/api/network/OTDHT.hpp"
#include "opentxs/api/network/Blockchain.hpp"
#include "opentxs/api/network/internal.factory.hpp"
#include "opentxs/api/session/Client.internal.hpp"
#include "opentxs/api/session/Notary.internal.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::api
{
NetworkPrivate::NetworkPrivate(
    const api::internal::Session& api,
    const network::Asio& asio,
    const opentxs::network::zeromq::Context& zmq,
    const network::ZAP& zap,
    const api::session::Endpoints& endpoints,
    std::unique_ptr<api::network::Blockchain> blockchain) noexcept
    : asio_(asio)
    , zmq_(factory::NetworkZMQ(zmq, zap))
    , blockchain_(std::move(blockchain))
    , otdht_(factory::OTDHT(api, zmq, endpoints, *blockchain_))
{
    assert_false(nullptr == blockchain_);
    assert_false(nullptr == otdht_);
}

auto NetworkPrivate::Start(
    std::shared_ptr<const api::session::internal::Client> api,
    const api::crypto::Blockchain& crypto,
    const api::internal::Paths& legacy,
    const std::filesystem::path& dataFolder,
    const Options& args) noexcept -> void
{
    blockchain_->Internal().Init(api, crypto, legacy, dataFolder, args);
    otdht_->Internal().Start(api);
}

auto NetworkPrivate::Start(
    std::shared_ptr<const api::session::internal::Notary> api,
    const api::crypto::Blockchain& crypto,
    const api::internal::Paths& legacy,
    const std::filesystem::path& dataFolder,
    const Options& args) noexcept -> void
{
    otdht_->Internal().Start(api);
}

auto NetworkPrivate::Shutdown() noexcept -> void
{
    blockchain_->Internal().Shutdown();
}

NetworkPrivate::~NetworkPrivate() = default;
}  // namespace opentxs::api
