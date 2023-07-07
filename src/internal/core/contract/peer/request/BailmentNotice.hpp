// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "internal/core/contract/peer/request/Base.hpp"
#include "internal/util/SharedPimpl.hpp"

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
class BailmentNotice;
}  // namespace internal
}  // namespace request
}  // namespace peer
}  // namespace contract

using OTBailmentNotice =
    SharedPimpl<contract::peer::request::internal::BailmentNotice>;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::contract::peer::request::internal
{
class BailmentNotice : virtual public internal::Request
{
public:
    BailmentNotice(const BailmentNotice&) = delete;
    BailmentNotice(BailmentNotice&&) = delete;
    auto operator=(const BailmentNotice&) -> BailmentNotice& = delete;
    auto operator=(BailmentNotice&&) -> BailmentNotice& = delete;

    ~BailmentNotice() override = default;

protected:
    BailmentNotice() noexcept = default;

private:
    friend OTBailmentNotice;
};
}  // namespace opentxs::contract::peer::request::internal
