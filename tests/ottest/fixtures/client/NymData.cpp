// Copyright (c) 2010-2023 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "ottest/fixtures/client/NymData.hpp"  // IWYU pragma: associated

#include <ankerl/unordered_dense.h>
#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <algorithm>
#include <cstdint>
#include <iterator>
#include <memory>

#include "internal/identity/wot/claim/Types.hpp"
#include "internal/serialization/protobuf/Contact.hpp"
#include "ottest/env/OTTestEnvironment.hpp"

namespace ottest
{
namespace ot = opentxs;

auto NymData::ExpectedStringOutput(const std::uint32_t version)
    -> ot::UnallocatedCString
{
    return ot::UnallocatedCString{"Version "} + std::to_string(version) +
           ot::UnallocatedCString(
               " contact data\nSections found: 1\n- Section: "
               "Scope, version: ") +
           std::to_string(version) +
           ot::UnallocatedCString{
               " containing 1 item(s).\n-- Item type: "
               "\"Individual\", value: "
               "\"testNym\", start: 0, end: 0, version: "} +
           std::to_string(version) +
           ot::UnallocatedCString{"\n--- Attributes: Active Primary \n"};
}

NymData::NymData()
    : client_(OTTestEnvironment::GetOT().StartClientSession(0))
    , reason_(client_.Factory().PasswordPrompt(__func__))
    , nym_data_(client_.Wallet().mutable_Nym(
          client_.Wallet().Nym(reason_, "testNym")->ID(),
          reason_))
{
}

}  // namespace ottest
