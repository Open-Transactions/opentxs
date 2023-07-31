// Copyright (c) 2010-2023 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::api::session::Notary

#pragma once

#include <opentxs/opentxs.hpp>

#include "ottest/fixtures/integration/Helpers.hpp"

namespace ottest
{
namespace ot = opentxs;

class OPENTXS_EXPORT Integration : public IntegrationFixture
{
public:
    static const bool have_hd_;
    static Issuer issuer_data_;
    static int msg_count_;
    static ot::UnallocatedMap<int, ot::UnallocatedCString> message_;
    static ot::identifier::UnitDefinition unit_id_;

    const ot::api::session::Client& api_alex_;
    const ot::api::session::Client& api_bob_;
    const ot::api::session::Client& api_issuer_;
    const ot::api::session::Notary& api_server_1_;

    auto idle() const noexcept -> void;

    Integration();
};

}  // namespace ottest
