// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "core/Armored.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <array>
#include <cstdint>
#include <fstream>  // IWYU pragma: keep
#include <limits>
#include <sstream>  // IWYU pragma: keep
#include <stdexcept>
#include <string_view>

#include "2_Factory.hpp"
#include "BoostIostreams.hpp"
#include "core/String.hpp"
#include "internal/core/String.hpp"
#include "internal/crypto/Envelope.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "internal/util/Pimpl.hpp"
#include "internal/util/Size.hpp"
#include "opentxs/OT.hpp"
#include "opentxs/api/crypto/Crypto.hpp"
#include "opentxs/api/crypto/Encode.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Writer.hpp"

template class opentxs::Pimpl<opentxs::Armored>;

namespace opentxs
{
using namespace std::literals;

const char* OT_BEGIN_ARMORED = "-----BEGIN OT ARMORED";
const char* OT_END_ARMORED = "-----END OT ARMORED";

const char* OT_BEGIN_ARMORED_escaped = "- -----BEGIN OT ARMORED";
const char* OT_END_ARMORED_escaped = "- -----END OT ARMORED";

const char* OT_BEGIN_SIGNED = "-----BEGIN SIGNED";
const char* OT_BEGIN_SIGNED_escaped = "- -----BEGIN SIGNED";

auto Armored::Factory(const api::Crypto& crypto) -> OTArmored
{
    return OTArmored(new implementation::Armored(crypto));
}

auto Armored::Factory(const api::Crypto& crypto, const opentxs::String& value)
    -> OTArmored
{
    return OTArmored(new implementation::Armored(crypto, value));
}

auto Armored::LoadFromString(
    Armored& ascArmor,
    const String& strInput,
    UnallocatedCString str_bookend) -> bool
{

    if (strInput.Contains(String::Factory(str_bookend)))  // YES there are
                                                          // bookends around
                                                          // this.
    {
        const UnallocatedCString str_escaped("- " + str_bookend);

        const bool bEscaped = strInput.Contains(String::Factory(str_escaped));

        auto strLoadFrom = String::Factory(strInput.Get());

        if (!ascArmor.LoadFromString(strLoadFrom, bEscaped))  // removes the
                                                              // bookends so we
                                                              // have JUST the
                                                              // coded part.
        {

            return false;
        }
    } else {
        ascArmor.Set(strInput.Get());
    }

    return true;
}

auto Factory::Armored(const api::Crypto& crypto) -> opentxs::Armored*
{
    return new implementation::Armored(crypto);
}

auto Factory::Armored(const api::Crypto& crypto, const opentxs::Data& input)
    -> opentxs::Armored*
{
    return new implementation::Armored(crypto, input);
}

auto Factory::Armored(const api::Crypto& crypto, const String& input)
    -> opentxs::Armored*
{
    return new implementation::Armored(crypto, input);
}

auto Factory::Armored(const api::Crypto& crypto, const crypto::Envelope& input)
    -> opentxs::Armored*
{
    return new implementation::Armored(crypto, input);
}
}  // namespace opentxs

