// Copyright (c) 2010-2023 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <atomic>
#include <future>
#include <utility>

#include "internal/api/session/UI.hpp"
#include "internal/interface/ui/ContactList.hpp"
#include "ottest/env/OTTestEnvironment.hpp"
#include "ottest/fixtures/common/Counter.hpp"
#include "ottest/fixtures/common/User.hpp"
#include "ottest/fixtures/integration/Helpers.hpp"
#include "ottest/fixtures/ui/ContactList.hpp"

namespace ottest
{
namespace ot = opentxs;

struct OPENTXS_EXPORT AddContact : public IntegrationFixture {
    static const bool have_hd_;

    const ot::api::session::Client& api_alex_;
    const ot::api::session::Client& api_bob_;
    const ot::api::session::Client& api_chris_;
    const ot::api::session::Notary& api_server_1_;

    AddContact();
};

}  // namespace ottest

