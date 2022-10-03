// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                            // IWYU pragma: associated
#include "internal/otx/smartcontract/OTStash.hpp"  // IWYU pragma: associated

#include <irrxml/irrXML.hpp>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <utility>

#include "internal/otx/common/XML.hpp"
#include "internal/otx/common/util/Tag.hpp"
#include "internal/otx/smartcontract/OTStashItem.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Pimpl.hpp"

namespace opentxs
{
OTStash::OTStash()
    : stash_name_()
    , stash_items_()
{
}

OTStash::OTStash(const UnallocatedCString& str_stash_name)
    : stash_name_(str_stash_name)
    , stash_items_()
{
}

OTStash::OTStash(const String& strInstrumentDefinitionID, std::int64_t lAmount)
    : stash_name_()
    , stash_items_()
{
    auto* pItem = new OTStashItem(strInstrumentDefinitionID, lAmount);
    OT_ASSERT(nullptr != pItem);

    stash_items_.insert(std::pair<UnallocatedCString, OTStashItem*>(
        strInstrumentDefinitionID.Get(), pItem));
}

OTStash::OTStash(
    const identifier::Generic& theInstrumentDefinitionID,
    std::int64_t lAmount)
    : stash_name_()
    , stash_items_()
{
    auto* pItem = new OTStashItem(theInstrumentDefinitionID, lAmount);
    OT_ASSERT(nullptr != pItem);

    auto strInstrumentDefinitionID = String::Factory(theInstrumentDefinitionID);

    stash_items_.insert(std::pair<UnallocatedCString, OTStashItem*>(
        strInstrumentDefinitionID->Get(), pItem));
}

void OTStash::Serialize(Tag& parent) const
{
    const auto sizeMapStashItems = stash_items_.size();

    TagPtr pTag(new Tag("stash"));

    pTag->add_attribute("name", stash_name_);
    pTag->add_attribute("count", std::to_string(sizeMapStashItems));

    for (const auto& it : stash_items_) {
        const UnallocatedCString str_instrument_definition_id = it.first;
        OTStashItem* pStashItem = it.second;
        OT_ASSERT(
            (str_instrument_definition_id.size() > 0) &&
            (nullptr != pStashItem));

        TagPtr pTagItem(new Tag("stashItem"));

        pTagItem->add_attribute(
            "instrumentDefinitionID",
            pStashItem->GetInstrumentDefinitionID().Get());
        pTagItem->add_attribute(
            "balance", std::to_string(pStashItem->GetAmount()));

        pTag->add_tag(pTagItem);
    }

    parent.add_tag(pTag);
}

auto OTStash::ReadFromXMLNode(
    irr::io::IrrXMLReader*& xml,
    const String& strStashName,
    const String& strItemCount) -> std::int32_t
{
    if (!strStashName.Exists()) {
        LogError()(OT_PRETTY_CLASS())("Failed: Empty stash 'name' "
                                      "attribute.")
            .Flush();
        return (-1);
    }

    stash_name_ = strStashName.Get();

    //
    // Load up the stash items.
    //
    // NOLINTNEXTLINE(cert-err34-c)
    std::int32_t nCount = strItemCount.Exists() ? atoi(strItemCount.Get()) : 0;
    if (nCount > 0) {
        while (nCount-- > 0) {
            //            xml->read();
            if (!SkipToElement(xml)) {
                LogConsole()(OT_PRETTY_CLASS())("Failure: Unable to find "
                                                "expected element.")
                    .Flush();
                return (-1);
            }

            if ((xml->getNodeType() == irr::io::EXN_ELEMENT) &&
                (!strcmp("stashItem", xml->getNodeName()))) {
                auto strInstrumentDefinitionID =
                    String::Factory(xml->getAttributeValue(
                        "instrumentDefinitionID"));  // Instrument Definition ID
                                                     // of this account.
                auto strAmount = String::Factory(xml->getAttributeValue(
                    "balance"));  // Account ID for this account.

                if (!strInstrumentDefinitionID->Exists() ||
                    !strAmount->Exists()) {
                    LogError()(OT_PRETTY_CLASS())(
                        "Error loading "
                        "stashItem: Either the instrumentDefinitionID (")(
                        strInstrumentDefinitionID.get())("), or the balance (")(
                        strAmount.get())(") was EMPTY.")
                        .Flush();
                    return (-1);
                }

                if (!CreditStash(
                        strInstrumentDefinitionID->Get(),
                        strAmount->ToLong()))  // <===============
                {
                    LogError()(OT_PRETTY_CLASS())("Failed crediting "
                                                  "stashItem for stash ")(
                        strStashName)(". instrumentDefinitionID (")(
                        strInstrumentDefinitionID.get())("), balance (")(
                        strAmount.get())(").")
                        .Flush();
                    return (-1);
                }

                // (Success)
            } else {
                LogError()(OT_PRETTY_CLASS())("Expected stashItem element.")
                    .Flush();
                return (-1);  // error condition
            }
        }  // while
    }

    if (!SkipAfterLoadingField(xml))  // </stash>
    {
        LogConsole()(OT_PRETTY_CLASS())("Bad data? Expected "
                                        "EXN_ELEMENT_END here, but "
                                        "didn't get it. Returning -1.")
            .Flush();
        return (-1);
    }

    return 1;
}

// Creates it if it's not already there.
// (*this owns it and will clean it up when destroyed.)
//
auto OTStash::GetStash(const UnallocatedCString& str_instrument_definition_id)
    -> OTStashItem*
{
    auto it = stash_items_.find(str_instrument_definition_id);

    if (stash_items_.end() == it)  // It's not already there for this
                                   // instrument definition.
    {
        const auto strInstrumentDefinitionID =
            String::Factory(str_instrument_definition_id.c_str());
        auto* pStashItem = new OTStashItem(strInstrumentDefinitionID);
        OT_ASSERT(nullptr != pStashItem);

        stash_items_.insert(std::pair<UnallocatedCString, OTStashItem*>(
            strInstrumentDefinitionID->Get(), pStashItem));
        return pStashItem;
    }

    OTStashItem* pStashItem = it->second;
    OT_ASSERT(nullptr != pStashItem);

    return pStashItem;
}

auto OTStash::GetAmount(const UnallocatedCString& str_instrument_definition_id)
    -> std::int64_t
{
    OTStashItem* pStashItem =
        GetStash(str_instrument_definition_id);  // (Always succeeds, and will
                                                 // OT_ASSERT();
                                                 // if failure.)

    return pStashItem->GetAmount();
}

auto OTStash::CreditStash(
    const UnallocatedCString& str_instrument_definition_id,
    const std::int64_t& lAmount) -> bool
{
    OTStashItem* pStashItem =
        GetStash(str_instrument_definition_id);  // (Always succeeds, and will
                                                 // OT_ASSERT();
                                                 // if failure.)

    return pStashItem->CreditStash(lAmount);
}

auto OTStash::DebitStash(
    const UnallocatedCString& str_instrument_definition_id,
    const std::int64_t& lAmount) -> bool
{
    OTStashItem* pStashItem =
        GetStash(str_instrument_definition_id);  // (Always succeeds, and will
                                                 // OT_ASSERT();
                                                 // if failure.)

    return pStashItem->DebitStash(lAmount);
}

OTStash::~OTStash()
{
    while (!stash_items_.empty()) {
        OTStashItem* pTemp = stash_items_.begin()->second;
        OT_ASSERT(nullptr != pTemp);
        delete pTemp;
        pTemp = nullptr;
        stash_items_.erase(stash_items_.begin());
    }
}
}  // namespace opentxs
