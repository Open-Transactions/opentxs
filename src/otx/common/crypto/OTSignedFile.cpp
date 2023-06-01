// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/otx/common/crypto/OTSignedFile.hpp"  // IWYU pragma: associated

#include <cstdint>
#include <cstring>
#include <filesystem>

#include "internal/core/Armored.hpp"
#include "internal/core/String.hpp"
#include "internal/otx/common/Contract.hpp"
#include "internal/otx/common/StringXML.hpp"
#include "internal/otx/common/XML.hpp"
#include "internal/otx/common/util/Tag.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/Pimpl.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "otx/common/OTStorage.hpp"

namespace opentxs
{
OTSignedFile::OTSignedFile(const api::Session& api)
    : Contract(api)
    , signed_file_payload_(String::Factory())
    , local_dir_(String::Factory())
    , signed_filename_(String::Factory())
    , purported_local_dir_(String::Factory())
    , purported_filename_(String::Factory())
    , signer_nym_id_(String::Factory())
{
    contract_type_->Set("FILE");
}

OTSignedFile::OTSignedFile(
    const api::Session& api,
    const String& LOCAL_SUBDIR,
    const String& FILE_NAME)
    : Contract(api)
    , signed_file_payload_(String::Factory())
    , local_dir_(String::Factory())
    , signed_filename_(String::Factory())
    , purported_local_dir_(String::Factory())
    , purported_filename_(String::Factory())
    , signer_nym_id_(String::Factory())
{
    contract_type_->Set("FILE");

    SetFilename(LOCAL_SUBDIR, FILE_NAME);
}

OTSignedFile::OTSignedFile(
    const api::Session& api,
    const char* LOCAL_SUBDIR,
    const String& FILE_NAME)
    : Contract(api)
    , signed_file_payload_(String::Factory())
    , local_dir_(String::Factory())
    , signed_filename_(String::Factory())
    , purported_local_dir_(String::Factory())
    , purported_filename_(String::Factory())
    , signer_nym_id_(String::Factory())
{
    contract_type_->Set("FILE");

    auto strLocalSubdir = String::Factory(LOCAL_SUBDIR);

    SetFilename(strLocalSubdir, FILE_NAME);
}

OTSignedFile::OTSignedFile(
    const api::Session& api,
    const char* LOCAL_SUBDIR,
    const char* FILE_NAME)
    : Contract(api)
    , signed_file_payload_(String::Factory())
    , local_dir_(String::Factory())
    , signed_filename_(String::Factory())
    , purported_local_dir_(String::Factory())
    , purported_filename_(String::Factory())
    , signer_nym_id_(String::Factory())
{
    contract_type_->Set("FILE");

    auto strLocalSubdir = String::Factory(LOCAL_SUBDIR),
         strFile_Name = String::Factory(FILE_NAME);

    SetFilename(strLocalSubdir, strFile_Name);
}

auto OTSignedFile::GetFilePayload() -> String& { return signed_file_payload_; }

void OTSignedFile::SetFilePayload(const String& strArg)
{
    signed_file_payload_ = strArg;
}

auto OTSignedFile::GetSignerNymID() -> String& { return signer_nym_id_; }

void OTSignedFile::SetSignerNymID(const String& strArg)
{
    signer_nym_id_ = strArg;
}

void OTSignedFile::UpdateContents(const PasswordPrompt& reason)
{
    // I release this because I'm about to repopulate it.
    xml_unsigned_->Release();

    Tag tag("signedFile");

    tag.add_attribute("version", version_->Get());
    tag.add_attribute("localDir", local_dir_->Get());
    tag.add_attribute("filename", signed_filename_->Get());

    if (signer_nym_id_->Exists()) {
        tag.add_attribute("signer", signer_nym_id_->Get());
    }

    if (signed_file_payload_->Exists()) {
        auto ascPayload = Armored::Factory(api_.Crypto(), signed_file_payload_);
        tag.add_tag("filePayload", ascPayload->Get());
    }

    UnallocatedCString str_result;
    tag.output(str_result);

    xml_unsigned_->Concatenate(String::Factory(str_result));
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
        version_ = String::Factory(xml->getAttributeValue("version"));

        purported_local_dir_ =
            String::Factory(xml->getAttributeValue("localDir"));
        purported_filename_ =
            String::Factory(xml->getAttributeValue("filename"));
        signer_nym_id_ = String::Factory(xml->getAttributeValue("signer"));

        nReturnVal = 1;
    } else if (!strcmp("filePayload", xml->getNodeName())) {
        if (false ==
            LoadEncodedTextField(api_.Crypto(), xml, signed_file_payload_)) {
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
    if (local_dir_->Compare(purported_local_dir_) &&
        signed_filename_->Compare(purported_filename_)) {
        return true;
    }

    LogError()(OT_PRETTY_CLASS())("Failed verifying signed file: "
                                  "Expected directory: ")(local_dir_.get())(
        ". Found: ")(purported_local_dir_.get())(". Expected filename: ")(
        signed_filename_.get())(". Found: ")(purported_filename_.get())(".")
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
    const auto strTheFileName(filename_);
    const auto strTheFolderName(foldername_);

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
            api_.DataFolder().string(),
            foldername_->Get(),
            filename_->Get(),
            "",
            "")) {
        return LoadContract();
    }

    return false;
}

void OTSignedFile::SetFilename(
    const String& LOCAL_SUBDIR,
    const String& FILE_NAME)
{
    // OTSignedFile specific variables.
    local_dir_ = LOCAL_SUBDIR;
    signed_filename_ = FILE_NAME;

    // Contract variables.
    foldername_ = local_dir_;
    filename_ = signed_filename_;

    /*
    filename_.Format("%s%s" // data_folder/
                         "%s%s" // nyms/
                         "%s",  // 5bf9a88c.nym
                         OTLog::Path(), OTapi::Legacy::PathSeparator(),
                         local_dir_.Get(), OTapi::Legacy::PathSeparator(),
                         signed_filename_.Get());
    */
    // Software Path + Local Sub-directory + Filename
    //
    // Finished Product:    "transaction/nyms/5bf9a88c.nym"
}

void OTSignedFile::Release_SignedFile()
{
    signed_file_payload_->Release();  // This is the file contents we were
                                      // wrapping.
                                      // We can release this now.

    //  local_dir_.Release();          // We KEEP these, *not* release,
    //  because LoadContract()
    //  signed_filename_.Release();    // calls Release(), and these are our
    //  core values. We
    // don't want to lose them when the file is loaded.

    // Note: Additionally, neither does Contract release filename_ here,
    // for the SAME reason.

    purported_local_dir_->Release();
    purported_filename_->Release();
}

void OTSignedFile::Release()
{
    Release_SignedFile();

    Contract::Release();

    contract_type_->Set("FILE");
}

OTSignedFile::~OTSignedFile() { Release_SignedFile(); }
}  // namespace opentxs
