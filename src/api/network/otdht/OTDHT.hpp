// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cs_plain_guarded.h>
#include <filesystem>
#include <memory>
#include <string_view>

#include "internal/api/network/OTDHT.hpp"
#include "internal/network/zeromq/socket/Raw.hpp"
#include "internal/util/AsyncConst.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/core/FixedByteArray.hpp"
#include "opentxs/core/Secret.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace boost
{
namespace json
{
class object;
}  // namespace json
}  // namespace boost

namespace opentxs
{
namespace api
{
namespace network
{
class Blockchain;
}  // namespace network

namespace session
{
class Endpoints;
}  // namespace session

class Session;
}  // namespace api

namespace network
{

namespace zeromq
{
class Context;
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::api::network::implementation
{
using namespace std::literals;

class OTDHT final : public internal::OTDHT
{
public:
    auto AddPeer(std::string_view endpoint) const noexcept -> bool final;
    auto CurvePublicKey() const noexcept -> ReadView final
    {
        return public_key_.get().Bytes();
    }
    auto DeletePeer(std::string_view endpoint) const noexcept -> bool final;
    auto KnownPeers(alloc::Default alloc) const noexcept -> Endpoints final;
    auto StartListener(
        std::string_view syncEndpoint,
        std::string_view publicSyncEndpoint,
        std::string_view updateEndpoint,
        std::string_view publicUpdateEndpoint) const noexcept -> bool final;

    auto Start(std::shared_ptr<const api::Session> api) noexcept -> void final;

    OTDHT(
        const api::Session& api,
        const opentxs::network::zeromq::Context& zmq,
        const api::session::Endpoints& endpoints,
        const api::network::Blockchain& blockchain)
    noexcept;
    OTDHT() = delete;
    OTDHT(const OTDHT&) = delete;
    OTDHT(OTDHT&&) = delete;
    auto operator=(const OTDHT&) -> OTDHT& = delete;
    auto operator=(OTDHT&&) -> OTDHT& = delete;

    ~OTDHT() final;

private:
    using GuardedSocket =
        libguarded::plain_guarded<opentxs::network::zeromq::socket::Raw>;

    static constexpr auto key_size_ = 32_uz;
    static constexpr auto encoded_key_size_ = key_size_ * 5_uz / 4_uz;
    static constexpr auto encoded_buffer_size_ = encoded_key_size_ + 1_uz;
    static constexpr auto seckey_json_key_ = "curve_secret_key"sv;
    static constexpr auto pubkey_json_key_ = "curve_public_key"sv;

    const api::Session& api_;
    const api::network::Blockchain& blockchain_;
    AsyncConst<Secret> private_key_;
    AsyncConst<FixedByteArray<key_size_>> public_key_;
    mutable GuardedSocket to_node_;

    static auto write_config(
        const boost::json::object& json,
        const std::filesystem::path& path) noexcept -> void;

    auto create_config(
        const api::Session& api,
        const std::filesystem::path& path) noexcept -> void;
    auto load_config(const api::Session& api) noexcept -> void;
    auto read_config(
        const api::Session& api,
        const std::filesystem::path& path) noexcept -> void;
};
}  // namespace opentxs::api::network::implementation
