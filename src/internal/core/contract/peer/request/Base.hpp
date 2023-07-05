// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <functional>

#include "internal/util/SharedPimpl.hpp"
#include "opentxs/core/contract/Signable.hpp"
#include "opentxs/core/contract/peer/Types.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace contract
{
namespace peer
{
namespace request
{
namespace internal
{
class Bailment;
class BailmentNotice;
class Connection;
class Faucet;
class Outbailment;
class Request;
class StoreSecret;
}  // namespace internal
}  // namespace request
}  // namespace peer
}  // namespace contract

namespace identifier
{
class Nym;
class Notary;
}  // namespace identifier

namespace proto
{
class PeerRequest;
}  // namespace proto

using OTPeerRequest = SharedPimpl<contract::peer::request::internal::Request>;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace std
{
template <>
struct less<opentxs::OTPeerRequest> {
    auto operator()(
        const opentxs::OTPeerRequest& lhs,
        const opentxs::OTPeerRequest& rhs) const -> bool;
};
}  // namespace std

namespace opentxs
{
auto operator<(const OTPeerRequest&, const OTPeerRequest&) noexcept -> bool;
}  // namespace opentxs

namespace opentxs::contract::peer::request::internal
{
class Request : virtual public opentxs::contract::Signable<identifier::Generic>
{
public:
    using SerializedType = proto::PeerRequest;

    virtual auto asBailment() const noexcept -> const internal::Bailment& = 0;
    virtual auto asBailmentNotice() const noexcept
        -> const internal::BailmentNotice& = 0;
    virtual auto asConnection() const noexcept
        -> const internal::Connection& = 0;
    virtual auto asFaucet() const noexcept -> const internal::Faucet& = 0;
    virtual auto asOutbailment() const noexcept
        -> const internal::Outbailment& = 0;
    virtual auto asStoreSecret() const noexcept
        -> const internal::StoreSecret& = 0;

    using Signable::Serialize;
    virtual auto Serialize(SerializedType&) const -> bool = 0;
    virtual auto Initiator() const -> const identifier::Nym& = 0;
    virtual auto Recipient() const -> const identifier::Nym& = 0;
    virtual auto Server() const -> const identifier::Notary& = 0;
    virtual auto Type() const -> RequestType = 0;

    Request(const Request&) = delete;
    Request(Request&&) = delete;
    auto operator=(const Request&) -> Request& = delete;
    auto operator=(Request&&) -> Request& = delete;

    ~Request() override = default;

protected:
    Request() noexcept = default;

private:
    friend OTPeerRequest;
};
}  // namespace opentxs::contract::peer::request::internal
