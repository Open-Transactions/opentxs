// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <cstdint>
#include <memory>

#include "internal/identity/Authority.hpp"
#include "internal/identity/Nym.hpp"

namespace ot = opentxs;

namespace ottest
{
class OPENTXS_EXPORT Authority : public ::testing::Test
{
public:
    const ot::api::session::Client& api_;
    const ot::PasswordPrompt reason_;
    ot::PasswordPrompt non_const_reason_;

    ot::Secret words_;
    const std::uint8_t version_ = 6;
    const int nym_ = 1;
    const ot::UnallocatedCString alias_ = ot::UnallocatedCString{"alias"};

    ot::crypto::Parameters parameters_;
    std::unique_ptr<ot::identity::Source> source_;
    std::unique_ptr<ot::identity::internal::Nym> internal_nym_;
    std::unique_ptr<ot::identity::internal::Authority> authority_;

    auto GetTag(
        const ot::crypto::asymmetric::Key& key,
        const ot::crypto::asymmetric::Key& dh,
        std::uint32_t& out) noexcept -> bool;

    Authority();

    void SetUp() override;
};
}  // namespace ottest
