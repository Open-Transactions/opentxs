// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include "opentxs/identity/credential/Base.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace api
{
class Session;
}  // namespace api

namespace proto
{
class Verification;
}  // namespace proto
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::identity::credential
{
class OPENTXS_EXPORT Verification : virtual public Base
{
public:
    OPENTXS_NO_EXPORT static auto SigningForm(const proto::Verification& item)
        -> proto::Verification;
    OPENTXS_NO_EXPORT static auto VerificationID(
        const api::Session& api,
        const proto::Verification& item) -> UnallocatedCString;

    ~Verification() override = default;

protected:
    Verification() noexcept {}  // TODO Signable

private:
    Verification(const Verification&) = delete;
    Verification(Verification&&) = delete;
    auto operator=(const Verification&) -> Verification& = delete;
    auto operator=(Verification&&) -> Verification& = delete;
};
}  // namespace opentxs::identity::credential
