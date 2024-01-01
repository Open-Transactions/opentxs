// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Export.hpp"
#include "opentxs/identity/credential/Base.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

namespace protobuf
{
class VerificationItem;
}  // namespace protobuf
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::identity::credential
{
class OPENTXS_EXPORT Verification : virtual public Base
{
public:
    OPENTXS_NO_EXPORT static auto SigningForm(
        const protobuf::VerificationItem& item) -> protobuf::VerificationItem;
    OPENTXS_NO_EXPORT static auto VerificationID(
        const api::Session& api,
        const protobuf::VerificationItem& item) -> UnallocatedCString;

    Verification(const Verification&) = delete;
    Verification(Verification&&) = delete;
    auto operator=(const Verification&) -> Verification& = delete;
    auto operator=(Verification&&) -> Verification& = delete;

    ~Verification() override = default;

protected:
    Verification() noexcept = default;  // TODO Signable
};
}  // namespace opentxs::identity::credential
