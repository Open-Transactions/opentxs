// Copyright (c) 2010-2023 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "ottest/fixtures/client/NymData.hpp"  // IWYU pragma: associated

#include <opentxs/opentxs.hpp>
#include <cstdint>
#include <memory>

namespace ottest
{
NymData::NymData()
    : nym_id_(client_1_.Factory().NymIDFromRandom())
    , reason_(client_1_.Factory().PasswordPrompt(__func__))
    , nym_data_(client_1_.Wallet().mutable_Nym(
          client_1_.Wallet().Nym(reason_, "testNym")->ID(),
          reason_))
{
}

auto NymData::ExpectedStringOutput(const std::uint32_t version)
    -> ot::UnallocatedCString
{
    return ot::UnallocatedCString{"Version "} + std::to_string(version) +
           ot::UnallocatedCString(
               " contact data\nSections found: 1\n- Section: "
               "Scope, version: ") +
           std::to_string(version) +
           ot::UnallocatedCString{" containing 1 item(s).\n-- Item type: "
                                  "\"Individual\", value: "
                                  "\"testNym\", start: 0, end: 0, version: "} +
           std::to_string(version) +
           ot::UnallocatedCString{"\n--- Attributes: Active Primary \n"};
}
}  // namespace ottest
