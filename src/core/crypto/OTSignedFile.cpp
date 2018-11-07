// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/core/crypto/OTSignedFile.hpp"

#include "opentxs/api/Core.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/core/util/Tag.hpp"
#include "opentxs/core/Armored.hpp"
#include "opentxs/core/Contract.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/OTStorage.hpp"
#include "opentxs/core/StringXML.hpp"
#include "opentxs/core/String.hpp"

#include <irrxml/irrXML.hpp>

#include <cstdint>
#include <cstring>
#include <ostream>
#include <string>

#define OT_METHOD "opentxs::OTSignedFile::"

namespace opentxs
{
OTSignedFile::OTSignedFile(const api::Core& core)
    : Contract(core)
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
    const api::Core& core,
    const String& LOCAL_SUBDIR,
    const String& FILE_NAME)
    : Contract(core)
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
    const api::Core& core,
    const char* LOCAL_SUBDIR,
    const String& FILE_NAME)
    : Contract(core)
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
    const api::Core& core,
    const char* LOCAL_SUBDIR,
    const char* FILE_NAME)
    : Contract(core)
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

String& OTSignedFile::GetFilePayload() { return m_strSignedFilePayload; }

void OTSignedFile::SetFilePayload(const String& strArg)
{
    m_strSignedFilePayload = strArg;
}

String& OTSignedFile::GetSignerNymID() { return m_strSignerNymID; }

void OTSignedFile::SetSignerNymID(const String& strArg)
{
    m_strSignerNymID = strArg;
}

void OTSignedFile::UpdateContents()
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

    std::string str_result;
    tag.output(str_result);

    m_xmlUnsigned->Concatenate("%s", str_result.c_str());
}

std::int32_t OTSignedFile::ProcessXMLNode(irr::io::IrrXMLReader*& xml)
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
        if (false ==
            Contract::LoadEncodedTextField(xml, m_strSignedFilePayload)) {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Error in OTSignedFile::ProcessXMLNode: filePayload field "
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
bool OTSignedFile::VerifyFile()
{
    if (m_strLocalDir->Compare(m_strPurportedLocalDir) &&
        m_strSignedFilename->Compare(m_strPurportedFilename))
        return true;

    LogOutput(OT_METHOD)(__FUNCTION__)(": Failed verifying signed file: "
                                       "Expected directory: ")(m_strLocalDir)(
        ". Found: ")(m_strPurportedLocalDir)(". Expected filename: ")(
        m_strSignedFilename)(". Found: ")(m_strPurportedFilename)(".")
        .Flush();
    return false;
}

// This is entirely separate from the Contract saving methods.  This is
// specifically
// for saving the internal file payload based on the internal file information,
// which
// this method assumes has already been set (using SetFilename())
bool OTSignedFile::SaveFile()
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
bool OTSignedFile::LoadFile()
{
    //    otOut << "DEBUG LoadFile (Signed) folder: %s file: %s \n",
    // m_strFoldername.Get(), m_strFilename.Get());

    if (OTDB::Exists(
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
                         OTLog::Path(), OTLog::PathSeparator(),
                         m_strLocalDir.Get(), OTLog::PathSeparator(),
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
