// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::Data

#include "api/network/otdht/OTDHT.hpp"               // IWYU pragma: associated
#include "opentxs/api/network/internal.factory.hpp"  // IWYU pragma: associated

#include <boost/json.hpp>
#include <array>
#include <fstream>
#include <stdexcept>
#include <utility>

#include "internal/api/network/Blockchain.hpp"
#include "internal/api/session/Endpoints.hpp"
#include "internal/network/otdht/Node.hpp"
#include "internal/network/zeromq/Context.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/WorkType.internal.hpp"
#include "opentxs/api/Paths.internal.hpp"
#include "opentxs/api/Session.internal.hpp"
#include "opentxs/api/crypto/Encode.hpp"
#include "opentxs/api/network/Blockchain.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Endpoints.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/core/Secret.hpp"
#include "opentxs/network/otdht/Types.internal.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/Types.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/network/zeromq/socket/SocketType.hpp"  // IWYU pragma: keep
#include "opentxs/network/zeromq/socket/Types.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Options.hpp"
#include "opentxs/util/Writer.hpp"

namespace opentxs::factory
{
auto OTDHT(
    const api::internal::Session& api,
    const network::zeromq::Context& zmq,
    const api::session::Endpoints& endpoints,
    const api::network::Blockchain& blockchain) noexcept
    -> std::unique_ptr<api::network::OTDHT>
{
    using ReturnType = api::network::implementation::OTDHT;

    return std::make_unique<ReturnType>(api, zmq, endpoints, blockchain);
}
}  // namespace opentxs::factory

namespace opentxs::api::network::implementation
{
OTDHT::Data::Data(
    const opentxs::network::zeromq::Context& zmq,
    const api::session::Endpoints& endpoints) noexcept
    : to_node_([&] {
        auto out = zmq.Internal().RawSocket(
            opentxs::network::zeromq::socket::Type::Push);
        const auto rc =
            out.Connect(endpoints.Internal().OTDHTNodePull().data());

        assert_true(rc);

        return out;
    }())
    , private_key_()
    , public_key_()
    , init_(false)
{
}
}  // namespace opentxs::api::network::implementation

namespace opentxs::api::network::implementation
{
OTDHT::OTDHT(
    const api::internal::Session& api,
    const opentxs::network::zeromq::Context& zmq,
    const api::session::Endpoints& endpoints,
    const api::network::Blockchain& blockchain) noexcept
    : api_(api)
    , blockchain_(blockchain)
    , data_(zmq, endpoints)
{
}

auto OTDHT::AddPeer(std::string_view endpoint) const noexcept -> bool
{
    if (api_.GetOptions().LoopbackDHT()) {

        return true;
    } else {

        return blockchain_.Internal().AddSyncServer(endpoint);
    }
}

auto OTDHT::create_config(const std::filesystem::path& path, Data& data)
    const noexcept -> void
{
    static_assert(41_uz == encoded_buffer_size_);
    auto bufPublic = std::array<char, encoded_buffer_size_>{};
    auto bufSecret = std::array<char, encoded_buffer_size_>{};
    {
        const auto rc = opentxs::network::zeromq::CurveKeypairZ85(
            preallocated(bufSecret.size(), bufSecret.data()),
            preallocated(bufPublic.size(), bufPublic.data()));

        assert_true(rc);
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
    data.public_key_.emplace([&] {
        auto out = decltype(data.public_key_)::value_type{};
        const auto rc =
            api_.Crypto().Encode().Z85Decode(pubkey, out.WriteInto());

        assert_true(rc);

        return out;
    }());
    data.private_key_.emplace([&] {
        auto out = api_.Factory().Secret(key_size_);
        const auto rc =
            api_.Crypto().Encode().Z85Decode(seckey, out.WriteInto());

        assert_true(rc);

        return out;
    }());
    const auto json = [&] {
        auto out = boost::json::object{};
        out[{seckey_json_key_.data(), seckey_json_key_.size()}] = seckeyJson;
        out[{pubkey_json_key_.data(), pubkey_json_key_.size()}] = pubkeyJson;

        return out;
    }();
    write_config(json, path);
}

auto OTDHT::CurvePublicKey() const noexcept -> ReadView
{
    return get_data()->public_key_->Bytes();
}

auto OTDHT::DeletePeer(std::string_view endpoint) const noexcept -> bool
{
    if (api_.GetOptions().LoopbackDHT()) {

        return true;
    } else {

        return blockchain_.Internal().DeleteSyncServer(endpoint);
    }
}

auto OTDHT::get_data() const noexcept -> Guarded::handle
{
    auto handle = data_.lock();
    auto& data = *handle;

    if (false == data.init_) { load_config(data); }

    assert_true(data.init_);

    return handle;
}

auto OTDHT::KnownPeers(alloc::Default alloc) const noexcept -> Endpoints
{
    static const auto loopback = Endpoints{
        "tcp://127.0.0.1:8814",
    };

    if (api_.GetOptions().LoopbackDHT()) {

        return Endpoints{loopback, alloc};
    } else {

        return blockchain_.Internal().GetSyncServers(alloc);
    }
}

auto OTDHT::load_config(Data& data) const noexcept -> void
{
    const auto path = [&] {
        auto out = std::filesystem::path{};
        const auto& paths = api_.Paths();
        const auto base = paths.ClientDataFolder(api_.Instance());
        paths.AppendFile(out, base, "otdht.json");
        const auto rc = paths.BuildFilePath(out);

        assert_true(rc);

        return out;
    }();

    if (std::filesystem::exists(path)) {
        LogConsole()("loading otdht config from ")(path).Flush();
        read_config(path, data);
    } else {
        LogConsole()("creating new otdht config at ")(path).Flush();
        create_config(path, data);
    }

    data.init_ = true;
}

auto OTDHT::read_config(const std::filesystem::path& path, Data& data)
    const noexcept -> void
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

            assert_true(rc);
        }

