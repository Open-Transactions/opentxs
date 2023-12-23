// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "interface/rpc/RPC.hpp"  // IWYU pragma: associated

#include <utility>

#include "opentxs/api/Context.hpp"
#include "opentxs/api/Session.hpp"
#include "opentxs/api/session/Wallet.hpp"
#include "opentxs/identifier/Nym.hpp"
#include "opentxs/interface/rpc/ResponseCode.hpp"  // IWYU pragma: keep
#include "opentxs/interface/rpc/Types.hpp"
#include "opentxs/interface/rpc/request/Base.hpp"
#include "opentxs/interface/rpc/response/Base.hpp"
#include "opentxs/interface/rpc/response/ListNyms.hpp"
#include "opentxs/util/Container.hpp"

namespace opentxs::rpc::implementation
{
auto RPC::list_nyms(const request::Base& base) const noexcept
    -> std::unique_ptr<response::Base>
{
    const auto& in = base.asListNyms();
    auto ids = response::Base::Identifiers{};
    const auto reply = [&](const auto code) {
        return std::make_unique<response::ListNyms>(
            in, response::Base::Responses{{0, code}}, std::move(ids));
    };

    try {
        const auto& session = this->session(base);

        for (const auto& id : session.Wallet().LocalNyms()) {
            ids.emplace_back(id.asBase58(ot_.Crypto()));
        }

        return reply(status(ids));
    } catch (...) {

        return reply(ResponseCode::bad_session);
    }
}
}  // namespace opentxs::rpc::implementation
