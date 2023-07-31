// Copyright (c) 2010-2023 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::api::session::Notary

#pragma once

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <atomic>
#include <future>
#include <memory>
#include <sstream>
#include <utility>

#include "internal/api/session/UI.hpp"
#include "internal/api/session/Wallet.hpp"
#include "internal/core/String.hpp"
#include "internal/core/contract/ServerContract.hpp"
#include "internal/core/contract/Unit.hpp"
#include "internal/interface/ui/AccountActivity.hpp"
#include "internal/interface/ui/AccountList.hpp"
#include "internal/interface/ui/AccountListItem.hpp"
#include "internal/interface/ui/AccountSummary.hpp"
#include "internal/interface/ui/ActivitySummary.hpp"
#include "internal/interface/ui/ActivitySummaryItem.hpp"
#include "internal/interface/ui/ActivityThread.hpp"
#include "internal/interface/ui/ActivityThreadItem.hpp"
#include "internal/interface/ui/BalanceItem.hpp"
#include "internal/interface/ui/Contact.hpp"
#include "internal/interface/ui/ContactList.hpp"
#include "internal/interface/ui/ContactListItem.hpp"
#include "internal/interface/ui/ContactSection.hpp"
#include "internal/interface/ui/IssuerItem.hpp"
#include "internal/interface/ui/MessagableList.hpp"
#include "internal/interface/ui/PayableList.hpp"
#include "internal/interface/ui/PayableListItem.hpp"
#include "internal/interface/ui/Profile.hpp"
#include "internal/interface/ui/ProfileSection.hpp"
#include "internal/otx/common/Account.hpp"
#include "internal/otx/common/Message.hpp"
#include "internal/util/SharedPimpl.hpp"
#include "ottest/env/OTTestEnvironment.hpp"
#include "ottest/fixtures/common/Counter.hpp"
#include "ottest/fixtures/common/User.hpp"
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
