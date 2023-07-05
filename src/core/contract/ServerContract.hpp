// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <ServerContract.pb.h>
#include <cstdint>
#include <string_view>

#include "core/contract/Signable.hpp"
#include "internal/core/contract/ServerContract.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/core/Secret.hpp"
#include "opentxs/core/Types.hpp"
#include "opentxs/core/identifier/Notary.hpp"
#include "opentxs/identity/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

namespace proto
{
class Signature;
}  // namespace proto

class Data;
class Factory;
class PasswordPrompt;
class String;
class Writer;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::contract::implementation
{
class Server final
    : public contract::Server,
      public opentxs::contract::implementation::Signable<identifier::Notary>
{
public:
    auto ConnectInfo(
        UnallocatedCString& strHostname,
        std::uint32_t& nPort,
        AddressType& actual,
        const AddressType& preferred) const -> bool final;
    auto EffectiveName() const -> UnallocatedCString final;
    auto Name() const noexcept -> std::string_view final { return name_; }
    auto Serialize(Writer&& out) const noexcept -> bool final;
    auto Serialize(Writer&& destination, bool includeNym = false) const
        -> bool final;
    auto Serialize(proto::ServerContract& output, bool includeNym = false) const
        -> bool final;
    auto Statistics(String& strContents) const -> bool final;
    auto TransportKey() const -> const Data& final;
    auto TransportKey(Data& pubkey, const PasswordPrompt& reason) const
        -> Secret final;

    auto InitAlias(std::string_view alias) -> void final
    {
        Signable::SetAlias(alias);
    }
    auto SetAlias(std::string_view) noexcept -> bool final;

    Server(
        const api::Session& api,
        const Nym_p& nym,
        const VersionNumber version,
        const UnallocatedCString& terms,
        const UnallocatedCString& name,
        UnallocatedList<contract::Server::Endpoint>&& endpoints,
        ByteArray&& key,
        Signatures&& signatures = {});
    Server(
        const api::Session& api,
        const Nym_p& nym,
        const proto::ServerContract& serialized);
    Server() = delete;
    Server(const Server&);
    Server(Server&&) = delete;
    auto operator=(const Server&) -> Server& = delete;
    auto operator=(Server&&) -> Server& = delete;

    ~Server() final = default;

private:
    friend opentxs::Factory;

    const UnallocatedList<contract::Server::Endpoint> listen_params_;
    const UnallocatedCString name_;
    const ByteArray transport_key_;

    static auto extract_endpoints(
        const proto::ServerContract& serialized) noexcept
        -> UnallocatedList<contract::Server::Endpoint>;

    auto calculate_id() const -> identifier_type final;
    auto contract() const -> proto::ServerContract;
    auto IDVersion() const -> proto::ServerContract;
    auto SigVersion() const -> proto::ServerContract;
    auto validate() const -> bool final;
    auto verify_signature(const proto::Signature& signature) const
        -> bool final;

    auto update_signature(const PasswordPrompt& reason) -> bool final;
};
}  // namespace opentxs::contract::implementation
