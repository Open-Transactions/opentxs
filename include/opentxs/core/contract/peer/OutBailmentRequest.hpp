// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include "opentxs/core/contract/peer/PeerRequest.hpp"
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
namespace request
{
class Outbailment;
}  // namespace request
}  // namespace peer
}  // namespace contract

using OTOutbailmentRequest = SharedPimpl<contract::peer::request::Outbailment>;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::contract::peer::request
{
class OPENTXS_EXPORT Outbailment : virtual public peer::Request
{
public:
    Outbailment(const Outbailment&) = delete;
    Outbailment(Outbailment&&) = delete;
    auto operator=(const Outbailment&) -> Outbailment& = delete;
    auto operator=(Outbailment&&) -> Outbailment& = delete;

    ~Outbailment() override = default;

protected:
    Outbailment() noexcept = default;

private:
    friend OTOutbailmentRequest;

#ifndef _WIN32
    auto clone() const noexcept -> Outbailment* override = 0;
#endif
};
}  // namespace opentxs::contract::peer::request
