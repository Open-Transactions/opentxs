// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>
#include <iosfwd>
#include <memory>

#include "String.hpp"
#include "opentxs/core/Armored.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace OTDB
{
class OTPacker;
}  // namespace OTDB

namespace crypto
{
class Envelope;
}  // namespace crypto

class Data;
class Factory;
class String;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::implementation
{
class Armored : virtual public opentxs::Armored, public String
{
public:
    auto GetData(opentxs::Data& theData, bool bLineBreaks = true) const
        -> bool override;
    auto GetString(opentxs::String& theData, bool bLineBreaks = true) const
        -> bool override;
    auto WriteArmoredString(
        opentxs::String& strOutput,
        const UnallocatedCString str_type,
        bool bEscaped = false) const -> bool override;
    auto LoadFrom_ifstream(std::ifstream& fin) -> bool override;
    auto LoadFromExactPath(const UnallocatedCString& filename) -> bool override;
    auto LoadFromString(
        opentxs::String& theStr,
        bool bEscaped = false,
        const UnallocatedCString str_override = "-----BEGIN") -> bool override;
    auto SaveTo_ofstream(std::ofstream& fout) -> bool override;
    auto SaveToExactPath(const UnallocatedCString& filename) -> bool override;
    auto SetData(const opentxs::Data& theData, bool bLineBreaks = true)
        -> bool override;
    auto SetString(const opentxs::String& theData, bool bLineBreaks = true)
        -> bool override;

    ~Armored() override = default;

protected:
    Armored();

private:
    friend OTArmored;
    friend opentxs::Armored;
    friend opentxs::Factory;

    static std::unique_ptr<OTDB::OTPacker> s_pPacker;

    auto clone() const -> Armored* override;
    auto compress_string(
        const UnallocatedCString& str,
        std::int32_t compressionlevel) const -> UnallocatedCString;
    auto decompress_string(const UnallocatedCString& str) const
        -> UnallocatedCString;

    explicit Armored(const opentxs::Data& theValue);
    explicit Armored(const opentxs::String& strValue);
    explicit Armored(const crypto::Envelope& theEnvelope);
    Armored(const Armored& strValue);

    auto operator=(const char* szValue) -> Armored&;
    auto operator=(const opentxs::Data& theValue) -> Armored&;
    auto operator=(const opentxs::String& strValue) -> Armored&;
    auto operator=(const Armored& strValue) -> Armored&;
};
}  // namespace opentxs::implementation
