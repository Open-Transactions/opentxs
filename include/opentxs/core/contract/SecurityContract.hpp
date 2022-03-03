// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include "opentxs/core/contract/Unit.hpp"
#include "opentxs/util/SharedPimpl.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace contract
{
namespace unit
{
class Security;
}  // namespace unit
}  // namespace contract

using OTSecurityContract = SharedPimpl<contract::unit::Security>;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::contract::unit
{
class OPENTXS_EXPORT Security : virtual public contract::Unit
{
public:
    ~Security() override = default;

protected:
    Security() noexcept = default;

private:
    friend OTSecurityContract;

#ifndef _WIN32
    auto clone() const noexcept -> Security* override = 0;
#endif

    Security(const Security&) = delete;
    Security(Security&&) = delete;
    auto operator=(const Security&) -> Security& = delete;
    auto operator=(Security&&) -> Security& = delete;
};
}  // namespace opentxs::contract::unit
