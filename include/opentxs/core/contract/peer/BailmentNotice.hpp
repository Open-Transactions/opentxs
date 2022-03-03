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
class BailmentNotice;
}  // namespace request
}  // namespace peer
}  // namespace contract

using OTBailmentNotice = SharedPimpl<contract::peer::request::BailmentNotice>;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::contract::peer::request
{
class OPENTXS_EXPORT BailmentNotice : virtual public peer::Request
{
public:
    ~BailmentNotice() override = default;

protected:
    BailmentNotice() noexcept = default;

private:
    friend OTBailmentNotice;

#ifndef _WIN32
    auto clone() const noexcept -> BailmentNotice* override = 0;
#endif

    BailmentNotice(const BailmentNotice&) = delete;
    BailmentNotice(BailmentNotice&&) = delete;
    auto operator=(const BailmentNotice&) -> BailmentNotice& = delete;
    auto operator=(BailmentNotice&&) -> BailmentNotice& = delete;
};
}  // namespace opentxs::contract::peer::request
