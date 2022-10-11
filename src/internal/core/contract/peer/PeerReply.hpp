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
namespace reply
{
class Acknowledgement;
class Bailment;
class Connection;
class Outbailment;
}  // namespace reply

class Reply;
}  // namespace peer
}  // namespace contract

namespace identifier
{
class Notary;
}  // namespace identifier

namespace proto
{
class PeerReply;
}  // namespace proto

using OTPeerReply = SharedPimpl<contract::peer::Reply>;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace std
{
template <>
struct less<opentxs::OTPeerReply> {
    auto operator()(
        const opentxs::OTPeerReply& lhs,
        const opentxs::OTPeerReply& rhs) const -> bool;
};
}  // namespace std

namespace opentxs::contract::peer
{
auto operator<(const OTPeerReply&, const OTPeerReply&) noexcept -> bool;
}  // namespace opentxs::contract::peer

namespace opentxs::contract::peer
{
class Reply : virtual public opentxs::contract::Signable
{
public:
    using SerializedType = proto::PeerReply;

    virtual auto asAcknowledgement() const noexcept
        -> const reply::Acknowledgement& = 0;
    virtual auto asBailment() const noexcept -> const reply::Bailment& = 0;
    virtual auto asConnection() const noexcept -> const reply::Connection& = 0;
    virtual auto asOutbailment() const noexcept
        -> const reply::Outbailment& = 0;

    using Signable::Serialize;
    virtual auto Serialize(SerializedType&) const -> bool = 0;
    virtual auto Type() const -> PeerRequestType = 0;
    virtual auto Server() const -> const identifier::Notary& = 0;

    Reply(const Reply&) = delete;
    Reply(Reply&&) = delete;
    auto operator=(const Reply&) -> Reply& = delete;
    auto operator=(Reply&&) -> Reply& = delete;

    ~Reply() override = default;

protected:
    Reply() noexcept = default;

private:
    friend OTPeerReply;

#ifndef _WIN32
    auto clone() const noexcept -> Reply* override = 0;
#endif
};
}  // namespace opentxs::contract::peer
