// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <irrxml/irrXML.hpp>
#include <cstdint>

#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace irr
{
namespace io
{
class IXMLBase;
template <class char_type, class super_class>
class IIrrXMLReader;

using IrrXMLReader = IIrrXMLReader<char, IXMLBase>;
}  // namespace io
}  // namespace irr

namespace opentxs
{
namespace identifier
{
class Generic;
}  // namespace identifier

class OTStashItem;
class String;
class Tag;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs
{
using mapOfStashItems = UnallocatedMap<UnallocatedCString, OTStashItem*>;

class OTStash
{
    UnallocatedCString stash_name_;

    mapOfStashItems stash_items_;  // map of stash items by instrument
                                   // definition ID.
                                   // owned.
public:
    auto GetName() const -> const UnallocatedCString { return stash_name_; }
    auto GetStash(const UnallocatedCString& str_instrument_definition_id)
        -> OTStashItem*;

    auto GetAmount(const UnallocatedCString& str_instrument_definition_id)
        -> std::int64_t;
    auto CreditStash(
        const UnallocatedCString& str_instrument_definition_id,
        const std::int64_t& lAmount) -> bool;
    auto DebitStash(
        const UnallocatedCString& str_instrument_definition_id,
        const std::int64_t& lAmount) -> bool;

    void Serialize(Tag& parent) const;
    auto ReadFromXMLNode(
        irr::io::IrrXMLReader*& xml,
        const String& strStashName,
        const String& strItemCount) -> std::int32_t;

    OTStash();
    OTStash(const UnallocatedCString& str_stash_name);
    OTStash(const String& strInstrumentDefinitionID, std::int64_t lAmount = 0);
    OTStash(
        const identifier::Generic& theInstrumentDefinitionID,
        std::int64_t lAmount = 0);
    virtual ~OTStash();
};

}  // namespace opentxs
