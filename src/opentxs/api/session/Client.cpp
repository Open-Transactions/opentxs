// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/api/session/Client.hpp"  // IWYU pragma: associated

#include "opentxs/api/Session.internal.hpp"
#include "opentxs/api/session/Client.internal.hpp"

namespace opentxs::api::session
{
Client::Client(api::internal::Session* imp) noexcept
    : Session(imp)
{
}

auto Client::Activity() const -> const session::Activity&
{
    return imp_->asClient().Activity();
}

auto Client::Contacts() const -> const api::session::Contacts&
{
    return imp_->asClient().Contacts();
}

auto Client::OTX() const -> const session::OTX&
{
    return imp_->asClient().OTX();
}

auto Client::UI() const -> const session::UI& { return imp_->asClient().UI(); }

auto Client::Workflow() const -> const session::Workflow&
{
    return imp_->asClient().Workflow();
}

auto Client::ZMQ() const -> const network::ZMQ&
{
    return imp_->asClient().ZMQ();
}

Client::~Client() = default;
}  // namespace opentxs::api::session
