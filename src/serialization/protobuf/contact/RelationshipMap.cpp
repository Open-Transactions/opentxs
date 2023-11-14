// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include <functional>

#include "internal/serialization/protobuf/Contact.hpp"  // IWYU pragma: associated

#include <ContactItemType.pb.h>

namespace opentxs::proto
{
auto RelationshipMap() noexcept -> const RelationshipReciprocity&
{
    static constexpr auto output = RelationshipReciprocity{
        {CITEMTYPE_ORGANIZATION, CITEMTYPE_MEMBER},
        {CITEMTYPE_GOVERNMENT, CITEMTYPE_CITIZEN},
        {CITEMTYPE_ALIAS, CITEMTYPE_ALIAS},
        {CITEMTYPE_ACQUAINTANCE, CITEMTYPE_ACQUAINTANCE},
        {CITEMTYPE_FRIEND, CITEMTYPE_FRIEND},
        {CITEMTYPE_SPOUSE, CITEMTYPE_SPOUSE},
        {CITEMTYPE_SIBLING, CITEMTYPE_SIBLING},
        {CITEMTYPE_MEMBER, CITEMTYPE_ORGANIZATION},
        {CITEMTYPE_COLLEAGUE, CITEMTYPE_COLLEAGUE},
        {CITEMTYPE_PARENT, CITEMTYPE_CHILD},
        {CITEMTYPE_CHILD, CITEMTYPE_PARENT},
        {CITEMTYPE_EMPLOYER, CITEMTYPE_EMPLOYEE},
        {CITEMTYPE_EMPLOYEE, CITEMTYPE_EMPLOYER},
        {CITEMTYPE_CITIZEN, CITEMTYPE_GOVERNMENT},
        {CITEMTYPE_SUPERVISOR, CITEMTYPE_SUBORDINATE},
        {CITEMTYPE_SUBORDINATE, CITEMTYPE_SUPERVISOR},
        {CITEMTYPE_MET, CITEMTYPE_MET},
        {CITEMTYPE_OWNER, CITEMTYPE_PROPERTY},
        {CITEMTYPE_PROPERTY, CITEMTYPE_OWNER}};

    return output;
}
}  // namespace opentxs::proto
