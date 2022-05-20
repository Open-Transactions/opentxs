// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <functional>

#include "opentxs/core/contract/Signable.hpp"
#include "opentxs/core/contract/peer/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/SharedPimpl.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
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
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace std
{
template <>
struct OPENTXS_EXPORT less<opentxs::OTPeerReply> {
    auto operator()(
        const opentxs::OTPeerReply& lhs,
        const opentxs::OTPeerReply& rhs) const -> bool;
};
}  // namespace std

namespace opentxs::contract::peer
{
OPENTXS_EXPORT auto operator<(const OTPeerReply&, const OTPeerReply&) noexcept
    -> bool;
}  // namespace opentxs::contract::peer

namespace opentxs::contract::peer
{
class OPENTXS_EXPORT Reply : virtual public opentxs::contract::Signable
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
    OPENTXS_NO_EXPORT virtual auto Serialize(SerializedType&) const -> bool = 0;
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
