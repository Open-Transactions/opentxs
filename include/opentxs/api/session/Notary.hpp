// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <cstdint>
#include <string_view>

#include "opentxs/Export.hpp"
#include "opentxs/api/Session.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace internal
{
class Session;
}  // namespace internal

namespace session
{
class Notary;  // IWYU pragma: keep
}  // namespace session
}  // namespace api

namespace identifier
{
class Notary;
class Nym;
class UnitDefinition;
}  // namespace identifier

namespace otx
{
namespace blind
{
class Mint;
}  // namespace blind
}  // namespace otx

namespace server
{
class Server;
}  // namespace server
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

class OPENTXS_EXPORT opentxs::api::session::Notary final : public api::Session
{
public:
    static auto DefaultMintKeyBytes() noexcept -> std::size_t;

    /** Drop a specified number of incoming requests for testing purposes */
    auto DropIncoming(const int count) const -> void;
    /** Drop a specified number of outgoing replies for testing purposes */
    auto DropOutgoing(const int count) const -> void;
    auto GetAdminNym() const -> UnallocatedCString;
    auto GetAdminPassword() const -> UnallocatedCString;
    auto GetOwnerName() const -> std::string_view;
    auto GetPrivateMint(
        const identifier::UnitDefinition& unitid,
        std::uint32_t series) const noexcept -> otx::blind::Mint&;
    auto GetPublicMint(const identifier::UnitDefinition& unitID) const noexcept
        -> otx::blind::Mint&;
    auto GetUserTerms() const -> std::string_view;
    auto ID() const -> const identifier::Notary&;
    auto NymID() const -> const identifier::Nym&;
    auto Server() const -> opentxs::server::Server&;
    auto SetMintKeySize(const std::size_t size) const -> void;
    auto UpdateMint(const identifier::UnitDefinition& unitID) const -> void;

    OPENTXS_NO_EXPORT Notary(api::internal::Session* imp) noexcept;
    Notary() = delete;
    Notary(const Notary&) = delete;
    Notary(Notary&&) = delete;
    auto operator=(const Notary&) -> Notary& = delete;
    auto operator=(Notary&&) -> Notary& = delete;

    OPENTXS_NO_EXPORT ~Notary() final;
};
