// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/otx/consensus/TransactionStatement.hpp"  // IWYU pragma: associated

#include <irrxml/irrXML.hpp>
#include <memory>

#include "internal/core/Armored.hpp"
#include "internal/otx/common/NumList.hpp"
#include "internal/otx/common/StringXML.hpp"
#include "internal/otx/common/XML.hpp"
#include "internal/otx/common/util/Tag.hpp"
#include "internal/util/Pimpl.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::otx::context
{
TransactionStatement::TransactionStatement(
    const api::Session& api,
    const UnallocatedCString& notary,
    const UnallocatedSet<TransactionNumber>& issued,
    const UnallocatedSet<TransactionNumber>& available)
    : api_(api)
    , version_("1.0")
    , nym_id_("")
    , notary_(notary)
    , available_(available)
    , issued_(issued)
{
}

TransactionStatement::TransactionStatement(
    const api::Session& api,
    const String& serialized)
    : api_(api)
    , version_()
    , nym_id_()
    , notary_()
    , available_()
    , issued_()
{
    auto* raw =
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
                        LoadEncodedTextField(api_.Crypto(), raw, list);

                    if (notary_.empty() || !loaded) {
                        LogError()()(
                            "Error: transactionNums field without value.")
                            .Flush();
                        break;
                    }

                    NumList numlist;

                    if (!list->empty()) { numlist.Add(list); }

                    TransactionNumber number = 0;

                    while (numlist.Peek(number)) {
                        numlist.Pop();

                        LogDebug()()("Transaction Number ")(
                            number)(" ready-to-use for NotaryID: ")(notary_)
                            .Flush();
                        available_.insert(number);
                    }
                } else if (nodeName->Compare("issuedNums")) {
                    notary_ = xml->getAttributeValue("notaryID");
                    auto list = String::Factory();
                    const bool loaded =
                        LoadEncodedTextField(api_.Crypto(), raw, list);

                    if (notary_.empty() || !loaded) {
                        LogError()()("Error: issuedNums field without value.")
                            .Flush();
                        break;
                    }

                    NumList numlist;

                    if (!list->empty()) { numlist.Add(list); }

                    TransactionNumber number = 0;

                    while (numlist.Peek(number)) {
                        numlist.Pop();

                        LogDebug()()("Currently liable for issued trans# ")(
                            number)(" at NotaryID: ")(notary_)
                            .Flush();
                        issued_.insert(number);
                    }
                } else {
                    LogError()()("Unknown element type in: ")(nodeName.get())(
                        ".")
                        .Flush();
                }
            } break;
            case irr::io::EXN_UNKNOWN:
            default: {
                LogInsane()()("Unknown XML type in ")(nodeName.get()).Flush();
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
        TagPtr issuedTag(new Tag(
            "issuedNums", Armored::Factory(api_.Crypto(), issued)->Get()));
        issuedTag->add_attribute("notaryID", notary_);
        serialized.add_tag(issuedTag);
    }

    if (0 < available_.size()) {
        NumList availableList(available_);
        auto available = String::Factory();
        availableList.Output(available);
        TagPtr availableTag(new Tag(
            "transactionNums",
            Armored::Factory(api_.Crypto(), available)->Get()));
        availableTag->add_attribute("notaryID", notary_);
        serialized.add_tag(availableTag);
    }

    UnallocatedCString result;
    serialized.output(result);

    return String::Factory(result.c_str());
}

auto TransactionStatement::Issued() const
    -> const UnallocatedSet<TransactionNumber>&
{
    return issued_;
}

auto TransactionStatement::Notary() const -> const UnallocatedCString&
{
    return notary_;
}

void TransactionStatement::Remove(const TransactionNumber& number)
{
    available_.erase(number);
    issued_.erase(number);
}
}  // namespace opentxs::otx::context
