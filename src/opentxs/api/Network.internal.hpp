// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <filesystem>
#include <memory>

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace crypto
{
class Blockchain;
}  // namespace crypto

namespace internal
{
class Network;  // IWYU pragma: keep
class Paths;
}  // namespace internal

namespace network
{
class Asio;
class Blockchain;
class OTDHT;
class ZeroMQ;
}  // namespace network

namespace session
{
namespace internal
{
class Client;
class Notary;
}  // namespace internal
}  // namespace session
}  // namespace api

class Options;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

class opentxs::api::internal::Network
{
public:
    virtual auto Asio() const noexcept -> const network::Asio& = 0;
    virtual auto Blockchain() const noexcept -> const network::Blockchain& = 0;
    virtual auto OTDHT() const noexcept -> const network::OTDHT& = 0;
    virtual auto ZeroMQ() const noexcept -> const network::ZeroMQ& = 0;

    virtual auto Shutdown() noexcept -> void = 0;
    virtual auto Start(
        std::shared_ptr<const api::session::internal::Client> api,
        const api::crypto::Blockchain& crypto,
        const api::internal::Paths& legacy,
        const std::filesystem::path& dataFolder,
        const Options& args) noexcept -> void = 0;
    virtual auto Start(
        std::shared_ptr<const api::session::internal::Notary> api,
        const api::crypto::Blockchain& crypto,
        const api::internal::Paths& legacy,
        const std::filesystem::path& dataFolder,
        const Options& args) noexcept -> void = 0;

    virtual ~Network() = default;
};
