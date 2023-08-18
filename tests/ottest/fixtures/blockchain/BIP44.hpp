// Copyright (c) 2010-2023 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>

#include "internal/util/DeferredConstruction.hpp"  // IWYU pragma: keep
#include "internal/util/P0330.hpp"

namespace ottest
{
namespace ot = opentxs;
using namespace opentxs::literals;

class OPENTXS_EXPORT BIP44 : public ::testing::Test
{
protected:
    static constexpr auto count_ = 1000_uz;
    static constexpr auto account_id_base58_{
        "otfQgxcu7MtAFbWHnhsnqjL26gG8TBdg1BatHoZxPopW3gRL8KfvK2gL9"};

    const ot::api::session::Client& api_;
    const ot::PasswordPrompt reason_;
    const ot::identifier::Nym& nym_id_;
    const ot::blockchain::crypto::HD& account_;
    const ot::identifier::Generic account_id_;

    static bool init_;

    static ot::DeferredConstruction<ot::Nym_p> nym_;
    static ot::DeferredConstruction<ot::crypto::SeedID> seed_id_;

    BIP44();
};
}  // namespace ottest
