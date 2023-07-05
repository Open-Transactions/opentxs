// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>
#include <memory>
#include <tuple>

#include "internal/util/SharedPimpl.hpp"
#include "opentxs/core/Secret.hpp"
#include "opentxs/core/Types.hpp"
#include "opentxs/core/contract/Signable.hpp"
#include "opentxs/core/contract/Types.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace contract
{
class Server;
}  // namespace contract

namespace identifier
{
class Notary;
}  // namespace identifier

namespace proto
{
class ServerContract;
}  // namespace proto

class PasswordPrompt;
class Writer;

using OTServerContract = SharedPimpl<contract::Server>;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::contract
{
class Server : virtual public opentxs::contract::Signable<identifier::Notary>
{
public:
    using Endpoint = std::tuple<
        AddressType,
        contract::ProtocolVersion,
        UnallocatedCString,  // hostname / address
        std::uint32_t,       // port
        VersionNumber>;      // version

    static const VersionNumber DefaultVersion;

    virtual auto ConnectInfo(
        UnallocatedCString& strHostname,
        std::uint32_t& nPort,
        AddressType& actual,
        const AddressType& preferred) const -> bool = 0;
    virtual auto EffectiveName() const -> UnallocatedCString = 0;
    using Signable::Serialize;
    virtual auto Serialize(Writer&& destination, bool includeNym) const
        -> bool = 0;
    virtual auto Serialize(proto::ServerContract&, bool includeNym = false)
        const -> bool = 0;
    virtual auto Statistics(String& strContents) const -> bool = 0;
    virtual auto TransportKey() const -> const Data& = 0;
    virtual auto TransportKey(Data& pubkey, const PasswordPrompt& reason) const
        -> Secret = 0;

    virtual void InitAlias(std::string_view alias) = 0;

    Server(const Server&) = delete;
    Server(Server&&) = delete;
    auto operator=(const Server&) -> Server& = delete;
    auto operator=(Server&&) -> Server& = delete;

    ~Server() override = default;

protected:
    Server() noexcept = default;

private:
    friend OTServerContract;
};
}  // namespace opentxs::contract
