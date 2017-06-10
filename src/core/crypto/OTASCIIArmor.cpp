/************************************************************
 *
 *                 OPEN TRANSACTIONS
 *
 *       Financial Cryptography and Digital Cash
 *       Library, Protocol, API, Server, CLI, GUI
 *
 *       -- Anonymous Numbered Accounts.
 *       -- Untraceable Digital Cash.
 *       -- Triple-Signed Receipts.
 *       -- Cheques, Vouchers, Transfers, Inboxes.
 *       -- Basket Currencies, Markets, Payment Plans.
 *       -- Signed, XML, Ricardian-style Contracts.
 *       -- Scripted smart contracts.
 *
 *  EMAIL:
 *  fellowtraveler@opentransactions.org
 *
 *  WEBSITE:
 *  http://www.opentransactions.org/
 *
 *  -----------------------------------------------------
 *
 *   LICENSE:
 *   This Source Code Form is subject to the terms of the
 *   Mozilla Public License, v. 2.0. If a copy of the MPL
 *   was not distributed with this file, You can obtain one
 *   at http://mozilla.org/MPL/2.0/.
 *
 *   DISCLAIMER:
 *   This program is distributed in the hope that it will
 *   be useful, but WITHOUT ANY WARRANTY; without even the
 *   implied warranty of MERCHANTABILITY or FITNESS FOR A
 *   PARTICULAR PURPOSE.  See the Mozilla Public License
 *   for more details.
 *
 ************************************************************/

#include "opentxs/core/stdafx.hpp"

#include "opentxs/core/crypto/OTASCIIArmor.hpp"

#include "opentxs/api/OT.hpp"
#include "opentxs/core/crypto/CryptoEncodingEngine.hpp"
#include "opentxs/core/crypto/CryptoEngine.hpp"
#include "opentxs/core/crypto/OTEnvelope.hpp"
#include "opentxs/core/util/Assert.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/OTStorage.hpp"
#include "opentxs/core/String.hpp"

#include <stdint.h>
#include <sys/types.h>
#include <zconf.h>
#include <zlib.h>
#include <cstring>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>

