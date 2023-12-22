// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>

#include "internal/core/String.hpp"
#include "internal/util/Pimpl.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace irr
{
namespace io
{
class IFileReadCallBack;
}  // namespace io
}  // namespace irr

namespace opentxs
{
class StringXML;

using OTStringXML = Pimpl<StringXML>;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs
{
class StringXML : virtual public String
{
public:
    static auto Factory() -> OTStringXML;
    static auto Factory(const String& value) -> OTStringXML;

    virtual operator irr::io::IFileReadCallBack*() = 0;

    virtual auto read(void* buffer, std::uint32_t sizeToRead)
        -> std::int32_t = 0;
    virtual auto getSize() -> std::int32_t = 0;

    virtual auto operator=(const String& rhs) -> StringXML& = 0;
    // NOLINTNEXTLINE(misc-unconventional-assign-operator)
    virtual auto operator=(const StringXML& rhs) -> StringXML& = 0;

    StringXML(StringXML&&) = delete;
    auto operator=(StringXML&&) -> StringXML& = delete;

    ~StringXML() override = default;

protected:
    StringXML() = default;
};
}  // namespace opentxs