namespace opentxs::implementation
{
// initializes blank.
Armored::Armored(const api::Crypto& crypto)
    : String()
    , crypto_(crypto)
{
}

// encodes
Armored::Armored(const api::Crypto& crypto, const opentxs::String& strValue)
    : Armored(crypto)
{
    SetString(strValue);
}

// encodes
Armored::Armored(const api::Crypto& crypto, const opentxs::Data& theValue)
    : Armored(crypto)
{
    SetData(theValue);
}

// assumes envelope contains encrypted data; grabs that data in base64-form onto
// *this.
Armored::Armored(const api::Crypto& crypto, const crypto::Envelope& theEnvelope)
    : Armored(crypto)
{
    theEnvelope.Armored(*this);
}

// Copies (already encoded)
Armored::Armored(const Armored& strValue)
    : Armored(strValue.crypto_)
{
    Set(strValue.Get());
}

// copies, assumes already encoded.
auto Armored::operator=(const char* szValue) -> Armored&
{
    Set(szValue);
    return *this;
}

// encodes
auto Armored::operator=(const opentxs::String& strValue) -> Armored&
{
    if ((&strValue) != (&(dynamic_cast<const opentxs::String&>(*this)))) {
        SetString(strValue);
    }
    return *this;
}

// encodes
auto Armored::operator=(const opentxs::Data& theValue) -> Armored&
{
    SetData(theValue);
    return *this;
}

// assumes is already encoded and just copies the encoded text
auto Armored::operator=(const Armored& strValue) -> Armored&
{
    if ((&strValue) != this)  // prevent self-assignment
    {
        String::operator=(dynamic_cast<const String&>(strValue));
    }
    return *this;
}

auto Armored::clone() const -> Armored* { return new Armored(*this); }

auto Armored::compress_string(std::string_view data) const noexcept(false)
    -> UnallocatedCString
{
    using namespace boost::iostreams;
    auto compressed = std::stringstream{};
    auto decompressed = std::stringstream{};
    decompressed << data;
    auto buf = filtering_istreambuf{};
    buf.push(zlib_compressor());
    buf.push(decompressed);
    copy(buf, compressed);

    return compressed.str();
}

auto Armored::decompress_string(std::string_view data) const noexcept(false)
    -> UnallocatedCString
{
    using namespace boost::iostreams;
    auto compressed = std::stringstream{};
    auto decompressed = std::stringstream{};
    compressed << data;
    auto buf = filtering_istreambuf{};
    buf.push(zlib_decompressor());
    buf.push(compressed);
    copy(buf, decompressed);

    return decompressed.str();
}

// Base64-decode
auto Armored::GetData(opentxs::Data& theData, bool bLineBreaks) const -> bool
{
    theData.clear();

    if (GetLength() < 1) { return true; }

    auto decoded = UnallocatedCString{};
    const auto rc = crypto_.Encode().Base64Decode(Bytes(), writer(decoded));

    if (false == rc) {
        LogError()(OT_PRETTY_CLASS())("Base64Decode failed.").Flush();

        return false;
    }

    theData.Assign(decoded.c_str(), decoded.size());

    return (0 < decoded.size());
}

// Base64-decode and decompress
auto Armored::GetString(opentxs::String& strData, bool bLineBreaks) const
    -> bool
{
    try {
        strData.Release();

        if (empty()) { return true; }

        const auto decoded = [&] {
            auto out = UnallocatedCString{};
            const auto rc = crypto_.Encode().Base64Decode(Bytes(), writer(out));

            if (false == rc) {

                throw std::runtime_error{"Base64Decode failed"};
            }

            return out;
        }();
        const auto decompressed = [&] {
            try {

                return decompress_string(decoded);
            } catch (const std::exception& e) {

                throw std::runtime_error{"decompress failed: "s + e.what()};
            }
        }();

        strData.Set(decompressed.c_str(), shorten(decompressed.size()));

        return true;
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return false;
    }
}

// This code reads up the file, discards the bookends, and saves only the
// gibberish itself.
auto Armored::LoadFrom_ifstream(std::ifstream& fin) -> bool
{
    std::stringstream buffer;
    buffer << fin.rdbuf();

    UnallocatedCString contents(buffer.str());

    auto theString = String::Factory();
    theString->Set(contents.c_str());

    return LoadFromString(theString);
}

auto Armored::LoadFromExactPath(const UnallocatedCString& filename) -> bool
{
    std::ifstream fin(filename.c_str(), std::ios::binary);

    if (!fin.is_open()) {
        LogDetail()(OT_PRETTY_CLASS())("Failed opening file: ")(filename)
            .Flush();
        return false;
    }

    return LoadFrom_ifstream(fin);
}

auto Armored::LoadFromString(
    opentxs::String& theStr,  // input
    bool bEscaped,
    const UnallocatedCString str_override) -> bool
{
    // Should never be 0 size, as default is "-----BEGIN"
    // But if you want to load a private key, try "-----BEGIN ENCRYPTED PRIVATE"
    // instead.
    // *smile*
    const UnallocatedCString str_end_line =
        "-----END";  // Someday maybe allow parameterized option for this.

    const std::int32_t nBufSize = 2100;   // todo: hardcoding
    const std::int32_t nBufSize2 = 2048;  // todo: hardcoding

    auto buffer1 = std::array<char, 2100>{};  // todo: hardcoding

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
        bIsEOF = !(theStr.sgets(buffer1.data(), nBufSize2));  // 2048

        UnallocatedCString line = buffer1.data();

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
                if (line.find(str_override) != UnallocatedCString::npos &&
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
                (line.find(str_end_line) != UnallocatedCString::npos)) {
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
            line.append("\n");
            Concatenate(String::Factory(line));
        }
    } while (!bIsEOF && (bContentMode || !bHaveEnteredContentMode));

