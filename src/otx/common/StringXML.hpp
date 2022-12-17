// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include <irrxml/irrXML.hpp>

#pragma once

#include <cstdint>

#include "core/String.hpp"
#include "internal/otx/common/StringXML.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace irr
{
namespace io
{
class IFileReadCallBack;  // IWYU pragma: keep
}  // namespace io
}  // namespace irr

namespace opentxs
{
class String;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::implementation
{
class StringXML final : virtual public opentxs::StringXML, public String
{
public:
    operator irr::io::IFileReadCallBack*() final;

    auto read(void* buffer, std::uint32_t sizeToRead) -> std::int32_t final;
    auto getSize() -> std::int32_t final;

    auto operator=(const opentxs::String& rhs) -> StringXML& final;
    auto operator=(const opentxs::StringXML& rhs) -> StringXML& final;

    StringXML(StringXML&&) = delete;
    auto operator=(const StringXML&) -> StringXML& = delete;
    auto operator=(StringXML&&) -> StringXML& = delete;

    ~StringXML() final;

private:
    friend opentxs::StringXML;

    class StringXMLPvt;

    StringXMLPvt* const pvt_;

    StringXML(const opentxs::String& value);
    StringXML();
    StringXML(const StringXML& value);
};
}  // namespace opentxs::implementation
