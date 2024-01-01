// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/protobuf/contact/Types.internal.hpp"  // IWYU pragma: associated

#include "util/Sodium.hpp"

auto std::hash<opentxs::protobuf::contact::ContactSectionVersion>::operator()(
    const opentxs::protobuf::contact::ContactSectionVersion& data)
    const noexcept -> std::size_t
{
    const auto& [key, value] = data;

    return opentxs::crypto::sodium::Siphash(
        opentxs::crypto::sodium::MakeSiphashKey(
            {reinterpret_cast<const char*>(&key), sizeof(key)}),
        {reinterpret_cast<const char*>(&value), sizeof(value)});
}

auto std::hash<opentxs::protobuf::contact::EnumLang>::operator()(
    const opentxs::protobuf::contact::EnumLang& data) const noexcept
    -> std::size_t
{
    const auto& [key, text] = data;

    return opentxs::crypto::sodium::Siphash(
        opentxs::crypto::sodium::MakeSiphashKey(
            {reinterpret_cast<const char*>(&key), sizeof(key)}),
        text);
}
