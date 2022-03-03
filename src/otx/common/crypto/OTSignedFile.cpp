// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated
#include "internal/otx/common/crypto/OTSignedFile.hpp"  // IWYU pragma: associated

#include <cstdint>
#include <cstring>

#include "internal/otx/common/Contract.hpp"
#include "internal/otx/common/StringXML.hpp"
#include "internal/otx/common/XML.hpp"
#include "internal/otx/common/util/Tag.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/core/Armored.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "otx/common/OTStorage.hpp"

namespace opentxs
{
OTSignedFile::OTSignedFile(const api::Session& api)
    : Contract(api)
    , m_strSignedFilePayload(String::Factory())
    , m_strLocalDir(String::Factory())
    , m_strSignedFilename(String::Factory())
    , m_strPurportedLocalDir(String::Factory())
    , m_strPurportedFilename(String::Factory())
    , m_strSignerNymID(String::Factory())
{
    m_strContractType->Set("FILE");
}

OTSignedFile::OTSignedFile(
    const api::Session& api,
    const String& LOCAL_SUBDIR,
    const String& FILE_NAME)
    : Contract(api)
    , m_strSignedFilePayload(String::Factory())
    , m_strLocalDir(String::Factory())
    , m_strSignedFilename(String::Factory())
    , m_strPurportedLocalDir(String::Factory())
    , m_strPurportedFilename(String::Factory())
    , m_strSignerNymID(String::Factory())
{
    m_strContractType->Set("FILE");

    SetFilename(LOCAL_SUBDIR, FILE_NAME);
}

OTSignedFile::OTSignedFile(
    const api::Session& api,
    const char* LOCAL_SUBDIR,
    const String& FILE_NAME)
    : Contract(api)
    , m_strSignedFilePayload(String::Factory())
    , m_strLocalDir(String::Factory())
    , m_strSignedFilename(String::Factory())
    , m_strPurportedLocalDir(String::Factory())
    , m_strPurportedFilename(String::Factory())
    , m_strSignerNymID(String::Factory())
{
    m_strContractType->Set("FILE");

    auto strLocalSubdir = String::Factory(LOCAL_SUBDIR);

    SetFilename(strLocalSubdir, FILE_NAME);
}

OTSignedFile::OTSignedFile(
    const api::Session& api,
    const char* LOCAL_SUBDIR,
    const char* FILE_NAME)
    : Contract(api)
    , m_strSignedFilePayload(String::Factory())
    , m_strLocalDir(String::Factory())
    , m_strSignedFilename(String::Factory())
    , m_strPurportedLocalDir(String::Factory())
    , m_strPurportedFilename(String::Factory())
    , m_strSignerNymID(String::Factory())
{
    m_strContractType->Set("FILE");

    auto strLocalSubdir = String::Factory(LOCAL_SUBDIR),
         strFile_Name = String::Factory(FILE_NAME);

    SetFilename(strLocalSubdir, strFile_Name);
}

auto OTSignedFile::GetFilePayload() -> String&
{
    return m_strSignedFilePayload;
}

void OTSignedFile::SetFilePayload(const String& strArg)
{
    m_strSignedFilePayload = strArg;
}

auto OTSignedFile::GetSignerNymID() -> String& { return m_strSignerNymID; }

void OTSignedFile::SetSignerNymID(const String& strArg)
{
    m_strSignerNymID = strArg;
}

void OTSignedFile::UpdateContents(const PasswordPrompt& reason)
{
    // I release this because I'm about to repopulate it.
    m_xmlUnsigned->Release();

    Tag tag("signedFile");

    tag.add_attribute("version", m_strVersion->Get());
    tag.add_attribute("localDir", m_strLocalDir->Get());
    tag.add_attribute("filename", m_strSignedFilename->Get());

    if (m_strSignerNymID->Exists()) {
        tag.add_attribute("signer", m_strSignerNymID->Get());
    }

    if (m_strSignedFilePayload->Exists()) {
        auto ascPayload = Armored::Factory(m_strSignedFilePayload);
        tag.add_tag("filePayload", ascPayload->Get());
    }

    UnallocatedCString str_result;
    tag.output(str_result);

    m_xmlUnsigned->Concatenate(String::Factory(str_result));
}

auto OTSignedFile::ProcessXMLNode(irr::io::IrrXMLReader*& xml) -> std::int32_t
{
    std::int32_t nReturnVal = 0;

    // Here we call the parent class first.
    // If the node is found there, or there is some error,
    // then we just return either way.  But if it comes back
    // as '0', then nothing happened, and we'll continue executing.
    //
    // -- Note you can choose not to call the parent if
    // you don't want to use any of those xml tags.
    // As I do below, in the case of OTAccount.
    // if (nReturnVal = ot_super::ProcessXMLNode(xml))
    //    return nReturnVal;

    if (!strcmp("signedFile", xml->getNodeName())) {
        m_strVersion = String::Factory(xml->getAttributeValue("version"));

        m_strPurportedLocalDir =
            String::Factory(xml->getAttributeValue("localDir"));
        m_strPurportedFilename =
            String::Factory(xml->getAttributeValue("filename"));
        m_strSignerNymID = String::Factory(xml->getAttributeValue("signer"));

        nReturnVal = 1;
    } else if (!strcmp("filePayload", xml->getNodeName())) {
        if (false == LoadEncodedTextField(xml, m_strSignedFilePayload)) {
            LogError()(OT_PRETTY_CLASS())(
                "Error in OTSignedFile::ProcessXMLNode: filePayload field "
                "without value.")
                .Flush();
            return (-1);  // error condition
        }

        return 1;
    }

    return nReturnVal;
}

// We just loaded a certain subdirectory/filename
// This file also contains that information within it.
// This function allows me to compare the two and make sure
// the file that I loaded is what it claims to be.
//
// Make sure you also VerifySignature() whenever doing something
// like this  :-)
//
// Assumes SetFilename() has been set, and that LoadFile() has just been called.
auto OTSignedFile::VerifyFile() -> bool
{
    if (m_strLocalDir->Compare(m_strPurportedLocalDir) &&
        m_strSignedFilename->Compare(m_strPurportedFilename))
        return true;

    LogError()(OT_PRETTY_CLASS())(
        "Failed verifying signed file: "
        "Expected directory: ")(m_strLocalDir)(". Found: ")(
        m_strPurportedLocalDir)(". Expected filename: ")(
        m_strSignedFilename)(". Found: ")(m_strPurportedFilename)(".")
        .Flush();
    return false;
}

// This is entirely separate from the Contract saving methods.  This is
// specifically
// for saving the internal file payload based on the internal file information,
// which
// this method assumes has already been set (using SetFilename())
auto OTSignedFile::SaveFile() -> bool
{
    const auto strTheFileName(m_strFilename);
    const auto strTheFolderName(m_strFoldername);

    // Contract doesn't natively make it easy to save a contract to its own
    // filename.
    // Funny, I know, but Contract is designed to save either to a specific
    // filename,
    // or to a string parameter, or to the internal rawfile member. It doesn't
    // normally
    // save to its own filename that was used to load it. But OTSignedFile is
    // different...

    // This saves to a file, the name passed in as a char *.
    return SaveContract(strTheFolderName->Get(), strTheFileName->Get());
}

// Assumes SetFilename() has already been set.
auto OTSignedFile::LoadFile() -> bool
{
    if (OTDB::Exists(
            api_,
            api_.DataFolder(),
            m_strFoldername->Get(),
            m_strFilename->Get(),
            "",
            ""))
        return LoadContract();

    return false;
}

void OTSignedFile::SetFilename(
    const String& LOCAL_SUBDIR,
    const String& FILE_NAME)
{
    // OTSignedFile specific variables.
    m_strLocalDir = LOCAL_SUBDIR;
    m_strSignedFilename = FILE_NAME;

    // Contract variables.
    m_strFoldername = m_strLocalDir;
    m_strFilename = m_strSignedFilename;

    /*
    m_strFilename.Format("%s%s" // data_folder/
                         "%s%s" // nyms/
                         "%s",  // 5bf9a88c.nym
                         OTLog::Path(), OTapi::Legacy::PathSeparator(),
                         m_strLocalDir.Get(), OTapi::Legacy::PathSeparator(),
                         m_strSignedFilename.Get());
    */
    // Software Path + Local Sub-directory + Filename
    //
    // Finished Product:    "transaction/nyms/5bf9a88c.nym"
}

void OTSignedFile::Release_SignedFile()
{
    m_strSignedFilePayload->Release();  // This is the file contents we were
                                        // wrapping.
                                        // We can release this now.

    //  m_strLocalDir.Release();          // We KEEP these, *not* release,
    //  because LoadContract()
    //  m_strSignedFilename.Release();    // calls Release(), and these are our
    //  core values. We
    // don't want to lose them when the file is loaded.

    // Note: Additionally, neither does Contract release m_strFilename here,
    // for the SAME reason.

    m_strPurportedLocalDir->Release();
    m_strPurportedFilename->Release();
}

void OTSignedFile::Release()
{
    Release_SignedFile();

    Contract::Release();

    m_strContractType->Set("FILE");
}

OTSignedFile::~OTSignedFile() { Release_SignedFile(); }
}  // namespace opentxs
