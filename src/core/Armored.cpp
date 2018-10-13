// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/core/Armored.hpp"

#include "opentxs/api/crypto/Crypto.hpp"
#include "opentxs/api/crypto/Encode.hpp"
#include "opentxs/api/Native.hpp"
#include "opentxs/core/crypto/OTEnvelope.hpp"
#include "opentxs/core/util/Assert.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/OTStorage.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/OT.hpp"

#include <sys/types.h>
#include <zconf.h>
#include <zlib.h>

#include <cstdint>
#include <cstring>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>

#include "Armored.hpp"

template class opentxs::Pimpl<opentxs::Armored>;

namespace opentxs
{
const char* OT_BEGIN_ARMORED = "-----BEGIN OT ARMORED";
const char* OT_END_ARMORED = "-----END OT ARMORED";

const char* OT_BEGIN_ARMORED_escaped = "- -----BEGIN OT ARMORED";
const char* OT_END_ARMORED_escaped = "- -----END OT ARMORED";

const char* OT_BEGIN_SIGNED = "-----BEGIN SIGNED";
const char* OT_BEGIN_SIGNED_escaped = "- -----BEGIN SIGNED";

OTArmored Armored::Factory()
{
    return OTArmored(new implementation::Armored());
}

OTArmored Armored::Factory(const opentxs::String& value)
{
    return OTArmored(new implementation::Armored(value));
}

OTArmored Armored::Factory(const Data& value)
{
    return OTArmored(new implementation::Armored(value));
}

OTArmored Armored::Factory(const OTEnvelope& value)
{
    return OTArmored(new implementation::Armored(value));
}

OTArmored Armored::Factory(const char* value)
{
    return OTArmored(new implementation::Armored(value));
}

bool Armored::LoadFromString(
    Armored& ascArmor,
    const String& strInput,
    std::string str_bookend)
{

    if (strInput.Contains(String::Factory(str_bookend)))  // YES there are
                                                          // bookends around
                                                          // this.
    {
        const std::string str_escaped("- " + str_bookend);

        const bool bEscaped = strInput.Contains(String::Factory(str_escaped));

        auto strLoadFrom = String::Factory(strInput.Get());

        if (!ascArmor.LoadFromString(strLoadFrom, bEscaped))  // removes the
                                                              // bookends so we
                                                              // have JUST the
                                                              // coded part.
        {

            return false;
        }
    } else
        ascArmor.Set(strInput.Get());

    return true;
}
}  // namespace opentxs

