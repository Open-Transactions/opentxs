// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <opentxs/opentxs.hpp>

#include "ottest/fixtures/common/Client.hpp"

namespace ot = opentxs;

namespace ottest
{
class OPENTXS_EXPORT OneClientSession : virtual public Client_fixture
{
protected:
    const ot::api::session::Client& client_1_;

    OneClientSession() noexcept;

    ~OneClientSession() override = default;
};
}  // namespace ottest
