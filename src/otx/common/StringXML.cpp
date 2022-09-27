// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"              // IWYU pragma: associated
#include "1_Internal.hpp"            // IWYU pragma: associated
#include "otx/common/StringXML.hpp"  // IWYU pragma: associated

#include <irrxml/irrXML.hpp>

#include "core/String.hpp"
#include "opentxs/core/String.hpp"

namespace opentxs
{
auto StringXML::Factory() -> OTStringXML
{
    return OTStringXML(new implementation::StringXML());
}

auto StringXML::Factory(const String& value) -> OTStringXML
{
    return OTStringXML(new implementation::StringXML(value));
}
}  // namespace opentxs

namespace opentxs::implementation
{
class StringXML::StringXMLPvt final : public irr::io::IFileReadCallBack
{
public:
    StringXML* super_;

    auto read(void* buffer, unsigned sizeToRead) -> int final
    {
        return super_->read(buffer, sizeToRead);
    }

    auto getSize() -> int final { return super_->getSize(); }

    StringXMLPvt(StringXML* ptr)
        : super_(ptr)
    {
    }
    StringXMLPvt(const StringXMLPvt&) = delete;
    StringXMLPvt(StringXMLPvt&&) = delete;
    auto operator=(const StringXMLPvt&) -> StringXMLPvt& = delete;
    auto operator=(StringXMLPvt&&) -> StringXMLPvt& = delete;
};

StringXML::StringXML()
    : opentxs::String()
    , opentxs::StringXML()
    , String()
    , pvt_(new StringXMLPvt(this))
{
}

StringXML::StringXML(const opentxs::String& value)
    : opentxs::String()
    , opentxs::StringXML()
    , String(value.Get())
    , pvt_(new StringXMLPvt(this))
{
}

StringXML::StringXML(const StringXML& value)
    : opentxs::String()
    , opentxs::StringXML()
    , String(value)
    , pvt_(new StringXMLPvt(this))
{
    Set(value.Get());
}

auto StringXML::operator=(const opentxs::String& rhs) -> StringXML&
{
    if ((&rhs) != (&(dynamic_cast<const opentxs::String&>(*this)))) {
        String::operator=(dynamic_cast<const String&>(rhs));
    }

    return *this;
}

auto StringXML::operator=(const opentxs::StringXML& rhs) -> StringXML&
{
    if ((&rhs) != this) { String::operator=(dynamic_cast<const String&>(rhs)); }

    return *this;
}

StringXML::operator irr::io::IFileReadCallBack*() { return pvt_; }

auto StringXML::read(void* buffer, std::uint32_t sizeToRead) -> std::int32_t
{
    if (buffer && sizeToRead && Exists()) {
        char* pBuf = static_cast<char*>(buffer);

        std::int32_t nBytesToCopy =
            (sizeToRead > GetLength() ? GetLength() : sizeToRead);
        std::int32_t i;
        for (i = 0; i < nBytesToCopy; i++) { pBuf[i] = sgetc(); }
        return i;
    } else {
        return 0;
    }
}

auto StringXML::getSize() -> std::int32_t { return GetLength(); }

StringXML::~StringXML() { delete pvt_; }
}  // namespace opentxs::implementation
