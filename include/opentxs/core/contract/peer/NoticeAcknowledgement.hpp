// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include "opentxs/core/contract/peer/PeerReply.hpp"

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
}  // namespace reply
}  // namespace peer
}  // namespace contract

using OTReplyAcknowledgement =
    SharedPimpl<contract::peer::reply::Acknowledgement>;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::contract::peer::reply
{
class OPENTXS_EXPORT Acknowledgement : virtual public peer::Reply
{
public:
    ~Acknowledgement() override = default;

protected:
    Acknowledgement() noexcept = default;

private:
    friend OTReplyAcknowledgement;

#ifndef _WIN32
    auto clone() const noexcept -> Acknowledgement* override = 0;
#endif

    Acknowledgement(const Acknowledgement&) = delete;
    Acknowledgement(Acknowledgement&&) = delete;
    auto operator=(const Acknowledgement&) -> Acknowledgement& = delete;
    auto operator=(Acknowledgement&&) -> Acknowledgement& = delete;
};
}  // namespace opentxs::contract::peer::reply
