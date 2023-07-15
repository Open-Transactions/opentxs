// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>

#include "opentxs/Export.hpp"
#include "opentxs/identity/credential/Base.hpp"
#include "opentxs/identity/wot/claim/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Time.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

namespace identity
{
namespace wot
{
class Claim;
}  // namespace wot
}  // namespace identity

namespace proto
{
class Claim;
class ContactItem;
}  // namespace proto
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::identity::credential
{
class OPENTXS_EXPORT Contact : virtual public Base
{
public:
    static auto ClaimID(
        const api::Session& api,
        const UnallocatedCString& nymid,
        const wot::claim::SectionType section,
        const wot::claim::ClaimType type,
        const Time start,
        const Time end,
        const UnallocatedCString& value,
        const UnallocatedCString& subtype) -> UnallocatedCString;

    Contact(const Contact&) = delete;
    Contact(Contact&&) = delete;
    auto operator=(const Contact&) -> Contact& = delete;
    auto operator=(Contact&&) -> Contact& = delete;

    ~Contact() override = default;

protected:
    Contact() noexcept = default;  // TODO Signable
};
}  // namespace opentxs::identity::credential