namespace opentxs::implementation
{
// initializes blank.
Armored::Armored()
    : String()
{
}

// encodes
Armored::Armored(const opentxs::String& strValue)
    : Armored()
{
    SetString(strValue);
}

// encodes
Armored::Armored(const Data& theValue)
    : Armored()
{
    SetData(theValue);
}

// assumes envelope contains encrypted data; grabs that data in base64-form onto
// *this.
Armored::Armored(const OTEnvelope& theEnvelope)
    : Armored()
{
    theEnvelope.GetCiphertext(*this);
}

// copies (already encoded)
Armored::Armored(const char* szValue)
    : Armored()
{
    Set(szValue);
}

// Copies (already encoded)
Armored::Armored(const Armored& strValue)
    : opentxs::String()
    , opentxs::Armored()
    , String(dynamic_cast<const String&>(strValue))
{
    Set(strValue.Get());
}

// copies, assumes already encoded.
Armored& Armored::operator=(const char* szValue)
{
    Set(szValue);
    return *this;
}

// encodes
Armored& Armored::operator=(const opentxs::String& strValue)
{
    if ((&strValue) != (&(dynamic_cast<const opentxs::String&>(*this)))) {
        SetString(strValue);
    }
    return *this;
}

// encodes
Armored& Armored::operator=(const Data& theValue)
{
    SetData(theValue);
    return *this;
}

// assumes is already encoded and just copies the encoded text
Armored& Armored::operator=(const Armored& strValue)
{
    if ((&strValue) != this)  // prevent self-assignment
    {
        String::operator=(dynamic_cast<const String&>(strValue));
    }
    return *this;
}

Armored* Armored::clone() const { return new Armored(*this); }

// Source for these two functions: http://panthema.net/2007/0328-ZLibString.html
/** Compress a STL string using zlib with given compression level and return
 * the binary data. */
std::string Armored::compress_string(
    const std::string& str,
    std::int32_t compressionlevel = Z_BEST_COMPRESSION) const
{
    z_stream zs;  // z_stream is zlib's control structure
    memset(&zs, 0, sizeof(zs));

    if (deflateInit(&zs, compressionlevel) != Z_OK)
        throw(std::runtime_error("deflateInit failed while compressing."));

    zs.next_in = reinterpret_cast<Bytef*>(const_cast<char*>(str.data()));
    zs.avail_in = static_cast<uInt>(str.size());  // set the z_stream's input

    std::int32_t ret;
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
std::string Armored::decompress_string(const std::string& str) const
{
    z_stream zs;  // z_stream is zlib's control structure
    memset(&zs, 0, sizeof(zs));

    if (inflateInit(&zs) != Z_OK)
        throw(std::runtime_error("inflateInit failed while decompressing."));

    zs.next_in = reinterpret_cast<Bytef*>(const_cast<char*>(str.data()));
    zs.avail_in = static_cast<uInt>(str.size());

    std::int32_t ret;
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
        if (zs.msg != nullptr) { oss << " " << zs.msg; }
        throw(std::runtime_error(oss.str()));
    }

    return outstring;
}

// Base64-decode
bool Armored::GetData(Data& theData,
                      bool bLineBreaks) const  // linebreaks=true
{
    theData.Release();

    if (GetLength() < 1) return true;

    auto decoded =
        OT::App().Crypto().Encode().DataDecode(std::string(Get(), GetLength()));

    theData.Assign(decoded.c_str(), decoded.size());

    return (0 < decoded.size());
}

// Base64-decode and decompress
bool Armored::GetString(opentxs::String& strData, bool bLineBreaks) const
{
    strData.Release();

    if (GetLength() < 1) { return true; }

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

// This code reads up the file, discards the bookends, and saves only the
// gibberish itself.
bool Armored::LoadFrom_ifstream(std::ifstream& fin)
{
    std::stringstream buffer;
    buffer << fin.rdbuf();

    std::string contents(buffer.str());

    auto theString = String::Factory();
    theString->Set(contents.c_str());

    return LoadFromString(theString);
}

bool Armored::LoadFromExactPath(const std::string& filename)
{
    std::ifstream fin(filename.c_str(), std::ios::binary);

    if (!fin.is_open()) {
        otWarn << "Armored::LoadFromExactPath: Failed opening file: "
               << filename << "\n";
        return false;
    }

    return LoadFrom_ifstream(fin);
}

bool Armored::LoadFromString(
    opentxs::String& theStr,  // input
    bool bEscaped,
    const std::string str_override)
{
    // Should never be 0 size, as default is "-----BEGIN"
    // But if you want to load a private key, try "-----BEGIN ENCRYPTED PRIVATE"
    // instead.
    // *smile*
    const std::string str_end_line =
        "-----END";  // Someday maybe allow parameterized option for this.

    const std::int32_t nBufSize = 2100;   // todo: hardcoding
    const std::int32_t nBufSize2 = 2048;  // todo: hardcoding

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
        if (bContentMode) { Concatenate("%s\n", pBuf); }
    } while (!bIsEOF && (bContentMode || !bHaveEnteredContentMode));

    // reset the string position back to 0
    theStr.reset();

    if (!bHaveEnteredContentMode) {
        otErr << "Error in Armored::LoadFromString: EOF before "
                 "ascii-armored "
                 "content found, in:\n\n"
              << theStr << "\n\n";
        return false;
    } else if (bContentMode) {
        otErr << "Error in Armored::LoadFromString: EOF while still reading "
                 "content, in:\n\n"
              << theStr << "\n\n";
        return false;
    } else
        return true;
}

// Base64-encode
bool Armored::SetData(const Data& theData, bool bLineBreaks)
{
    Release();

    if (theData.size() < 1) return true;

    auto string = OT::App().Crypto().Encode().DataEncode(theData);

    if (string.empty()) {
        otErr << __FUNCTION__ << "Base64Encode failed" << std::endl;

        return false;
    }

    Set(string.data(), string.size());

    return true;
}

bool Armored::SaveTo_ofstream(std::ofstream& fout)
{
    auto strOutput = String::Factory();
    std::string str_type("DATA");  // -----BEGIN OT ARMORED DATA-----

    if (WriteArmoredString(strOutput, str_type) && strOutput->Exists()) {
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

bool Armored::SaveToExactPath(const std::string& filename)
{
    std::ofstream fout(filename.c_str(), std::ios::out | std::ios::binary);

    if (!fout.is_open()) {
        otWarn << "Armored::SaveToExactPath: Failed opening file: " << filename
               << "\n";
        return false;
    }

    return SaveTo_ofstream(fout);
}

// Compress and Base64-encode
bool Armored::SetString(
    const opentxs::String& strData,
    bool bLineBreaks)  //=true
{
    Release();

    if (strData.GetLength() < 1) return true;

    std::string str_compressed = compress_string(strData.Get());

    // "Success"
    if (str_compressed.size() == 0) {
        otErr << "Armored::" << __FUNCTION__ << ": compression failed."
              << std::endl;

        return false;
    }

    auto pString = OT::App().Crypto().Encode().DataEncode(str_compressed);

    if (pString.empty()) {
        otErr << "Armored::" << __FUNCTION__ << ": Base64Encode failed."
              << std::endl;

        return false;
    }

    Set(pString.data(), pString.size());

    return true;
}

bool Armored::WriteArmoredString(
    opentxs::String& strOutput,
    const std::string str_type,  // for "-----BEGIN OT LEDGER-----", str_type
                                 // would contain "LEDGER" There's no default,
                                 // to force you to enter the right string.
    bool bEscaped) const
{
    const char* szEscape = "- ";

    auto strTemp = String::Factory();
    strTemp->Format(
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

    strOutput.Concatenate("%s", strTemp->Get());

    return true;
}
}  // namespace opentxs::implementation
