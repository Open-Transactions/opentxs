// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/otx/smartcontract/OTStash.hpp"  // IWYU pragma: associated

#include <irrxml/irrXML.hpp>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <utility>

#include "internal/core/String.hpp"
#include "internal/otx/common/XML.hpp"
#include "internal/otx/common/util/Tag.hpp"
#include "internal/otx/smartcontract/OTStashItem.hpp"
#include "internal/util/Pimpl.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs
{
OTStash::OTStash(const api::Session& api)
    : api_(api)
    , stash_name_()
    , stash_items_()
{
}

OTStash::OTStash(
    const api::Session& api,
    const UnallocatedCString& str_stash_name)
    : api_(api)
    , stash_name_(str_stash_name)
    , stash_items_()
{
}

OTStash::OTStash(
    const api::Session& api,
    const String& strInstrumentDefinitionID,
    std::int64_t lAmount)
    : api_(api)
    , stash_name_()
    , stash_items_()
{
    auto* pItem = new OTStashItem(api_, strInstrumentDefinitionID, lAmount);
    assert_false(nullptr == pItem);

    stash_items_.insert(std::pair<UnallocatedCString, OTStashItem*>(
        strInstrumentDefinitionID.Get(), pItem));
}

OTStash::OTStash(
    const api::Session& api,
    const identifier::Generic& theInstrumentDefinitionID,
    std::int64_t lAmount)
    : api_(api)
    , stash_name_()
    , stash_items_()
{
    auto* pItem = new OTStashItem(api_, theInstrumentDefinitionID, lAmount);
    assert_false(nullptr == pItem);

    auto strInstrumentDefinitionID =
        String::Factory(theInstrumentDefinitionID, api_.Crypto());

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
        assert_true(
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
        LogError()()("Failed: Empty stash 'name' "
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
                LogConsole()()("Failure: Unable to find "
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
                    LogError()()(
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
                    LogError()()("Failed crediting "
                                 "stashItem for stash ")(
                        strStashName)(". instrumentDefinitionID (")(
                        strInstrumentDefinitionID.get())("), balance (")(
                        strAmount.get())(").")
                        .Flush();
                    return (-1);
                }

                // (Success)
            } else {
                LogError()()("Expected stashItem element.").Flush();
                return (-1);  // error condition
            }
        }  // while
    }

    if (!SkipAfterLoadingField(xml))  // </stash>
    {
        LogConsole()()("Bad data? Expected "
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
        auto* pStashItem = new OTStashItem(api_, strInstrumentDefinitionID);
        assert_false(nullptr == pStashItem);

        stash_items_.insert(std::pair<UnallocatedCString, OTStashItem*>(
            strInstrumentDefinitionID->Get(), pStashItem));
        return pStashItem;
    }

    OTStashItem* pStashItem = it->second;
    assert_false(nullptr == pStashItem);

    return pStashItem;
}

auto OTStash::GetAmount(const UnallocatedCString& str_instrument_definition_id)
    -> std::int64_t
{
    OTStashItem* pStashItem =
        GetStash(str_instrument_definition_id);  // (Always succeeds, and will
                                                 // assert_true();
                                                 // if failure.)

    return pStashItem->GetAmount();
}

auto OTStash::CreditStash(
    const UnallocatedCString& str_instrument_definition_id,
    const std::int64_t& lAmount) -> bool
{
    OTStashItem* pStashItem =
        GetStash(str_instrument_definition_id);  // (Always succeeds, and will
                                                 // assert_true();
                                                 // if failure.)

    return pStashItem->CreditStash(lAmount);
}

auto OTStash::DebitStash(
    const UnallocatedCString& str_instrument_definition_id,
    const std::int64_t& lAmount) -> bool
{
    OTStashItem* pStashItem =
        GetStash(str_instrument_definition_id);  // (Always succeeds, and will
                                                 // assert_true();
                                                 // if failure.)

    return pStashItem->DebitStash(lAmount);
}

OTStash::~OTStash()
{
    while (!stash_items_.empty()) {
        OTStashItem* pTemp = stash_items_.begin()->second;
        assert_false(nullptr == pTemp);
        delete pTemp;
        pTemp = nullptr;
        stash_items_.erase(stash_items_.begin());
    }
}
}  // namespace opentxs
