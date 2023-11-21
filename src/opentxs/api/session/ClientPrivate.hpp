// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <filesystem>
#include <memory>
#include <mutex>

#include "internal/otx/client/Pair.hpp"
#include "internal/otx/client/ServerAction.hpp"
#include "internal/util/Types.hpp"
#include "opentxs/api/Session.hpp"
#include "opentxs/api/Session.internal.hpp"
#include "opentxs/api/SessionPrivate.hpp"
#include "opentxs/api/network/ZMQ.hpp"
#include "opentxs/api/session/Activity.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Client.internal.hpp"
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
namespace crypto
{
class Blockchain;
}  // namespace crypto

namespace session
{
class ClientPrivate;  // IWYU pragma: keep
}  // namespace session

class Context;
class Crypto;
class Settings;
}  // namespace api

namespace identifier
{
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

class Flag;
class OTAPI_Exec;
class OT_API;
class Options;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

class opentxs::api::session::ClientPrivate final : public internal::Client,
                                                   public SessionPrivate
{
public:
    auto Activity() const -> const session::Activity& final;
    auto Contacts() const -> const session::Contacts& final;
    auto Exec(const UnallocatedCString& wallet = "") const
        -> const OTAPI_Exec& final;
    auto GetShared() const noexcept
        -> std::shared_ptr<const api::internal::Session> final;
    using SessionPrivate::Lock;
    auto Lock(const identifier::Nym& nymID, const identifier::Notary& serverID)
        const -> std::recursive_mutex& final;
    auto NewNym(const identifier::Nym& id) const noexcept -> void final;
    auto OTAPI(const UnallocatedCString& wallet = "") const
        -> const OT_API& final;
    auto OTX() const -> const session::OTX& final;
    auto Pair() const -> const otx::client::Pair& final;
    auto Self() const noexcept -> const api::Session& final { return self_; }
    auto ServerAction() const -> const otx::client::ServerAction& final;
    auto SharedClient() const noexcept
        -> std::shared_ptr<const internal::Client> final;
    auto UI() const -> const session::UI& final;
    auto Workflow() const -> const session::Workflow& final;
    auto ZMQ() const -> const api::network::ZMQ& final;

    using SessionPrivate::asClient;
    auto asClient() noexcept -> session::internal::Client& final
    {
        return *this;
    }
    using SessionPrivate::asClientPublic;
    auto asClientPublic() noexcept -> session::Client& final { return self_; }
    auto Init() -> void final;
    auto Self() noexcept -> api::Session& final { return self_; }
    auto Start(std::shared_ptr<internal::Client> api) noexcept -> void final;
    auto StartBlockchain() noexcept -> void;
    auto StartContacts() -> void;

    ClientPrivate(
        const api::Context& parent,
        Flag& running,
        Options&& args,
        const api::Settings& config,
        const api::Crypto& crypto,
        const opentxs::network::zeromq::Context& context,
        const std::filesystem::path& dataFolder,
        const int instance);
    ClientPrivate() = delete;
    ClientPrivate(const ClientPrivate&) = delete;
    ClientPrivate(ClientPrivate&&) = delete;
    auto operator=(const ClientPrivate&) -> ClientPrivate& = delete;
    auto operator=(ClientPrivate&&) -> ClientPrivate& = delete;

    ~ClientPrivate() final;

private:
    session::Client self_;
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
    std::weak_ptr<internal::Client> me_;

    auto get_lock(const ContextID context) const -> std::recursive_mutex&;

    auto Cleanup() -> void;
};