    // reset the string position back to 0
    theStr.reset();

    if (!bHaveEnteredContentMode) {
        LogError()(OT_PRETTY_CLASS())(
            "Error in Armored::LoadFromString: EOF before "
            "ascii-armored "
            "content found, in: ")(theStr)(".")
            .Flush();
        return false;
    } else if (bContentMode) {
        LogError()(OT_PRETTY_CLASS())(
            "Error in Armored::LoadFromString: EOF while still reading "
            "content, in: ")(theStr)(".")
            .Flush();
        return false;
    } else {
        return true;
    }
}

// Base64-encode
auto Armored::SetData(const opentxs::Data& theData, bool) -> bool
{
    Release();

    if (theData.size() < 1) { return true; }

    auto string = UnallocatedCString{};
    const auto rc =
        crypto_.Encode().Base64Encode(theData.Bytes(), writer(string));

    if (false == rc) {
        LogError()(OT_PRETTY_CLASS())("Base64Encode failed.").Flush();

        return false;
    }

    OT_ASSERT(std::numeric_limits<std::uint32_t>::max() >= string.size());

    Set(string.data(), static_cast<std::uint32_t>(string.size()));

    return true;
}

auto Armored::SaveTo_ofstream(std::ofstream& fout) -> bool
{
    auto strOutput = String::Factory();
    UnallocatedCString str_type("DATA");  // -----BEGIN OT ARMORED DATA-----

    if (WriteArmoredString(strOutput, str_type) && strOutput->Exists()) {
        // WRITE IT TO THE FILE
        //
        fout << strOutput;

        if (fout.fail()) {
            LogError()(OT_PRETTY_CLASS())("Failed saving to file. Contents: ")(
                strOutput.get())(".")
                .Flush();
            return false;
        }

        return true;
    }

    return false;
}

auto Armored::SaveToExactPath(const UnallocatedCString& filename) -> bool
{
    std::ofstream fout(filename.c_str(), std::ios::out | std::ios::binary);

    if (!fout.is_open()) {
        LogDetail()(OT_PRETTY_CLASS())("Failed opening file: ")(filename)
            .Flush();
        return false;
    }

    return SaveTo_ofstream(fout);
}

// Compress and Base64-encode
auto Armored::SetString(
    const opentxs::String& strData,
    bool bLineBreaks) -> bool  //=true
{
    try {
        Release();

        if (strData.empty()) { return true; }

        const auto compressed = [&] {
            try {

                return compress_string(strData.Bytes());
            } catch (const std::exception& e) {

                throw std::runtime_error{"compression failed: "s + e.what()};
            }
        }();

        if (compressed.size() == 0_uz) {

            throw std::runtime_error{"empty compressed output"s};
        }

        const auto encoded = [&] {
            auto out = UnallocatedCString{};
            const auto rc =
                crypto_.Encode().Base64Encode(compressed, writer(out));

            if (false == rc) {

                throw std::runtime_error{"base64 encode failed"s};
            }

            return out;
        }();

        Set(encoded.data(), shorten(encoded.size()));

        return true;
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return false;
    }
}

auto Armored::WriteArmoredString(
    opentxs::String& strOutput,
    const UnallocatedCString type,
    bool bEscaped) const -> bool
{
    const auto escape = [&]() -> std::string_view {
        if (bEscaped) {

            return "- "sv;
        } else {

            return {};
        }
    }();
    strOutput.Release();
    strOutput.Concatenate(escape)
        .Concatenate(OT_BEGIN_ARMORED)
        .Concatenate(" "sv)
        .Concatenate(type)
        .Concatenate("-----\nVersion: Open Transactions "sv)
        .Concatenate(VersionString())
        .Concatenate("\nComment: http://opentransactions.org\n\n"sv)
        .Concatenate(Get())
        .Concatenate("\n"sv)
        .Concatenate(escape)
        .Concatenate(OT_END_ARMORED)
        .Concatenate(" "sv)
        .Concatenate(type)
        .Concatenate("-----\n\n"sv);

    return true;
}
}  // namespace opentxs::implementation
