// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <irrxml/irrXML.hpp>
#include <cstdint>
#include <map>
#include <memory_resource>
#include <string>

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
class Identifier;
class OTStashItem;
class String;
class Tag;

using mapOfStashItems = std::pmr::map<std::string, OTStashItem*>;

class OTStash
{
    std::string m_str_stash_name;

    mapOfStashItems m_mapStashItems;  // map of stash items by instrument
                                      // definition ID.
                                      // owned.
public:
    auto GetName() const -> const std::string { return m_str_stash_name; }
    auto GetStash(const std::string& str_instrument_definition_id)
        -> OTStashItem*;

    auto GetAmount(const std::string& str_instrument_definition_id)
        -> std::int64_t;
    auto CreditStash(
        const std::string& str_instrument_definition_id,
        const std::int64_t& lAmount) -> bool;
    auto DebitStash(
        const std::string& str_instrument_definition_id,
        const std::int64_t& lAmount) -> bool;

    void Serialize(Tag& parent) const;
    auto ReadFromXMLNode(
        irr::io::IrrXMLReader*& xml,
        const String& strStashName,
        const String& strItemCount) -> std::int32_t;

    OTStash();
    OTStash(const std::string& str_stash_name);
    OTStash(const String& strInstrumentDefinitionID, std::int64_t lAmount = 0);
    OTStash(
        const Identifier& theInstrumentDefinitionID,
        std::int64_t lAmount = 0);
    virtual ~OTStash();
};

}  // namespace opentxs
