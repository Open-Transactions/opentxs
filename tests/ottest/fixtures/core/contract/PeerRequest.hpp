// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <opentxs/opentxs.hpp>
#include <optional>

#include "ottest/fixtures/common/Notary.hpp"
#include "ottest/fixtures/common/PeerRequestListener.hpp"
#include "ottest/fixtures/common/TwoClientSessions.hpp"

namespace ot = opentxs;

namespace ottest
{
class OPENTXS_EXPORT PeerRequest : virtual public TwoClientSessions,
                                   virtual public Notary_fixture
{
protected:
    static std::optional<PeerRequestListener> listener_;
    const ot::api::session::Notary& notary_;

    auto InitListener(PeerRequestListener::Callback cb) noexcept
        -> PeerRequestListener::Future;

    PeerRequest() noexcept;

    ~PeerRequest() override;
};
}  // namespace ottest
