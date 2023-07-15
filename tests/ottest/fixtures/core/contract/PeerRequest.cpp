// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "ottest/fixtures/core/contract/PeerRequest.hpp"  // IWYU pragma: associated

#include <utility>

namespace ottest
{
std::optional<PeerRequestListener> PeerRequest::listener_{};
}  // namespace ottest

namespace ottest
{
PeerRequest::PeerRequest() noexcept
    : notary_(StartNotarySession(0))
{
}

auto PeerRequest::InitListener(PeerRequestListener::Callback cb) noexcept
    -> PeerRequestListener::Future
{
    listener_.emplace(client_1_, client_2_, std::move(cb));

    return listener_->get();
}

PeerRequest::~PeerRequest() = default;
}  // namespace ottest
