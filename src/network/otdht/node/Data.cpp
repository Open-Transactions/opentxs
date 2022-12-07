// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "network/otdht/node/Shared.hpp"  // IWYU pragma: associated

#include <boost/json.hpp>
#include <boost/utility/string_view.hpp>
#include <zmq.h>
#include <array>
#include <fstream>
#include <stdexcept>
#include <utility>

#include "internal/api/Legacy.hpp"
#include "internal/api/session/Session.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/crypto/Encode.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Types.hpp"
#include "opentxs/util/Writer.hpp"

namespace opentxs::network::otdht
{
Node::Shared::Data::Data(const api::Session& api, allocator_type alloc) noexcept
    : state_(alloc)
    , private_key_(api.Factory().Secret(key_size_))
    , public_key_()
{
    load_config(api);
}

auto Node::Shared::Data::create_config(
    const api::Session& api,
    const std::filesystem::path& path) noexcept -> void
{
    static_assert(41_uz == encoded_buffer_size_);
    auto bufPublic = std::array<char, encoded_buffer_size_>{};
    auto bufSecret = std::array<char, encoded_buffer_size_>{};
    {
        auto rc = ::zmq_curve_keypair(bufPublic.data(), bufSecret.data());

        OT_ASSERT(0 == rc);
    }
    const auto pubkey = std::string_view{bufPublic.data(), encoded_key_size_};
    const auto seckey = std::string_view{bufSecret.data(), encoded_key_size_};
    // NOTE in some versions of Boost on some platforms boost::json::string_view
    // can be implicitly converted to and from std::string_view. In some
    // configurations they can't.
    const auto pubkeyJson =
        boost::json::string_view{bufPublic.data(), encoded_key_size_};
    const auto seckeyJson =
        boost::json::string_view{bufSecret.data(), encoded_key_size_};

    auto rc = api.Crypto().Encode().Z85Decode(pubkey, public_key_.WriteInto());

    OT_ASSERT(rc);

    rc = api.Crypto().Encode().Z85Decode(seckey, private_key_.WriteInto());

    OT_ASSERT(rc);

    const auto json = [&] {
        auto out = boost::json::object{};
        out[{seckey_json_key_.data(), seckey_json_key_.size()}] = seckeyJson;
        out[{pubkey_json_key_.data(), pubkey_json_key_.size()}] = pubkeyJson;

        return out;
    }();
    write_config(json, path);
}

auto Node::Shared::Data::get_allocator() const noexcept -> allocator_type
{
    return state_.get_allocator();
}

auto Node::Shared::Data::load_config(const api::Session& api) noexcept -> void
{
    const auto path = [&] {
        auto out = std::filesystem::path{};
        const auto& legacy = api.Internal().Legacy();
        const auto base = legacy.ClientDataFolder(api.Instance());
        legacy.AppendFile(out, base, "otdht.json");
        const auto rc = legacy.BuildFilePath(out);

        OT_ASSERT(rc);

        return out;
    }();

    if (std::filesystem::exists(path)) {
        LogConsole()("loading otdht config from ")(path).Flush();
        read_config(api, path);
    } else {
        LogConsole()("creating new otdht config at ")(path).Flush();
        create_config(api, path);
    }
}

auto Node::Shared::Data::read_config(
    const api::Session& api,
    const std::filesystem::path& path) noexcept -> void
{
    try {
        const auto json = [&] {
            auto file = std::filebuf{};
            constexpr auto mode = std::ios::binary | std::ios::in;

            if (nullptr == file.open(path.c_str(), mode)) {
                throw std::runtime_error{"failed to open input file"};
            }

            auto stream = std::istream{std::addressof(file)};
            auto parser = boost::json::stream_parser{};
            auto buf = std::array<char, 1024>{};

            do {
                const auto bytes = stream.read(buf.data(), buf.size()).gcount();
                parser.write(buf.data(), bytes);
            } while (false == stream.eof());

            file.close();

            return parser.release().as_object();
        }();
        auto bufSecret = std::array<char, encoded_buffer_size_>{};
        auto bufPublic = std::array<char, encoded_buffer_size_>{};

        {
            const auto& value =
                json.at({seckey_json_key_.data(), seckey_json_key_.size()})
                    .as_string();
            const auto view = boost::json::string_view{value};
            const auto rc = copy(
                {view.data(), view.size()},
                preallocated(bufSecret.size(), bufSecret.data()));

            OT_ASSERT(rc);
        }

        {
            const auto& value =
                json.at({pubkey_json_key_.data(), pubkey_json_key_.size()})
                    .as_string();
            const auto view = boost::json::string_view{value};
            const auto rc = copy(
                {view.data(), view.size()},
                preallocated(bufPublic.size(), bufPublic.data()));

            OT_ASSERT(rc);
        }

        auto rc = api.Crypto().Encode().Z85Decode(
            {bufSecret.data(), encoded_key_size_}, private_key_.WriteInto());

        OT_ASSERT(rc);

        rc = api.Crypto().Encode().Z85Decode(
            {bufPublic.data(), encoded_key_size_}, public_key_.WriteInto());

        OT_ASSERT(rc);
    } catch (const std::exception& e) {
        LogAbort()(OT_PRETTY_STATIC(Data))(e.what()).Abort();
    }
}

auto Node::Shared::Data::write_config(
    const boost::json::object& json,
    const std::filesystem::path& path) noexcept -> void
{
    try {
        auto file = std::filebuf{};
        constexpr auto mode =
            std::ios::binary | std::ios::out | std::ios::trunc;

        if (nullptr == file.open(path.c_str(), mode)) {
            throw std::runtime_error{"failed to open output file"};
        }

        {
            auto stream = std::ostream{std::addressof(file)};
            opentxs::print(json, stream);
        }

        file.close();
    } catch (const std::exception& e) {
        LogAbort()(OT_PRETTY_STATIC(Data))(e.what()).Abort();
    }
}

Node::Shared::Data::~Data() = default;
}  // namespace opentxs::network::otdht
