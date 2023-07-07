// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "internal/core/contract/peer/reply/Base.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace contract
{
namespace peer
{
namespace reply
{
namespace internal
{
class Acknowledgement;
}  // namespace internal
}  // namespace reply
}  // namespace peer
}  // namespace contract

using OTReplyAcknowledgement =
    SharedPimpl<contract::peer::reply::internal::Acknowledgement>;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::contract::peer::reply::internal
{
class Acknowledgement : virtual public internal::Reply
{
public:
    Acknowledgement(const Acknowledgement&) = delete;
    Acknowledgement(Acknowledgement&&) = delete;
    auto operator=(const Acknowledgement&) -> Acknowledgement& = delete;
    auto operator=(Acknowledgement&&) -> Acknowledgement& = delete;

    ~Acknowledgement() override = default;

protected:
    Acknowledgement() noexcept = default;

private:
    friend OTReplyAcknowledgement;
};
}  // namespace opentxs::contract::peer::reply::internal
