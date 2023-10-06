// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <atomic>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <memory>
#include <string_view>

#include "api/session/Session.hpp"
#include "api/session/notary/Shared.hpp"
#include "internal/api/session/Session.hpp"
#include "internal/api/session/notary/Notary.hpp"
#include "internal/api/session/notary/Types.hpp"
#include "opentxs/Export.hpp"
#include "opentxs/core/identifier/Notary.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/otx/blind/Mint.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/PasswordPrompt.hpp"
#include "otx/server/Server.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace session
{
class Contacts;
class Notary;
}  // namespace session

class Context;
class Crypto;
class Session;
class Settings;
}  // namespace api

namespace identifier
{
class UnitDefinition;
}  // namespace identifier

namespace network
{
namespace zeromq
{
class Context;
}  // namespace zeromq
}  // namespace network

namespace server
{
class MessageProcessor;
}  // namespace server

class Flag;
class Options;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::api::session::imp
{
class OPENTXS_NO_EXPORT Notary final : public session::internal::Notary,
                                       public Session
{
public:
    auto CheckMint(const identifier::UnitDefinition& unit) noexcept
        -> void final;
    auto Contacts() const -> const session::Contacts& final { abort(); }
    auto DropIncoming(const int count) const -> void final;
    auto DropOutgoing(const int count) const -> void final;
    auto GetAdminNym() const -> UnallocatedCString final;
    auto GetAdminPassword() const -> UnallocatedCString final;
    auto GetOwnerName() const -> std::string_view final;
    auto GetPrivateMint(
        const identifier::UnitDefinition& unitID,
        std::uint32_t series) const noexcept -> otx::blind::Mint& final;
    auto GetPublicMint(const identifier::UnitDefinition& unitID) const noexcept
        -> otx::blind::Mint& final;
    auto GetShared() const noexcept
        -> std::shared_ptr<const api::Session> final;
    auto GetUserTerms() const -> std::string_view final;
    auto ID() const -> const identifier::Notary& final;
    auto InprocEndpoint() const -> UnallocatedCString final;
    auto NymID() const -> const identifier::Nym& final;
    auto Server() const -> opentxs::server::Server& final { return server_; }
    auto SetMintKeySize(const std::size_t size) const -> void final
    {
        mint_key_size_.store(size);
    }
    auto UpdateMint(const identifier::UnitDefinition& unitID) const
        -> void final;

    auto Init(std::shared_ptr<session::Notary> me) -> void;
    auto Start(std::shared_ptr<session::Notary> api) noexcept -> void final;

    Notary(
        const api::Context& parent,
        Flag& running,
        Options&& args,
        const api::Crypto& crypto,
        const api::Settings& config,
        const opentxs::network::zeromq::Context& context,
        const std::filesystem::path& dataFolder,
        const int instance);
    Notary() = delete;
    Notary(const Notary&) = delete;
    Notary(Notary&&) = delete;
    auto operator=(const Notary&) -> Notary& = delete;
    auto operator=(Notary&&) -> Notary& = delete;

    ~Notary() final;

private:
    const PasswordPrompt reason_;
    std::shared_ptr<notary::Shared> shared_p_;
    std::unique_ptr<opentxs::server::Server> server_p_;
    std::unique_ptr<opentxs::server::MessageProcessor> message_processor_p_;
    notary::Shared& shared_;
    opentxs::server::Server& server_;
    opentxs::server::MessageProcessor& message_processor_;
    mutable std::atomic<std::size_t> mint_key_size_;
    std::weak_ptr<api::session::Notary> me_;

    auto generate_mint(
        notary::Shared::Map& data,
        const identifier::Notary& serverID,
        const identifier::UnitDefinition& unitID,
        const std::uint32_t series) const -> void;
    auto get_private_mint(
        notary::Shared::Map& data,
        const identifier::UnitDefinition& unitID,
        std::uint32_t series) const noexcept -> otx::blind::Mint&;
    auto last_generated_series(
        notary::Shared::Map& data,
        const identifier::Notary& serverID,
        const identifier::UnitDefinition& unitID) const -> std::int32_t;
    auto load_private_mint(
        notary::Shared::Map& data,
        const identifier::UnitDefinition& unitID,
        const notary::MintSeriesID& seriesID) const -> otx::blind::Mint;
    auto load_public_mint(
        notary::Shared::Map& data,
        const identifier::UnitDefinition& unitID,
        const notary::MintSeriesID& seriesID) const -> otx::blind::Mint;
    auto verify_mint(
        notary::Shared::Map& data,
        const identifier::UnitDefinition& unitID,
        const notary::MintSeriesID& seriesID,
        otx::blind::Mint&& mint) const -> otx::blind::Mint;
    auto verify_mint_directory(
        notary::Shared::Map& data,
        const identifier::Notary& serverID) const -> bool;

    auto Cleanup() -> void;
    auto start(std::shared_ptr<session::Notary> me) -> void;
};
}  // namespace opentxs::api::session::imp
