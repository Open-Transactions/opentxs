// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated
#include "rpc/RPC.tpp"     // IWYU pragma: associated

#include <algorithm>
#include <iterator>
#include <list>
#include <set>
#include <string>
#include <vector>

#include "internal/core/Core.hpp"
#include "opentxs/api/crypto/Blockchain.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Storage.hpp"
#include "opentxs/rpc/ResponseCode.hpp"
#include "opentxs/rpc/request/Base.hpp"
#include "opentxs/rpc/request/ListAccounts.hpp"
#include "opentxs/rpc/response/Base.hpp"
#include "opentxs/rpc/response/ListAccounts.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "rpc/RPC.hpp"

namespace opentxs::rpc::implementation
{
auto RPC::list_accounts(const request::Base& base) const noexcept
    -> std::unique_ptr<response::Base>
{
    const auto& in = base.asListAccounts();
    auto ids = response::Base::Identifiers{};
    const auto reply = [&](const auto code) {
        return std::make_unique<response::ListAccounts>(
            in, response::Base::Responses{{0, code}}, std::move(ids));
    };

    try {
        const auto& session = client_session(base);
        const auto nym = session.Factory().NymID(in.FilterNym());
        const auto notary = session.Factory().ServerID(in.FilterNotary());
        const auto unitID = session.Factory().UnitID(in.FilterUnit());
        const auto haveNym = (false == in.FilterNym().empty());
        const auto haveServer = (false == in.FilterNotary().empty());
        const auto haveUnit = (false == in.FilterUnit().empty());
        const auto nymOnly = haveNym && (!haveServer) && (!haveUnit);
        const auto serverOnly = (!haveNym) && haveServer && (!haveUnit);
        const auto unitOnly = (!haveNym) && (!haveServer) && haveUnit;
        const auto nymAndServer = haveNym && haveServer && (!haveUnit);
        const auto nymAndUnit = haveNym && (!haveServer) && haveUnit;
        const auto serverAndUnit = (!haveNym) && haveServer && haveUnit;
        const auto all = haveNym && haveServer && haveUnit;
        const auto byNymOTX = [&] {
            auto out = std::pmr::set<std::string>{};
            const auto ids = session.Storage().AccountsByOwner(nym);
            std::transform(
                ids.begin(),
                ids.end(),
                std::inserter(out, out.end()),
                [](const auto& item) { return item->str(); });

            return out;
        };
        const auto byNymBlockchain = [&] {
            auto out = std::pmr::set<std::string>{};
            const auto ids = session.Crypto().Blockchain().AccountList(nym);
            std::transform(
                ids.begin(),
                ids.end(),
                std::inserter(out, out.end()),
                [](const auto& item) { return item->str(); });

            return out;
        };
        const auto byNym = [&] {
            auto out = byNymOTX();
            auto bc = byNymBlockchain();
            std::move(bc.begin(), bc.end(), std::inserter(out, out.end()));

            return out;
        };
        const auto byServerOTX = [&] {
            auto out = std::pmr::set<std::string>{};
            const auto ids = session.Storage().AccountsByServer(notary);
            std::transform(
                ids.begin(),
                ids.end(),
                std::inserter(out, out.end()),
                [](const auto& item) { return item->str(); });

            return out;
        };
        const auto byServerBlockchain = [&] {
            auto out = std::pmr::set<std::string>{};
            const auto chain = blockchain::Chain(session, notary);
            const auto ids = session.Crypto().Blockchain().AccountList(chain);
            std::transform(
                ids.begin(),
                ids.end(),
                std::inserter(out, out.end()),
                [](const auto& item) { return item->str(); });

            return out;
        };
        const auto byServer = [&] {
            auto out = byServerOTX();
            auto bc = byServerBlockchain();
            std::move(bc.begin(), bc.end(), std::inserter(out, out.end()));

            return out;
        };
        const auto byUnitOTX = [&] {
            auto out = std::pmr::set<std::string>{};
            const auto ids = session.Storage().AccountsByContract(unitID);
            std::transform(
                ids.begin(),
                ids.end(),
                std::inserter(out, out.end()),
                [](const auto& item) { return item->str(); });

            return out;
        };
        const auto byUnitBlockchain = [&] {
            auto out = std::pmr::set<std::string>{};
            const auto chain = blockchain::Chain(session, unitID);
            const auto ids = session.Crypto().Blockchain().AccountList(chain);
            std::transform(
                ids.begin(),
                ids.end(),
                std::inserter(out, out.end()),
                [](const auto& item) { return item->str(); });

            return out;
        };
        const auto byUnit = [&] {
            auto out = byUnitOTX();
            auto bc = byUnitBlockchain();
            std::move(bc.begin(), bc.end(), std::inserter(out, out.end()));

            return out;
        };

        if (all) {
            const auto nym = byNym();
            const auto server = byServer();
            const auto unit = byUnit();
            auto temp = std::pmr::set<std::string>{};
            std::set_intersection(
                nym.begin(),
                nym.end(),
                server.begin(),
                server.end(),
                std::inserter(temp, temp.end()));
            std::set_intersection(
                temp.begin(),
                temp.end(),
                unit.begin(),
                unit.end(),
                std::back_inserter(ids));
        } else if (nymAndServer) {
            const auto nym = byNym();
            const auto server = byServer();
            std::set_intersection(
                nym.begin(),
                nym.end(),
                server.begin(),
                server.end(),
                std::back_inserter(ids));
        } else if (nymAndUnit) {
            const auto nym = byNym();
            const auto unit = byUnit();
            std::set_intersection(
                nym.begin(),
                nym.end(),
                unit.begin(),
                unit.end(),
                std::back_inserter(ids));
        } else if (serverAndUnit) {
            const auto server = byServer();
            const auto unit = byUnit();
            std::set_intersection(
                server.begin(),
                server.end(),
                unit.begin(),
                unit.end(),
                std::back_inserter(ids));
        } else if (nymOnly) {
            auto data = byNym();
            std::move(data.begin(), data.end(), std::back_inserter(ids));
        } else if (serverOnly) {
            auto data = byServer();
            std::move(data.begin(), data.end(), std::back_inserter(ids));
        } else if (unitOnly) {
            auto data = byUnit();
            std::move(data.begin(), data.end(), std::back_inserter(ids));
        } else {
            const auto otx = session.Storage().AccountList();
            const auto bc = session.Crypto().Blockchain().AccountList();
            std::transform(
                otx.begin(),
                otx.end(),
                std::back_inserter(ids),
                [](const auto& item) { return item.first; });
            std::transform(
                bc.begin(),
                bc.end(),
                std::back_inserter(ids),
                [](const auto& item) { return item->str(); });
        }

        return reply(status(ids));
    } catch (...) {

        return reply(ResponseCode::bad_session);
    }
}
}  // namespace opentxs::rpc::implementation
