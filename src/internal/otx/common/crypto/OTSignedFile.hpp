// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <irrxml/irrXML.hpp>
#include <cstdint>

#include "internal/otx/common/Contract.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/core/String.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace api
{
namespace session
{
namespace imp
{
class Factory;
}  // namespace imp
}  // namespace session

class Session;
}  // namespace api

class PasswordPrompt;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs
{
class OTSignedFile : public Contract
{
public:
    auto LoadFile() -> bool;
    auto SaveFile() -> bool;
    auto VerifyFile() -> bool;  // Returns true or false, whether actual
                                // subdir/file matches purported subdir/file.
    // (You should still verify the signature on it as well, if you are doing
    // this.)
    void SetFilename(const String& LOCAL_SUBDIR, const String& FILE_NAME);
    auto GetFilePayload() -> String&;
    void SetFilePayload(const String& strArg);
    auto GetSignerNymID() -> String&;
    void SetSignerNymID(const String& strArg);
    ~OTSignedFile() override;
    void Release() override;
    void Release_SignedFile();
    void UpdateContents(const PasswordPrompt& reason) override;

    OTSignedFile() = delete;

protected:
    OTString signed_file_payload_;  // This class exists to wrap another and
                                    // save it in signed form.
    // The "payload" (the wrapped contents) are stored in this member.

    OTString local_dir_;  // The local subdirectory where the file is, such
                          // as "nyms" or "certs"
    OTString signed_filename_;  // The file stores its own name. Later, when
                                // loading it back up, you can
    // see that the name matches internally, and that the signature matches,
    // therefore, no one has switched the file or meddled with its contents.

    OTString purported_local_dir_;  // This is the subdirectory according to
                                    // the file.
    OTString purported_filename_;   // This is the filename according to the
                                    // file.

    OTString signer_nym_id_;  // Optional. Here in case you ever
                              // want to use it.

    // THOUGHT: What if someone switched the file for an older version of
    // itself? Seems to me that he could
    // make the server accept the file, in that case. Like maybe an account file
    // with a higher balance?
    // Similarly, what if someone erased a spent token file? Then the software
    // would accept it as a new
    // token once again. Also, the cash account would be deducted twice for the
    // same token, which means it
    // would no longer contain enough to cover all the tokens...
    // Therefore it seems to me that, even with the files signed, there are
    // still attacks possible when
    // the attacker has write/erase access to the filesystem. I'd like to make
    // it impervious even to that.

    auto ProcessXMLNode(irr::io::IrrXMLReader*& xml) -> std::int32_t override;

private:  // Private prevents erroneous use by other classes.
    friend api::session::imp::Factory;

    using ot_super = Contract;

    // These assume SetFilename() was already called,
    // or at least one of the constructors that uses it.
    //
    explicit OTSignedFile(const api::Session& api);
    explicit OTSignedFile(
        const api::Session& api,
        const String& LOCAL_SUBDIR,
        const String& FILE_NAME);
    explicit OTSignedFile(
        const api::Session& api,
        const char* LOCAL_SUBDIR,
        const String& FILE_NAME);
    explicit OTSignedFile(
        const api::Session& api,
        const char* LOCAL_SUBDIR,
        const char* FILE_NAME);
};
}  // namespace opentxs
