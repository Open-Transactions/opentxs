// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <atomic>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <functional>
#include <iosfwd>
#include <memory>
#include <mutex>
#include <thread>

#include "api/session/Session.hpp"
#include "internal/api/session/Notary.hpp"
#include "internal/api/session/Session.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/core/identifier/Notary.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/otx/blind/Mint.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/PasswordPrompt.hpp"
#include "otx/server/Server.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace api
{
namespace session
{
class Contacts;
}  // namespace session

class Context;
class Crypto;
class Settings;
}  // namespace api

namespace identifier
{
class Notary;
class Nym;
class UnitDefinition;
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
namespace blind
{
class Mint;
}  // namespace blind
}  // namespace otx

namespace server
{
class MessageProcessor;
}  // namespace server

class Factory;
class Flag;
class Options;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::api::session::imp
{
class Notary final : public session::internal::Notary, public Session
{
public:
    auto Contacts() const -> const session::Contacts& final { abort(); }
    auto DropIncoming(const int count) const -> void final;
    auto DropOutgoing(const int count) const -> void final;
    auto GetAdminNym() const -> UnallocatedCString final;
    auto GetAdminPassword() const -> UnallocatedCString final;
    auto GetPrivateMint(
        const identifier::UnitDefinition& unitID,
        std::uint32_t series) const noexcept -> otx::blind::Mint& final;
    auto GetPublicMint(const identifier::UnitDefinition& unitID) const noexcept
        -> otx::blind::Mint& final;
    auto GetUserName() const -> UnallocatedCString final;
    auto GetUserTerms() const -> UnallocatedCString final;
    auto ID() const -> const identifier::Notary& final;
    auto InprocEndpoint() const -> UnallocatedCString final;
    auto NymID() const -> const identifier::Nym& final;
    auto ScanMints() const -> void final;
    auto Server() const -> opentxs::server::Server& final { return server_; }
    auto SetMintKeySize(const std::size_t size) const -> void final
    {
        mint_key_size_.store(size);
    }
    auto UpdateMint(const identifier::UnitDefinition& unitID) const
        -> void final;

    auto Init() -> void;

    Notary(
        const api::Context& parent,
        Flag& running,
        Options&& args,
        const api::Crypto& crypto,
        const api::Settings& config,
        const opentxs::network::zeromq::Context& context,
        const UnallocatedCString& dataFolder,
        const int instance);

    ~Notary() final;

private:
    using MintSeries = UnallocatedMap<UnallocatedCString, otx::blind::Mint>;

    const OTPasswordPrompt reason_;
    std::unique_ptr<opentxs::server::Server> server_p_;
    opentxs::server::Server& server_;
    std::unique_ptr<opentxs::server::MessageProcessor> message_processor_p_;
    opentxs::server::MessageProcessor& message_processor_;
    std::thread mint_thread_;
    mutable std::mutex mint_lock_;
    mutable std::mutex mint_update_lock_;
    mutable std::mutex mint_scan_lock_;
    mutable UnallocatedMap<UnallocatedCString, MintSeries> mints_;
    mutable UnallocatedDeque<UnallocatedCString> mints_to_check_;
    mutable std::atomic<std::size_t> mint_key_size_;

    auto generate_mint(
        const UnallocatedCString& serverID,
        const UnallocatedCString& unitID,
        const std::uint32_t series) const -> void;
    auto last_generated_series(
        const UnallocatedCString& serverID,
        const UnallocatedCString& unitID) const -> std::int32_t;
    auto load_private_mint(
        const opentxs::Lock& lock,
        const UnallocatedCString& unitID,
        const UnallocatedCString seriesID) const -> otx::blind::Mint;
    auto load_public_mint(
        const opentxs::Lock& lock,
        const UnallocatedCString& unitID,
        const UnallocatedCString seriesID) const -> otx::blind::Mint;
    auto mint() const -> void;
    auto verify_lock(const opentxs::Lock& lock, const std::mutex& mutex) const
        -> bool;
    auto verify_mint(
        const opentxs::Lock& lock,
        const UnallocatedCString& unitID,
        const UnallocatedCString seriesID,
        otx::blind::Mint&& mint) const -> otx::blind::Mint;
    auto verify_mint_directory(const UnallocatedCString& serverID) const
        -> bool;

    auto Cleanup() -> void;
    auto Start() -> void final;

    Notary() = delete;
    Notary(const Notary&) = delete;
    Notary(Notary&&) = delete;
    auto operator=(const Notary&) -> Notary& = delete;
    auto operator=(Notary&&) -> Notary& = delete;
};
}  // namespace opentxs::api::session::imp
