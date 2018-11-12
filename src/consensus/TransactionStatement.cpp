// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/consensus/TransactionStatement.hpp"

#include "opentxs/core/util/Tag.hpp"
#include "opentxs/core/Armored.hpp"
#include "opentxs/core/Contract.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/NumList.hpp"
#include "opentxs/core/StringXML.hpp"

#include <irrxml/irrXML.hpp>

#define OT_METHOD "opentxs::TransactionStatement::"

namespace opentxs
{
TransactionStatement::TransactionStatement(
    const std::string& notary,
    const std::set<TransactionNumber>& issued,
    const std::set<TransactionNumber>& available)
    : version_("1.0")
    , nym_id_("")
    , notary_(notary)
    , available_(available)
    , issued_(issued)
{
}

TransactionStatement::TransactionStatement(const String& serialized)
{
    auto raw =
        irr::io::createIrrXMLReader(StringXML::Factory(serialized).get());
    std::unique_ptr<irr::io::IrrXMLReader> xml(raw);

    if (!xml) { return; }

    while (xml && xml->read()) {
        const auto nodeName = String::Factory(xml->getNodeName());
        switch (xml->getNodeType()) {
            case irr::io::EXN_NONE:
            case irr::io::EXN_TEXT:
            case irr::io::EXN_COMMENT:
            case irr::io::EXN_ELEMENT_END:
            case irr::io::EXN_CDATA: {
            } break;
            case irr::io::EXN_ELEMENT: {
                if (nodeName->Compare("nymData")) {
                    version_ = xml->getAttributeValue("version");
                    nym_id_ = xml->getAttributeValue("nymID");
                } else if (nodeName->Compare("transactionNums")) {
                    notary_ = xml->getAttributeValue("notaryID");
                    auto list = String::Factory();
                    const bool loaded =
                        Contract::LoadEncodedTextField(raw, list);

                    if (notary_.empty() || !loaded) {
                        LogOutput(OT_METHOD)(__FUNCTION__)(
                            ": Error: transactionNums field without value.")
                            .Flush();
                        break;
                    }

                    NumList numlist;

                    if (!list->empty()) { numlist.Add(list); }

                    TransactionNumber number = 0;

                    while (numlist.Peek(number)) {
                        numlist.Pop();

                        LogDebug(OT_METHOD)(__FUNCTION__)(
                            ": Transaction Number ")(number)(
                            " ready-to-use for NotaryID: ")(notary_)
                            .Flush();
                        available_.insert(number);
                    }
                } else if (nodeName->Compare("issuedNums")) {
                    notary_ = xml->getAttributeValue("notaryID");
                    auto list = String::Factory();
                    const bool loaded =
                        Contract::LoadEncodedTextField(raw, list);

                    if (notary_.empty() || !loaded) {
                        LogOutput(OT_METHOD)(__FUNCTION__)(
                            ": Error: issuedNums field without value.")
                            .Flush();
                        break;
                    }

                    NumList numlist;

                    if (!list->empty()) { numlist.Add(list); }

                    TransactionNumber number = 0;

                    while (numlist.Peek(number)) {
                        numlist.Pop();

                        LogDebug(OT_METHOD)(__FUNCTION__)(
                            ": Currently liable for issued trans# ")(number)(
                            " at NotaryID: ")(notary_)
                            .Flush();
                        issued_.insert(number);
                    }
                } else {
                    LogOutput(OT_METHOD)(__FUNCTION__)(
                        ": Unknown element type in: ")(nodeName)(".")
                        .Flush();
                }
            } break;
            default: {
                LogInsane(OT_METHOD)(__FUNCTION__)(": Unknown XML type in ")(
                    nodeName)
                    .Flush();
                break;
            }
        }
    }
}

TransactionStatement::operator OTString() const
{
    auto output = String::Factory();

    Tag serialized("nymData");

    serialized.add_attribute("version", version_);
    serialized.add_attribute("nymID", nym_id_);

    if (0 < issued_.size()) {
        NumList issuedList(issued_);
        auto issued = String::Factory();
        issuedList.Output(issued);
        TagPtr issuedTag(
            new Tag("issuedNums", Armored::Factory(issued)->Get()));
        issuedTag->add_attribute("notaryID", notary_);
        serialized.add_tag(issuedTag);
    }

    if (0 < available_.size()) {
        NumList availableList(available_);
        auto available = String::Factory();
        availableList.Output(available);
        TagPtr availableTag(
            new Tag("transactionNums", Armored::Factory(available)->Get()));
        availableTag->add_attribute("notaryID", notary_);
        serialized.add_tag(availableTag);
    }

    std::string result;
    serialized.output(result);

    return String::Factory(result.c_str());
}

const std::set<TransactionNumber>& TransactionStatement::Issued() const
{
    return issued_;
}

const std::string& TransactionStatement::Notary() const { return notary_; }

void TransactionStatement::Remove(const TransactionNumber& number)
{
    available_.erase(number);
    issued_.erase(number);
}
}  // namespace opentxs
