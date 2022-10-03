// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <ServerContract.pb.h>
#include <cstdint>
#include <memory>

#include "Proto.hpp"
#include "core/contract/Signable.hpp"
#include "internal/util/Mutex.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/core/Secret.hpp"
#include "opentxs/core/Types.hpp"
#include "opentxs/core/contract/ServerContract.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/identity/Types.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

namespace identifier
{
class Notary;
}  // namespace identifier

namespace proto
{
class Signature;
}  // namespace proto

class Data;
class Factory;
class PasswordPrompt;
class String;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::contract::implementation
{
class Server final : public contract::Server,
                     public opentxs::contract::implementation::Signable
{
public:
    auto ConnectInfo(
        UnallocatedCString& strHostname,
        std::uint32_t& nPort,
        AddressType& actual,
        const AddressType& preferred) const -> bool final;
    auto EffectiveName() const -> UnallocatedCString final;
    auto Name() const noexcept -> UnallocatedCString final { return name_; }
    auto Serialize() const noexcept -> ByteArray final;
    auto Serialize(AllocateOutput destination, bool includeNym = false) const
        -> bool final;
    auto Serialize(proto::ServerContract& output, bool includeNym = false) const
        -> bool final;
    auto Statistics(String& strContents) const -> bool final;
    auto TransportKey() const -> const Data& final;
    auto TransportKey(Data& pubkey, const PasswordPrompt& reason) const
        -> OTSecret final;

    void InitAlias(const UnallocatedCString& alias) final
    {
        contract::implementation::Signable::SetAlias(alias);
    }
    auto SetAlias(const UnallocatedCString& alias) noexcept -> bool final;

    Server(
        const api::Session& api,
        const Nym_p& nym,
        const VersionNumber version,
        const UnallocatedCString& terms,
        const UnallocatedCString& name,
        UnallocatedList<contract::Server::Endpoint>&& endpoints,
        ByteArray&& key,
        identifier::Notary&& id,
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

    auto clone() const noexcept -> Server* final { return new Server(*this); }
    auto contract(const Lock& lock) const -> proto::ServerContract;
    auto GetID(const Lock& lock) const -> identifier::Generic final;
    auto IDVersion(const Lock& lock) const -> proto::ServerContract;
    auto SigVersion(const Lock& lock) const -> proto::ServerContract;
    auto validate(const Lock& lock) const -> bool final;
    auto verify_signature(const Lock& lock, const proto::Signature& signature)
        const -> bool final;

    auto update_signature(const Lock& lock, const PasswordPrompt& reason)
        -> bool final;
};
}  // namespace opentxs::contract::implementation
