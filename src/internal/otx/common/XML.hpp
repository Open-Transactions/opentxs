// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/crypto/HashType.hpp"

#pragma once

#include <irrxml/irrXML.hpp>

#include "internal/otx/common/crypto/Signature.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/util/Container.hpp"

namespace irr
{
namespace io
{
class IFileReadCallBack;
class IXMLBase;
template <class char_type, class super_class>
class IIrrXMLReader;

using IrrXMLReader = IIrrXMLReader<char, IXMLBase>;
}  // namespace io
}  // namespace irr

namespace opentxs
{
class Armored;
class String;
}  // namespace opentxs

namespace opentxs
{
using listOfSignatures = UnallocatedList<OTSignature>;

auto AddBookendsAroundContent(
    String& strOutput,
    const String& strContents,
    const String& strContractType,
    const crypto::HashType hashType,
    const listOfSignatures& listSignatures) -> bool;
auto DearmorAndTrim(
    const String& strInput,
    String& strOutput,
    String& strFirstLine) -> bool;
auto LoadEncodedTextField(irr::io::IrrXMLReader*& xml, Armored& ascOutput)
    -> bool;
auto LoadEncodedTextField(irr::io::IrrXMLReader*& xml, String& strOutput)
    -> bool;
auto LoadEncodedTextFieldByName(
    irr::io::IrrXMLReader*& xml,
    Armored& ascOutput,
    const char* szName,
    String::Map* pmapExtraVars = nullptr) -> bool;
auto LoadEncodedTextFieldByName(
    irr::io::IrrXMLReader*& xml,
    String& strOutput,
    const char* szName,
    String::Map* pmapExtraVars = nullptr) -> bool;
auto SkipAfterLoadingField(irr::io::IrrXMLReader*& xml) -> bool;
auto SkipToElement(irr::io::IrrXMLReader*& xml) -> bool;
auto SkipToTextField(irr::io::IrrXMLReader*& xml) -> bool;
auto trim(const String& str) -> OTString;
}  // namespace opentxs
