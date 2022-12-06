// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::blockchain::Type
// IWYU pragma: no_include "opentxs/blockchain/BlockchainType.hpp"

#pragma once

#include <boost/smart_ptr/shared_ptr.hpp>
#include <cs_shared_guarded.h>
#include <filesystem>
#include <shared_mutex>
#include <string_view>

#include "internal/network/otdht/Node.hpp"
#include "internal/network/zeromq/Types.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/core/FixedByteArray.hpp"
#include "opentxs/core/Secret.hpp"
#include "opentxs/util/Allocated.hpp"
#include "opentxs/util/Container.hpp"
#include "util/Allocated.hpp"

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
class Session;
}  // namespace api

namespace blockchain
{
namespace block
{
class Position;
}  // namespace block
}  // namespace blockchain
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::network::otdht
{
using namespace std::literals;

class Node::Shared final : public opentxs::implementation::Allocated
{
public:
    class Data final : opentxs::Allocated
    {
    public:
        static constexpr auto key_size_ = 32_uz;

        Map<opentxs::blockchain::Type, opentxs::blockchain::block::Position>
            state_;
        Secret private_key_;
        FixedByteArray<key_size_> public_key_;

        auto get_allocator() const noexcept -> allocator_type final;

        Data(const api::Session& api, allocator_type alloc) noexcept;
        Data() = delete;
        Data(const Data&) = delete;
        Data(Data&&) = delete;
        auto operator=(const Data&) -> Data& = delete;
        auto operator=(Data&&) -> Data& = delete;

        ~Data() final;

    private:
        static constexpr auto encoded_key_size_ = key_size_ * 5_uz / 4_uz;
        static constexpr auto encoded_buffer_size_ = encoded_key_size_ + 1_uz;
        static constexpr auto seckey_json_key_ = "curve_secret_key"sv;
        static constexpr auto pubkey_json_key_ = "curve_public_key"sv;

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

    using Guarded = libguarded::shared_guarded<Data, std::shared_mutex>;

    const zeromq::BatchID batch_id_;
    mutable Guarded data_;

    static auto Chains() noexcept -> const Set<opentxs::blockchain::Type>&;

    Shared(
        const api::Session& api,
        zeromq::BatchID batchID,
        allocator_type alloc) noexcept;
    Shared() = delete;
    Shared(const Shared&) = delete;
    Shared(Shared&&) = delete;
    auto operator=(const Shared&) -> Shared& = delete;
    auto operator=(Shared&&) -> Shared& = delete;

    ~Shared() final;
};
}  // namespace opentxs::network::otdht
