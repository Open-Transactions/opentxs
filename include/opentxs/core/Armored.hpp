// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>
#include <iosfwd>

#include "opentxs/Export.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Pimpl.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
class Armored;
class Data;

using OTArmored = Pimpl<Armored>;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs
{
extern const char* OT_BEGIN_ARMORED;
extern const char* OT_END_ARMORED;

extern const char* OT_BEGIN_ARMORED_escaped;
extern const char* OT_END_ARMORED_escaped;

extern const char* OT_BEGIN_SIGNED;
extern const char* OT_BEGIN_SIGNED_escaped;

/** The natural state of Armored is in compressed and base64-encoded,
 string form.

 HOW TO USE THIS CLASS

 Methods that put data into Armored
   ...if the input is already encoded:
      Constructors that take Armored, OTEnvelope, char*
      Assignment operators that take Armored, char*
      Load methods

   ...if the data is *not* already encoded:
      Constructors that take String, Data
      Assignment operators that take String, Data
      Set methods

 Methods that take data out of Armored
   ...in encoded form:
      Write methods
      Save methods
      (inherited) String::Get() method

   ...in decoded form:
      Armored::GetString() and Armored::GetData() methods

      Note: if an Armored is provided to the constructor of String(),
      the resulting String will be in *decoded* form. */
class OPENTXS_EXPORT Armored : virtual public String
{
public:
    static auto Factory() -> opentxs::Pimpl<opentxs::Armored>;
    static auto Factory(const String& in) -> opentxs::Pimpl<opentxs::Armored>;

    /** Let's say you don't know if the input string is raw base64, or if it has
     * bookends on it like -----BEGIN BLAH BLAH ... And if it DOES have
     * Bookends, you don't know if they are escaped: - -----BEGIN ... Let's say
     * you just want an easy function that will figure that crap out, and load
     * the contents up properly into an Armored object. (That's what this
     * function will do.)
     *
     * str_bookend is a default. You could make it more specific like,
     * -----BEGIN ENCRYPTED KEY (or whatever.)
     */
    static auto LoadFromString(
        Armored& ascArmor,
        const String& strInput,
        UnallocatedCString str_bookend = "-----BEGIN") -> bool;

    virtual auto GetData(Data& theData, bool bLineBreaks = true) const
        -> bool = 0;
    virtual auto GetString(String& theData, bool bLineBreaks = true) const
        -> bool = 0;
    // for "-----BEGIN OT LEDGER-----", str_type would contain "LEDGER" There's
    // no default, to force you to enter the right string.
    // for "-----BEGIN OT LEDGER-----", str_type would contain "LEDGER" There's
    // no default, to force you to enter the right string.
    virtual auto WriteArmoredString(
        String& strOutput,
        const UnallocatedCString str_type,
        bool bEscaped = false) const -> bool = 0;

    virtual auto LoadFrom_ifstream(std::ifstream& fin) -> bool = 0;
    virtual auto LoadFromExactPath(const UnallocatedCString& filename)
        -> bool = 0;
    // This code reads up the string, discards the bookends, and saves only the
    // gibberish itself. the bEscaped option allows you to load a normal
    // ASCII-Armored file if off, and allows you to load an escaped
    // ASCII-armored  file (such as inside the contracts when the public keys
    // are escaped with a  "- " before the rest of the ------- starts.)
    //
    // str_override determines where the content starts, when loading.
    // "-----BEGIN" is the default "content start" substr.
    virtual auto LoadFromString(
        String& theStr,
        bool bEscaped = false,
        const UnallocatedCString str_override = "-----BEGIN") -> bool = 0;
    virtual auto SaveTo_ofstream(std::ofstream& fout) -> bool = 0;
    virtual auto SaveToExactPath(const UnallocatedCString& filename)
        -> bool = 0;
    virtual auto SetData(const Data& theData, bool bLineBreaks = true)
        -> bool = 0;
    virtual auto SetString(const String& theData, bool bLineBreaks = true)
        -> bool = 0;

    Armored(const Armored&) = delete;
    Armored(Armored&&) = delete;
    auto operator=(const Armored&) -> Armored& = delete;
    auto operator=(Armored&&) -> Armored& = delete;

    ~Armored() override = default;

protected:
    friend OTArmored;

#ifndef _WIN32
    auto clone() const -> Armored* override = 0;
#endif

    Armored() = default;
};
}  // namespace opentxs