        {
            const auto& value =
                json.at({pubkey_json_key_.data(), pubkey_json_key_.size()})
                    .as_string();
            const auto view = boost::json::string_view{value};
            const auto rc = copy(
                {view.data(), view.size()},
                preallocated(bufPublic.size(), bufPublic.data()));

            assert_true(rc);
        }

        data.public_key_.emplace([&] {
            auto out = decltype(data.public_key_)::value_type{};
            const auto rc = api_.Crypto().Encode().Z85Decode(
                {bufPublic.data(), encoded_key_size_}, out.WriteInto());

            assert_true(rc);

            return out;
        }());
        data.private_key_.emplace([&] {
            auto out = api_.Factory().Secret(key_size_);
            const auto rc = api_.Crypto().Encode().Z85Decode(
                {bufSecret.data(), encoded_key_size_}, out.WriteInto());

            assert_true(rc);

            return out;
        }());
    } catch (const std::exception& e) {
        LogAbort()()(e.what()).Abort();
    }
}

auto OTDHT::Start(std::shared_ptr<const api::internal::Session> api) noexcept
    -> void
{
    auto handle = get_data();
    const auto& data = *handle;
    const auto& options = api_.GetOptions();

    if (false == options.LoopbackDHT()) {
        static const auto defaultServers = Endpoints{
            "tcp://metier1.opentransactions.org:8814",
            "tcp://metier2.opentransactions.org:8814",
            "tcp://ot01.matterfi.net:8814",
        };
        const auto existing = [&] {
            auto out = Set<CString>{};

            // TODO allocator
            for (const auto& server : KnownPeers({})) {
                // TODO GetSyncServers should return pmr strings
                out.emplace(server.c_str());
            }

            for (const auto& server : defaultServers) {
                if (false == out.contains(server)) {
                    if (false == options.TestMode()) { AddPeer(server); }

                    out.emplace(server);
                }
            }

            return out;
        }();

        try {
            for (const auto& endpoint : options.RemoteBlockchainSyncServers()) {
                if (false == existing.contains(endpoint)) { AddPeer(endpoint); }
            }
        } catch (...) {
        }
    }

    opentxs::network::otdht::Node{
        api_.Self(), data.public_key_->Bytes(), *data.private_key_}
        .Init(api);
}

auto OTDHT::StartListener(
    std::string_view syncEndpoint,
    std::string_view publicSyncEndpoint,
    std::string_view updateEndpoint,
    std::string_view publicUpdateEndpoint) const noexcept -> bool
{
    return get_data()->to_node_.SendDeferred([&] {
        using Job = opentxs::network::otdht::NodeJob;
        auto out = MakeWork(Job::add_listener);
        out.AddFrame(syncEndpoint.data(), syncEndpoint.size());
        out.AddFrame(publicSyncEndpoint.data(), publicSyncEndpoint.size());
        out.AddFrame(updateEndpoint.data(), updateEndpoint.size());
        out.AddFrame(publicUpdateEndpoint.data(), publicUpdateEndpoint.size());

        return out;
    }());
}

auto OTDHT::write_config(
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
        LogAbort()()(e.what()).Abort();
    }
}

OTDHT::~OTDHT() = default;
}  // namespace opentxs::api::network::implementation
