// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <opentxs/opentxs.hpp>
#include <atomic>
#include <string_view>

#include "ottest/fixtures/common/OneClientSession.hpp"

namespace ottest
{
class User;

using namespace std::literals;

class OPENTXS_EXPORT PeerRequest : public OneClientSession
{
protected:
    static constexpr auto address_ = "address"sv;
    static constexpr auto description_ = "description"sv;
    static constexpr auto key_ = "key"sv;
    static constexpr auto login_ = "login"sv;
    static constexpr auto password_ = "password"sv;
    static constexpr auto secret_1_ = "secret 1"sv;
    static constexpr auto secret_2_ = "secret 2"sv;
    static constexpr auto terms_ = "terms"sv;
    static constexpr auto url_ = "url"sv;

    const opentxs::PasswordPrompt reason_;
    const User& alex_;
    const User& bob_;
    const opentxs::identifier::Notary notary_;
    const opentxs::identifier::UnitDefinition unit_;
    const opentxs::Amount amount_;
    const opentxs::blockchain::block::TransactionHash txid_;
    const opentxs::blockchain::block::Transaction tx_;

    PeerRequest() noexcept;

    ~PeerRequest() override = default;

private:
    static User alex_s_;
    static User bob_s_;
    static std::atomic<bool> init_;
};
}  // namespace ottest
