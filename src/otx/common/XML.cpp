// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/otx/common/XML.hpp"  // IWYU pragma: associated

#include <array>
#include <cstring>
#include <string_view>
#include <utility>

#include "internal/core/Armored.hpp"
#include "internal/crypto/library/HashingProvider.hpp"
#include "internal/otx/common/crypto/OTSignatureMetadata.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/Pimpl.hpp"
#include "opentxs/OT.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs
{
using namespace std::literals;

auto AddBookendsAroundContent(
    String& strOutput,
    const String& strContents,
    const String& strContractType,
    const crypto::HashType hashType,
    const listOfSignatures& listSignatures) -> bool
{
    const auto begin_signature = [&] {
        auto out = String::Factory();
        out->Concatenate("-----BEGIN "sv)
            .Concatenate(strContractType)
            .Concatenate(" SIGNATURE-----\nVersion: Open Transactions "sv)
            .Concatenate(VersionString())
            .Concatenate("\nComment: http://opentransactions.org\n"sv);

        return out;
    }();
    const auto end_signature = [&] {
        auto out = String::Factory();
        out->Concatenate("\n-----END "sv)
            .Concatenate(strContractType)
            .Concatenate(" SIGNATURE-----\n\n"sv);

        return out;
    }();
    auto strTemp = [&] {
        auto out = String::Factory();
        out->Concatenate("-----BEGIN SIGNED "sv)
            .Concatenate(strContractType)
            .Concatenate("-----\nHash: "sv)
            .Concatenate(crypto::HashingProvider::HashTypeToString(hashType))
            .Concatenate("\n\n"sv);

        return out;
    }();
    strTemp->Concatenate(strContents);
    static UnallocatedCString _meta{"Meta:    CCCC\n"};  // length 14
                                                         // [9,10,11,12] index
                                                         // to put chars on
    auto meta = String::Factory(_meta);

    for (const auto& sig : listSignatures) {
        strTemp->Concatenate(begin_signature);

        if (sig->getMetaData().HasMetadata()) {
            char* ptr = const_cast<char*>(meta->Get());  // direct memory access
            *(ptr + 9) = sig->getMetaData().GetKeyType();
            *(ptr + 10) = sig->getMetaData().FirstCharNymID();
            *(ptr + 11) = sig->getMetaData().FirstCharMasterCredID();
            *(ptr + 12) = sig->getMetaData().FirstCharChildCredID();
            strTemp->Concatenate(meta);
        }

        strTemp->Concatenate(sig);  // <=== *** THE SIGNATURE ITSELF ***
        strTemp->Concatenate(end_signature);
    }

    UnallocatedCString str_Trim(strTemp->Get());
    UnallocatedCString str_Trim2 = String::trim(str_Trim);
    strOutput.Set(str_Trim2.c_str());

    return true;
}

auto DearmorAndTrim(
    const String& strInput,
    String& strOutput,
    String& strFirstLine) -> bool
{

    if (!strInput.Exists()) {
        LogError()(__func__)(": Input string is empty.").Flush();
        return false;
    }

    strOutput.Set(strInput);

    if (false == strOutput.DecodeIfArmored(false))  // bEscapedIsAllowed=true by
                                                    // default.
    {
        LogInsane()(__func__)(
            ": Input string apparently was encoded and then failed decoding. "
            "Contents: \n")(strInput)
            .Flush();

        return false;
    }

    strOutput.reset();  // for sgets

    // At this point, strOutput contains the actual contents, whether they
    // were originally ascii-armored OR NOT. (And they are also now trimmed,
    // either way.)

    std::array<char, 75> buf{};
    bool bGotLine = strOutput.sgets(buf.data(), 70);

    if (!bGotLine) { return false; }

    strFirstLine.Set(buf.data());
    strOutput.reset();  // set the "file" pointer within this string back to
                        // index 0.

    // Now I feel pretty safe -- the string I'm examining is within
    // the first 70 characters of the beginning of the contract, and
    // it will NOT contain the escape "- " sequence. From there, if
    // it contains the proper sequence, I will instantiate that type.
    if (!strFirstLine.Exists() || strFirstLine.Contains("- -")) {
        return false;
    }

    return true;
}

auto LoadEncodedTextField(irr::io::IrrXMLReader*& xml, Armored& ascOutput)
    -> bool
{
    OT_ASSERT_MSG(
        nullptr != xml, "LoadEncodedTextField -- assert: nullptr != xml");

    // const char* szFunc = "LoadEncodedTextField";

    // If we're not ALREADY on a text field, maybe there is some whitespace, so
    // let's skip ahead...
    //
    if (irr::io::EXN_TEXT != xml->getNodeType()) {
        LogTrace()(__func__)(": Skipping non-text field...").Flush();

        // move to the next node which SHOULD be the expected text field.
        if (!SkipToTextField(xml)) {
            LogDetail()(__func__)(
                ": Failure: Unable to find expected text field.")
                .Flush();
            return false;
        }

        LogTrace()(__func__)(
            ": Finished skipping non-text field. (Successfully.)")
            .Flush();
    }

    if (irr::io::EXN_TEXT == xml->getNodeType())  // SHOULD always be true, in
                                                  // fact this could be an
                                                  // assert().
    {
        auto strNodeData = String::Factory(xml->getNodeData());

        // Sometimes the XML reads up the data with a prepended newline.
        // This screws up my own objects which expect a consistent in/out
        // So I'm checking here for that prepended newline, and removing it.
        //
        char cNewline;
        if (strNodeData->Exists() && strNodeData->GetLength() > 2 &&
            strNodeData->At(0, cNewline)) {
            if ('\n' == cNewline) {
                ascOutput.Set(strNodeData->Get() + 1);
            } else {
                ascOutput.Set(strNodeData->Get());
            }

            // SkipAfterLoadingField() only skips ahead if it's not ALREADY
            // sitting on an element_end node.
            //
            xml->read();  // THIS PUTS us on the CLOSING TAG.
                          // <========================

            // The below call won't advance any further if it's ALREADY on the
            // closing tag (e.g. from the above xml->read() call.)
            if (!SkipAfterLoadingField(xml)) {
                LogDetail()(__func__)(
                    ": Bad data? Expected EXN_ELEMENT_END here, but "
                    "didn't get it. Returning false.")
                    .Flush();
                return false;
            }

            return true;
        }
    } else {
        LogDetail()(__func__)(
            ": Failure: Unable to find expected text field 2.")
            .Flush();
    }

    return false;
}

auto LoadEncodedTextField(irr::io::IrrXMLReader*& xml, String& strOutput)
    -> bool
{
    auto ascOutput = Armored::Factory();

    if (LoadEncodedTextField(xml, ascOutput) && ascOutput->GetLength() > 2) {
        return ascOutput->GetString(strOutput, true);  // linebreaks = true
    }

    return false;
}

auto LoadEncodedTextFieldByName(
    irr::io::IrrXMLReader*& xml,
    Armored& ascOutput,
    const char* szName,
    String::Map* pmapExtraVars) -> bool
{
    OT_ASSERT(nullptr != szName);

    // If we're not ALREADY on an element, maybe there is some whitespace, so
    // let's skip ahead...
    // If we're not already on a node, OR
    if ((irr::io::EXN_ELEMENT != xml->getNodeType()) ||
        // if the node's name doesn't match the one expected.
        strcmp(szName, xml->getNodeName()) != 0) {
        // move to the next node which SHOULD be the expected name.
        if (!SkipToElement(xml)) {
            LogDetail()(__func__)(
                ": Failure: Unable to find expected element: ")(szName)(".")
                .Flush();
            return false;
        }
    }

    if (irr::io::EXN_ELEMENT != xml->getNodeType())  // SHOULD always be
                                                     // ELEMENT...
    {
        LogError()(__func__)(": Error: Expected ")(
            szName)(" element with text field.")
            .Flush();
        return false;  // error condition
    }

    if (strcmp(szName, xml->getNodeName()) != 0) {
        LogError()(__func__)(": Error: missing ")(szName)(" element.").Flush();
        return false;  // error condition
    }

    // If the caller wants values for certain
    // names expected to be on this node.
    if (nullptr != pmapExtraVars) {
        String::Map& mapExtraVars = (*pmapExtraVars);

        for (auto& it : mapExtraVars) {
            UnallocatedCString first = it.first;
            auto strTemp =
                String::Factory(xml->getAttributeValue(first.c_str()));

            if (strTemp->Exists()) { mapExtraVars[first] = strTemp->Get(); }
        }
    }
    // Any attribute names passed in, now have their corresponding
    // values set on mapExtraVars (for caller.)

    if (false == LoadEncodedTextField(xml, ascOutput)) {
        LogError()(__func__)(": Error loading ")(szName)(" field.").Flush();
        return false;
    }

    return true;
}

auto LoadEncodedTextFieldByName(
    irr::io::IrrXMLReader*& xml,
    String& strOutput,
    const char* szName,
    String::Map* pmapExtraVars) -> bool
{
    OT_ASSERT(nullptr != szName);

    auto ascOutput = Armored::Factory();

    if (LoadEncodedTextFieldByName(xml, ascOutput, szName, pmapExtraVars) &&
        ascOutput->GetLength() > 2) {
        return ascOutput->GetString(strOutput, true);  // linebreaks = true
    }

    return false;
}

auto SkipAfterLoadingField(irr::io::IrrXMLReader*& xml) -> bool
{
    OT_ASSERT_MSG(
        nullptr != xml, "SkipAfterLoadingField -- assert: nullptr != xml");

    if (irr::io::EXN_ELEMENT_END !=
        xml->getNodeType())  // If we're not ALREADY on the ending element, then
                             // go there.
    {

        while (xml->read()) {
            if (xml->getNodeType() == irr::io::EXN_NONE) {
                LogDetail()(__func__)(": EXN_NONE  (Skipping).").Flush();
                continue;
            }  // SKIP
            else if (xml->getNodeType() == irr::io::EXN_COMMENT) {
                LogDetail()(__func__)(": EXN_COMMENT  (Skipping).").Flush();
                continue;
            }  // SKIP
            else if (xml->getNodeType() == irr::io::EXN_ELEMENT_END) {
                LogInsane()(__func__)(": EXN_ELEMENT_END  (success)").Flush();
                break;
            }  // Success...
            else if (xml->getNodeType() == irr::io::EXN_CDATA) {
                LogDetail()(__func__)(": EXN_CDATA  (Unexpected!).").Flush();
                return false;
            }  // Failure / Error
            else if (xml->getNodeType() == irr::io::EXN_ELEMENT) {
                LogDetail()(__func__)(": EXN_ELEMENT  (Unexpected!).").Flush();
                return false;
            }  // Failure / Error
            else if (xml->getNodeType() == irr::io::EXN_TEXT) {
                LogError()(__func__)(": EXN_TEXT (Unexpected)!").Flush();
                return false;
            }  // Failure / Error
            else {
                LogError()(__func__)(
                    ": SHOULD NEVER HAPPEN (Unknown element type)!")
                    .Flush();
                return false;
            }  // Failure / Error
        }
    }

    // else ... (already on the ending element.)
    //

    return true;
}

auto SkipToElement(irr::io::IrrXMLReader*& xml) -> bool
{
    OT_ASSERT_MSG(nullptr != xml, "SkipToElement -- assert: nullptr != xml");

    while (xml->read() && (xml->getNodeType() != irr::io::EXN_ELEMENT)) {
        //      otOut << szFunc << ": Looping to skip non-elements: currently
        // on: " << xml->getNodeName() << " \n";

        if (xml->getNodeType() == irr::io::EXN_NONE) {
            LogConsole()(__func__)(": EXN_NONE  (Skipping).").Flush();
            continue;
        }  // SKIP
        else if (xml->getNodeType() == irr::io::EXN_COMMENT) {
            LogConsole()(__func__)(": EXN_COMMENT  (Skipping).").Flush();
            continue;
        }  // SKIP
        else if (xml->getNodeType() == irr::io::EXN_ELEMENT_END)
        //        { otOut << "*** SkipToElement: EXN_ELEMENT_END
        // (ERROR)\n";  return false; }
        {
            LogDetail()(__func__)(": *** ")(": EXN_ELEMENT_END  (skipping ")(
                xml->getNodeName())(")")
                .Flush();
            continue;
        } else if (xml->getNodeType() == irr::io::EXN_CDATA) {
            LogDetail()(__func__)(": EXN_CDATA (ERROR -- unexpected CData).")
                .Flush();
            return false;
        } else if (xml->getNodeType() == irr::io::EXN_TEXT) {
            LogError()(__func__)(": EXN_TEXT.").Flush();
            return false;
        } else if (xml->getNodeType() == irr::io::EXN_ELEMENT) {
            LogDetail()(__func__)(": EXN_ELEMENT.").Flush();
            break;
        }  // (Should never happen due to while() second condition.) Still
           // returns true.
        else {
            LogError()(__func__)(
                ": SHOULD NEVER HAPPEN (Unknown element type)!")
                .Flush();
            return false;
        }  // Failure / Error
    }

    return true;
}

auto SkipToTextField(irr::io::IrrXMLReader*& xml) -> bool
{
    OT_ASSERT_MSG(nullptr != xml, "SkipToTextField -- assert: nullptr != xml");

    while (xml->read() && (xml->getNodeType() != irr::io::EXN_TEXT)) {
        if (xml->getNodeType() == irr::io::EXN_NONE) {
            LogDetail()(__func__)(": EXN_NONE  (Skipping).").Flush();
            continue;
        }  // SKIP
        else if (xml->getNodeType() == irr::io::EXN_COMMENT) {
            LogDetail()(__func__)(": EXN_COMMENT  (Skipping).").Flush();
            continue;
        }  // SKIP
        else if (xml->getNodeType() == irr::io::EXN_ELEMENT_END)
        //        { otOut << "*** SkipToTextField:
        // EXN_ELEMENT_END  (skipping)\n";  continue; }     // SKIP
        // (debugging...)
        {
            LogDetail()(__func__)(": EXN_ELEMENT_END  (ERROR).").Flush();
            return false;
        } else if (xml->getNodeType() == irr::io::EXN_CDATA) {
            LogDetail()(__func__)(": EXN_CDATA (ERROR -- unexpected CData).")
                .Flush();
            return false;
        } else if (xml->getNodeType() == irr::io::EXN_ELEMENT) {
            LogDetail()(__func__)(": EXN_ELEMENT.").Flush();
            return false;
        } else if (xml->getNodeType() == irr::io::EXN_TEXT) {
            LogError()(__func__)(": EXN_TEXT.").Flush();
            break;
        }  // (Should never happen due to while() second condition.) Still
           // returns true.
        else {
            LogError()(__func__)(
                ": SHOULD NEVER HAPPEN (Unknown element type)!")
                .Flush();
            return false;
        }  // Failure / Error
    }

    return true;
}

auto trim(const String& str) -> OTString
{
    UnallocatedCString s(str.Get(), str.GetLength());
    return String::Factory(String::trim(s));
}
}  // namespace opentxs
