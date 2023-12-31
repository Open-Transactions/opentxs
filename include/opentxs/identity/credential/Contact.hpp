// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>

#include "opentxs/Export.hpp"
#include "opentxs/Time.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/identity/credential/Base.hpp"
#include "opentxs/identity/wot/Types.hpp"
#include "opentxs/identity/wot/claim/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

namespace identifier
{
class Generic;
}  // namespace identifier

namespace identity
{
namespace wot
{
class Claim;
}  // namespace wot
}  // namespace identity

namespace protobuf
{
class Claim;
class ContactItem;
}  // namespace protobuf
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::identity::credential
{
class OPENTXS_EXPORT Contact : virtual public Base
{
public:
    static auto ClaimID(
        const api::Session& api,
        const identity::wot::Claimant& claimant,
        const wot::claim::SectionType section,
        const wot::claim::ClaimType type,
        Time start,
        Time end,
        std::string_view value,
        ReadView subtype,
        VersionNumber version) -> identifier::Generic;

    Contact(const Contact&) = delete;
    Contact(Contact&&) = delete;
    auto operator=(const Contact&) -> Contact& = delete;
    auto operator=(Contact&&) -> Contact& = delete;

    ~Contact() override = default;

protected:
    Contact() noexcept = default;  // TODO Signable
};
}  // namespace opentxs::identity::credential
