// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <filesystem>
#include <functional>
#include <memory>
#include <mutex>
#include <tuple>
#include <type_traits>

#include "api/session/Session.hpp"
#include "internal/api/session/Client.hpp"
#include "internal/otx/client/Pair.hpp"
#include "internal/otx/client/ServerAction.hpp"
#include "internal/util/Types.hpp"
#include "opentxs/api/crypto/Blockchain.hpp"
#include "opentxs/api/network/ZMQ.hpp"
#include "opentxs/api/session/Activity.hpp"
#include "opentxs/api/session/Contacts.hpp"
#include "opentxs/api/session/OTX.hpp"
#include "opentxs/api/session/UI.hpp"
#include "opentxs/api/session/Workflow.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace session
{
class Client;
class Contacts;
}  // namespace session

namespace crypto
{
class Blockchain;
}  // namespace crypto

class Context;
class Crypto;
class Legacy;
class Session;
class Settings;
}  // namespace api

namespace identifier
{
class Generic;
class Notary;
class Nym;
}  // namespace identifier

namespace network
{
namespace zeromq
{
class Context;
}  // namespace zeromq
}  // namespace network

namespace otx
{
namespace client
{
class Pair;
class ServerAction;
}  // namespace client
}  // namespace otx

class Flag;
class OTAPI_Exec;
class OT_API;
class Options;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::api::session::imp
{
class Client final : public internal::Client, public Session
{
public:
    auto Activity() const -> const session::Activity& final;
    auto Contacts() const -> const session::Contacts& final;
    auto Exec(const UnallocatedCString& wallet = "") const
        -> const OTAPI_Exec& final;
    using Session::Lock;
    auto Lock(const identifier::Nym& nymID, const identifier::Notary& serverID)
        const -> std::recursive_mutex& final;
    auto NewNym(const identifier::Nym& id) const noexcept -> void final;
    auto OTAPI(const UnallocatedCString& wallet = "") const
        -> const OT_API& final;
    auto OTX() const -> const session::OTX& final;
    auto Pair() const -> const otx::client::Pair& final;
    auto ServerAction() const -> const otx::client::ServerAction& final;
    auto UI() const -> const session::UI& final;
    auto Workflow() const -> const session::Workflow& final;
    auto ZMQ() const -> const api::network::ZMQ& final;

    auto Init() -> void final;
    auto Start(std::shared_ptr<const api::Session> api) noexcept -> void final;
    auto StartActivity() -> void;
    auto StartBlockchain() noexcept -> void;
    auto StartContacts() -> void;

    Client(
        const api::Context& parent,
        Flag& running,
        Options&& args,
        const api::Settings& config,
        const api::Crypto& crypto,
        const opentxs::network::zeromq::Context& context,
        const std::filesystem::path& dataFolder,
        const int instance);
    Client() = delete;
    Client(const Client&) = delete;
    Client(Client&&) = delete;
    auto operator=(const Client&) -> Client& = delete;
    auto operator=(Client&&) -> Client& = delete;

    ~Client() final;

private:
    std::unique_ptr<network::ZMQ> zeromq_;
    std::unique_ptr<session::Contacts> contacts_;
    std::unique_ptr<session::Activity> activity_;
    std::shared_ptr<crypto::Blockchain> blockchain_;
    std::unique_ptr<session::Workflow> workflow_;
    std::unique_ptr<OT_API> ot_api_;
    std::unique_ptr<OTAPI_Exec> otapi_exec_;
    std::unique_ptr<otx::client::ServerAction> server_action_;
    std::unique_ptr<session::OTX> otx_;
    std::unique_ptr<otx::client::Pair> pair_;
    std::unique_ptr<session::UI> ui_;
    mutable std::mutex map_lock_;
    mutable UnallocatedMap<ContextID, std::recursive_mutex> context_locks_;

    auto get_lock(const ContextID context) const -> std::recursive_mutex&;

    auto Cleanup() -> void;
};
}  // namespace opentxs::api::session::imp