namespace opentxs
{

const char* OT_BEGIN_ARMORED = "-----BEGIN OT ARMORED";
const char* OT_END_ARMORED = "-----END OT ARMORED";

const char* OT_BEGIN_ARMORED_escaped = "- -----BEGIN OT ARMORED";
const char* OT_END_ARMORED_escaped = "- -----END OT ARMORED";

const char* OT_BEGIN_SIGNED = "-----BEGIN SIGNED";
const char* OT_BEGIN_SIGNED_escaped = "- -----BEGIN SIGNED";

// Let's say you don't know if the input string is raw base64, or if it has
// bookends
// on it like -----BEGIN BLAH BLAH ...
// And if it DOES have Bookends, you don't know if they are escaped:  -
// -----BEGIN ...
// Let's say you just want an easy function that will figure that crap out, and
// load the
// contents up properly into an OTASCIIArmor object. (That's what this function
// will do.)
//
// str_bookend is a default.
// So you could make it more specific like, -----BEGIN ENCRYPTED KEY (or
// whatever.)
//
// static
bool OTASCIIArmor::LoadFromString(
    OTASCIIArmor& ascArmor,
    const String& strInput,
    std::string str_bookend)
{

    if (strInput.Contains(String(str_bookend)))  // YES there are bookends
                                                 // around this.
    {
        const std::string str_escaped("- " + str_bookend);

        const bool bEscaped = strInput.Contains(String(str_escaped));

        String strLoadFrom(strInput);

        if (!ascArmor.LoadFromString(strLoadFrom, bEscaped))  // removes the
                                                              // bookends so we
                                                              // have JUST the
                                                              // coded part.
        {
            //          otErr << "%s: Failure loading string into OTASCIIArmor
            // object:\n\n%s\n\n",
            //                        __FUNCTION__, strInput.Get());
            return false;
        }
    } else
        ascArmor.Set(strInput.Get());

    return true;
}

// initializes blank.
OTASCIIArmor::OTASCIIArmor()
    : String()
{
}

// encodes
OTASCIIArmor::OTASCIIArmor(const String& strValue)
    : String(/*Don't pass here, since we're encoding.*/)
{
    SetString(strValue);
}

// encodes
OTASCIIArmor::OTASCIIArmor(const Data& theValue)
    : String()
{
    SetData(theValue);
}

// Copies (already encoded)
OTASCIIArmor::OTASCIIArmor(const OTASCIIArmor& strValue)
    : String(dynamic_cast<const String&>(strValue))
{
}

// assumes envelope contains encrypted data;
// grabs that data in base64-form onto *this.
OTASCIIArmor::OTASCIIArmor(const OTEnvelope& theEnvelope)
    : String()
{
    theEnvelope.GetCiphertext(*this);
}

// copies (already encoded)
OTASCIIArmor::OTASCIIArmor(const char* szValue)
    : String(szValue)
{
}

// copies, assumes already encoded.
OTASCIIArmor& OTASCIIArmor::operator=(const char* szValue)
{
    Set(szValue);
    return *this;
}

// encodes
OTASCIIArmor& OTASCIIArmor::operator=(const String& strValue)
{
    if ((&strValue) != (&(dynamic_cast<const String&>(*this)))) {
        SetString(strValue);
    }
    return *this;
}

// encodes
OTASCIIArmor& OTASCIIArmor::operator=(const Data& theValue)
{
    SetData(theValue);
    return *this;
}

// assumes is already encoded and just copies the encoded text
OTASCIIArmor& OTASCIIArmor::operator=(const OTASCIIArmor& strValue)
{
    if ((&strValue) != this)  // prevent self-assignment
    {
        String::operator=(dynamic_cast<const String&>(strValue));
    }
    return *this;
}

// Source for these two functions: http://panthema.net/2007/0328-ZLibString.html

/** Compress a STL string using zlib with given compression level and return
 * the binary data. */
std::string OTASCIIArmor::compress_string(
    const std::string& str,
    int32_t compressionlevel = Z_BEST_COMPRESSION) const
{
    z_stream zs;  // z_stream is zlib's control structure
    memset(&zs, 0, sizeof(zs));

    if (deflateInit(&zs, compressionlevel) != Z_OK)
        throw(std::runtime_error("deflateInit failed while compressing."));

    zs.next_in = reinterpret_cast<Bytef*>(const_cast<char*>(str.data()));
    zs.avail_in = static_cast<uInt>(str.size());  // set the z_stream's input

    int32_t ret;
    char outbuffer[32768];
    std::string outstring;

    // retrieve the compressed bytes blockwise
    do {
        zs.next_out = reinterpret_cast<Bytef*>(outbuffer);
        zs.avail_out = sizeof(outbuffer);

        ret = deflate(&zs, Z_FINISH);

        if (outstring.size() < zs.total_out) {
            // append the block to the output string
            outstring.append(outbuffer, zs.total_out - outstring.size());
        }
    } while (ret == Z_OK);

    deflateEnd(&zs);

    if (ret != Z_STREAM_END) {  // an error occurred that was not EOF
        std::ostringstream oss;
        oss << "Exception during zlib compression: (" << ret << ") " << zs.msg;
        throw(std::runtime_error(oss.str()));
    }

    return outstring;
}

/** Decompress an STL string using zlib and return the original data. */
std::string OTASCIIArmor::decompress_string(const std::string& str) const
{
    z_stream zs;  // z_stream is zlib's control structure
    memset(&zs, 0, sizeof(zs));

    if (inflateInit(&zs) != Z_OK)
        throw(std::runtime_error("inflateInit failed while decompressing."));

    zs.next_in = reinterpret_cast<Bytef*>(const_cast<char*>(str.data()));
    zs.avail_in = static_cast<uInt>(str.size());

    int32_t ret;
    char outbuffer[32768];
    std::string outstring;

    // get the decompressed bytes blockwise using repeated calls to inflate
    do {
        zs.next_out = reinterpret_cast<Bytef*>(outbuffer);
        zs.avail_out = sizeof(outbuffer);

        ret = inflate(&zs, 0);

        if (outstring.size() < zs.total_out) {
            outstring.append(outbuffer, zs.total_out - outstring.size());
        }

    } while (ret == Z_OK);

    inflateEnd(&zs);

    if (ret != Z_STREAM_END) {  // an error occurred that was not EOF
        std::ostringstream oss;
        oss << "Exception during zlib decompression: (" << ret << ")";
        if (zs.msg != nullptr) {
            oss << " " << zs.msg;
        }
        throw(std::runtime_error(oss.str()));
    }

    return outstring;
}

// Base64-decode
bool OTASCIIArmor::GetData(
    Data& theData,
    bool bLineBreaks) const  // linebreaks=true
{
    theData.Release();

    if (GetLength() < 1) return true;

    auto decoded =
        OT::App().Crypto().Encode().DataDecode(std::string(Get(), GetLength()));

    theData.Assign(decoded.c_str(), decoded.size());

    return (0 < decoded.size());
}

// Base64-encode
bool OTASCIIArmor::SetData(const Data& theData, bool bLineBreaks)
{
    Release();

    if (theData.GetSize() < 1) return true;

    auto string =
        OT::App().Crypto().Encode().DataEncode(theData);

    if (string.empty()) {
        otErr << __FUNCTION__ << "Base64Encode failed" << std::endl;

        return false;
    }

    Set(string.data(), string.size());

    return true;
}

// Base64-decode and decompress
bool OTASCIIArmor::GetString(String& strData, bool bLineBreaks) const
{
    strData.Release();

    if (GetLength() < 1) {
        return true;
    }

    std::string str_decoded = OT::App().Crypto().Encode().DataDecode(Get());

    if (str_decoded.empty()) {
        otErr << __FUNCTION__ << "Base58CheckDecode failed." << std::endl;

        return false;
    }

    std::string str_uncompressed;
    try {
        str_uncompressed = decompress_string(str_decoded);
    } catch (const std::runtime_error&) {
        otErr << __FUNCTION__ << ": decompress failed" << std::endl;

        return false;
    }

    strData.Set(str_uncompressed.c_str(), str_uncompressed.length());

    return true;
}

// Compress and Base64-encode
bool OTASCIIArmor::SetString(const String& strData, bool bLineBreaks)  //=true
{
    Release();

    if (strData.GetLength() < 1) return true;

    std::string str_compressed = compress_string(strData.Get());

    // "Success"
    if (str_compressed.size() == 0) {
        otErr << "OTASCIIArmor::" << __FUNCTION__ << ": compression failed."
              << std::endl;

        return false;
    }

    auto pString = OT::App().Crypto().Encode().DataEncode(str_compressed);

    if (pString.empty()) {
        otErr << "OTASCIIArmor::" << __FUNCTION__ << ": Base64Encode failed."
              << std::endl;

        return false;
    }

    Set(pString.data(), pString.size());

    return true;
}

// This code reads up the file, discards the bookends, and saves only the
// gibberish itself.
bool OTASCIIArmor::LoadFromFile(
    const String& foldername,
    const String& filename)
{
    OT_ASSERT(foldername.Exists());
    OT_ASSERT(filename.Exists());

    if (!OTDB::Exists(foldername.Get(), filename.Get())) {
        otErr << "OTASCIIArmor::LoadFromFile: File does not exist: "
              << foldername << "" << Log::PathSeparator() << "" << filename
              << "\n";
        return false;
    }

    String strFileContents(
        OTDB::QueryPlainString(foldername.Get(), filename.Get()));  // <===
                                                                    // LOADING
                                                                    // FROM DATA
                                                                    // STORE.

    if (strFileContents.GetLength() < 2) {
        otErr << "OTASCIIArmor::LoadFromFile: Error reading file: "
              << foldername << Log::PathSeparator() << filename << "\n";
        return false;
    }

    return LoadFromString(strFileContents);
}

bool OTASCIIArmor::LoadFromExactPath(const std::string& filename)
{
    std::ifstream fin(filename.c_str(), std::ios::binary);

    if (!fin.is_open()) {
        otWarn << "OTASCIIArmor::LoadFromExactPath: Failed opening file: "
               << filename << "\n";
        return false;
    }

    return LoadFrom_ifstream(fin);
}

// This code reads up the file, discards the bookends, and saves only the
// gibberish itself.
bool OTASCIIArmor::LoadFrom_ifstream(std::ifstream& fin)
{
    std::stringstream buffer;
    buffer << fin.rdbuf();

    std::string contents(buffer.str());

    String theString;
    theString.Set(contents.c_str());

    return LoadFromString(theString);
}

bool OTASCIIArmor::SaveToExactPath(const std::string& filename)
{
    std::ofstream fout(filename.c_str(), std::ios::out | std::ios::binary);

    if (!fout.is_open()) {
        otWarn << "OTASCIIArmor::SaveToExactPath: Failed opening file: "
               << filename << "\n";
        return false;
    }

    return SaveTo_ofstream(fout);
}

bool OTASCIIArmor::SaveTo_ofstream(std::ofstream& fout)
{
    String strOutput;
    std::string str_type("DATA");  // -----BEGIN OT ARMORED DATA-----

    if (WriteArmoredString(strOutput, str_type) && strOutput.Exists()) {
        // WRITE IT TO THE FILE
        //
        fout << strOutput;

        if (fout.fail()) {
            otErr << __FUNCTION__ << ": Failed saving to file.\n Contents:\n\n"
                  << strOutput << "\n\n";
            return false;
        }

        return true;
    }

    return false;
}

// const char * OT_BEGIN_ARMORED   = "-----BEGIN OT ARMORED";
// const char * OT_END_ARMORED     =   "-----END OT ARMORED";

bool OTASCIIArmor::WriteArmoredFile(
    const String& foldername,
    const String& filename,
    const  // for "-----BEGIN OT LEDGER-----", str_type would contain "LEDGER"
    std::string str_type,  // There's no default, to force you to enter the
                           // right
                           // string.
    bool bEscaped) const
{
    OT_ASSERT(foldername.Exists());
    OT_ASSERT(filename.Exists());

    String strOutput;

    if (WriteArmoredString(strOutput, str_type, bEscaped) &&
        strOutput.Exists()) {
        // WRITE IT TO THE FILE
        // StorePlainString will attempt to create all the folders leading up to
        // the path
        // for the output file.
        //
        bool bSaved = OTDB::StorePlainString(
            strOutput.Get(), foldername.Get(), filename.Get());

        if (!bSaved) {
            otErr << "OTASCIIArmor::WriteArmoredFile"
                  << ": Failed saving to file: %s%s%s\n\n Contents:\n\n"
                  << strOutput << "\n\n",
                foldername.Get(), Log::PathSeparator(), filename.Get();
            return false;
        }

        return true;
    }

    return false;
}

// const char * OT_BEGIN_ARMORED   = "-----BEGIN OT ARMORED";
// const char * OT_END_ARMORED     =   "-----END OT ARMORED";

bool OTASCIIArmor::WriteArmoredString(
    String& strOutput,
    const  // for "-----BEGIN OT LEDGER-----", str_type would contain "LEDGER"
    std::string str_type,  // There's no default, to force you to enter the
                           // right
                           // string.
    bool bEscaped) const
{
    const char* szEscape = "- ";

    String strTemp;
    strTemp.Format(
        "%s%s %s-----\n"  // "%s-----BEGIN OT ARMORED %s-----\n"
        "Version: Open Transactions %s\n"
        "Comment: "
        "http://opentransactions.org\n\n"  // todo
        // hardcoding.
        "%s\n"
        "%s%s %s-----\n\n",  // "%s-----END OT ARMORED %s-----\n"
        bEscaped ? szEscape : "",
        OT_BEGIN_ARMORED,
        str_type.c_str(),  // "%s%s %s-----\n"
        Log::Version(),    // "Version: Open Transactions %s\n"
        /* No variable */  // "Comment:
        // http://github.com/FellowTraveler/Open-Transactions/wiki\n\n",
        Get(),  //  "%s"     <==== CONTENTS OF THIS OBJECT BEING
                // WRITTEN...
        bEscaped ? szEscape : "",
        OT_END_ARMORED,
        str_type.c_str());  // "%s%s %s-----\n"

    strOutput.Concatenate("%s", strTemp.Get());

    return true;
}

// This code reads up the string, discards the bookends, and saves only the
// gibberish itself.
// the bEscaped option allows you to load a normal ASCII-Armored file if off,
// and allows
// you to load an escaped ASCII-armored file (such as inside the contracts when
// the public keys
// are escaped with a "- " before the rest of the ------- starts.)
//
bool OTASCIIArmor::LoadFromString(
    String& theStr,  // input
    bool bEscaped,
    const  // This szOverride sub-string determines
    // where the content starts, when loading.
    std::string str_override)  // Default is
                               // "-----BEGIN"
{
    // Should never be 0 size, as default is "-----BEGIN"
    // But if you want to load a private key, try "-----BEGIN ENCRYPTED PRIVATE"
    // instead.
    // *smile*
    const std::string str_end_line =
        "-----END";  // Someday maybe allow parameterized option for this.

    const int32_t nBufSize = 2100;   // todo: hardcoding
    const int32_t nBufSize2 = 2048;  // todo: hardcoding

    char buffer1[2100];  // todo: hardcoding

    std::fill(&buffer1[0], &buffer1[(nBufSize - 1)], 0);  // Initializing to 0.

    bool bContentMode = false;  // "Currently IN content mode."
    bool bHaveEnteredContentMode =
        false;  // "Have NOT YET entered content mode."

    // Clear out whatever string might have been in there before.
    Release();

    // Load up the string from theStr,
    // (bookended by "-----BEGIN ... -----" and "END-----" messages)
    bool bIsEOF = false;
    theStr.reset();  // So we can call theStr.sgets(). Making sure position is
                     // at
                     // start of string.

    do {
        bIsEOF = !(theStr.sgets(buffer1, nBufSize2));  // 2048

        std::string line = buffer1;
        const char* pBuf = line.c_str();

        // It's not a blank line.
        if (line.length() < 2) {
            continue;
        }

        // if we're on a dashed line...
        else if (
            line.at(0) == '-' && line.at(2) == '-' && line.at(3) == '-' &&
            (bEscaped ? (line.at(1) == ' ') : (line.at(1) == '-'))) {
            // If I just hit a dash, that means there are only two options:

            // a. I have not yet entered content mode, and potentially just now
            // entering it for the first time.
            if (!bHaveEnteredContentMode) {
                // str_override defaults to:  "-----BEGIN" (If you want to load
                // a private key instead,
                // Try passing "-----BEGIN ENCRYPTED PRIVATE" instead of going
                // with the default.)
                //
                if (line.find(str_override) != std::string::npos &&
                    line.at(0) == '-' && line.at(2) == '-' &&
                    line.at(3) == '-' &&
                    (bEscaped ? (line.at(1) == ' ') : (line.at(1) == '-'))) {
                    //                    otErr << "Reading ascii-armored
                    // contents...";
                    bHaveEnteredContentMode = true;
                    bContentMode = true;
                    continue;
                } else {
                    continue;
                }
            }

            // b. I am now LEAVING content mode!
            else if (
                bContentMode &&
                // str_end_line is "-----END"
                (line.find(str_end_line) != std::string::npos)) {
                //                otErr << "Finished reading ascii-armored
                // contents.\n";
                //                otErr << "Finished reading ascii-armored
                // contents:\n%s(END DATA)\n", Get());
                bContentMode = false;
                continue;
            }
        }

        // Else we're on a normal line, not a dashed line.
        else {
            if (bHaveEnteredContentMode && bContentMode) {
                if (line.compare(0, 8, "Version:") == 0) {
                    //                    otErr << "Skipping version line...\n";
                    continue;
                }
                if (line.compare(0, 8, "Comment:") == 0) {
                    //                    otErr << "Skipping comment line...\n";
                    continue;
                }
            }
        }

        // Here we save the line to member variables, if appropriate
        if (bContentMode) {
            Concatenate("%s\n", pBuf);
        }
    } while (!bIsEOF && (bContentMode || !bHaveEnteredContentMode));

    // reset the string position back to 0
    theStr.reset();

    if (!bHaveEnteredContentMode) {
        otErr << "Error in OTASCIIArmor::LoadFromString: EOF before "
                 "ascii-armored "
                 "content found, in:\n\n"
              << theStr << "\n\n";
        return false;
    } else if (bContentMode) {
        otErr
            << "Error in OTASCIIArmor::LoadFromString: EOF while still reading "
               "content, in:\n\n"
            << theStr << "\n\n";
        return false;
    } else
        return true;
}

OTASCIIArmor::~OTASCIIArmor() {}
}  // namespace opentxs
