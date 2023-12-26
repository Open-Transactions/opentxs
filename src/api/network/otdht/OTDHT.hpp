// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cs_plain_guarded.h>
#include <filesystem>
#include <memory>
#include <optional>
#include <string_view>

#include "internal/api/network/OTDHT.hpp"
#include "internal/network/zeromq/socket/Raw.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/core/FixedByteArray.hpp"
#include "opentxs/core/Secret.hpp"  // IWYU pragma: keep
#include "opentxs/util/Allocator.hpp"

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
namespace internal
{
class Session;
}  // namespace internal

namespace network
{
class Blockchain;
}  // namespace network

namespace session
{
class Endpoints;
}  // namespace session
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
    auto CurvePublicKey() const noexcept -> ReadView final;
    auto DeletePeer(std::string_view endpoint) const noexcept -> bool final;
    auto KnownPeers(alloc::Default alloc) const noexcept -> Endpoints final;
    auto StartListener(
        std::string_view syncEndpoint,
        std::string_view publicSyncEndpoint,
        std::string_view updateEndpoint,
        std::string_view publicUpdateEndpoint) const noexcept -> bool final;

    auto Start(std::shared_ptr<const api::internal::Session> api) noexcept
        -> void final;

    OTDHT(
        const api::internal::Session& api,
        const opentxs::network::zeromq::Context& zmq,
        const api::session::Endpoints& endpoints,
        const api::network::Blockchain& blockchain) noexcept;
    OTDHT() = delete;
    OTDHT(const OTDHT&) = delete;
    OTDHT(OTDHT&&) = delete;
    auto operator=(const OTDHT&) -> OTDHT& = delete;
    auto operator=(OTDHT&&) -> OTDHT& = delete;

    ~OTDHT() final;

private:
    static constexpr auto key_size_ = 32_uz;
    static constexpr auto encoded_key_size_ = key_size_ * 5_uz / 4_uz;
    static constexpr auto encoded_buffer_size_ = encoded_key_size_ + 1_uz;
    static constexpr auto seckey_json_key_ = "curve_secret_key"sv;
    static constexpr auto pubkey_json_key_ = "curve_public_key"sv;

    struct Data {
        opentxs::network::zeromq::socket::Raw to_node_;
        std::optional<Secret> private_key_;
        std::optional<FixedByteArray<key_size_>> public_key_;
        bool init_;

        Data(
            const opentxs::network::zeromq::Context& zmq,
            const api::session::Endpoints& endpoints) noexcept;
    };

    using Guarded = libguarded::plain_guarded<Data>;

    const api::internal::Session& api_;
    const api::network::Blockchain& blockchain_;
    mutable Guarded data_;

    static auto write_config(
        const boost::json::object& json,
        const std::filesystem::path& path) noexcept -> void;

    auto create_config(const std::filesystem::path& path, Data& data)
        const noexcept -> void;
    auto get_data() const noexcept -> Guarded::handle;
    auto load_config(Data& data) const noexcept -> void;
    auto read_config(const std::filesystem::path& path, Data& data)
        const noexcept -> void;
};
}  // namespace opentxs::api::network::implementation
