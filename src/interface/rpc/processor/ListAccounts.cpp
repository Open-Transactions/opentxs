// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::rpc::ResponseCode

#include "interface/rpc/RPC.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <iterator>
#include <utility>

#include "internal/core/Core.hpp"
#include "opentxs/api/Context.hpp"
#include "opentxs/api/crypto/Blockchain.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Storage.hpp"
#include "opentxs/core/identifier/Notary.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/interface/rpc/ResponseCode.hpp"  // IWYU pragma: keep
#include "opentxs/interface/rpc/Types.hpp"
#include "opentxs/interface/rpc/request/Base.hpp"
#include "opentxs/interface/rpc/request/ListAccounts.hpp"
#include "opentxs/interface/rpc/response/Base.hpp"
#include "opentxs/interface/rpc/response/ListAccounts.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Types.hpp"

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
        const auto nym = session.Factory().NymIDFromBase58(in.FilterNym());
        const auto notary =
            session.Factory().NotaryIDFromBase58(in.FilterNotary());
        const auto unitID = session.Factory().UnitIDFromBase58(in.FilterUnit());
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
            auto out = UnallocatedSet<UnallocatedCString>{};
            const auto accountids = session.Storage().AccountsByOwner(nym);
            std::transform(
                accountids.begin(),
                accountids.end(),
                std::inserter(out, out.end()),
                [this](const auto& item) {
                    return item.asBase58(ot_.Crypto());
                });

            return out;
        };
        const auto byNymBlockchain = [&] {
            auto out = UnallocatedSet<UnallocatedCString>{};
            const auto listids = session.Crypto().Blockchain().AccountList(nym);
            std::transform(
                listids.begin(),
                listids.end(),
                std::inserter(out, out.end()),
                [this](const auto& item) {
                    return item.asBase58(ot_.Crypto());
                });

            return out;
        };
        const auto byNym = [&] {
            auto out = byNymOTX();
            auto bc = byNymBlockchain();
            std::move(bc.begin(), bc.end(), std::inserter(out, out.end()));

            return out;
        };
        const auto byServerOTX = [&] {
            auto out = UnallocatedSet<UnallocatedCString>{};
            const auto accountids = session.Storage().AccountsByServer(notary);
            std::transform(
                accountids.begin(),
                accountids.end(),
                std::inserter(out, out.end()),
                [this](const auto& item) {
                    return item.asBase58(ot_.Crypto());
                });

            return out;
        };
        const auto byServerBlockchain = [&] {
            auto out = UnallocatedSet<UnallocatedCString>{};
            const auto chain = blockchain::Chain(session, notary);
            const auto listids =
                session.Crypto().Blockchain().AccountList(chain);
            std::transform(
                listids.begin(),
                listids.end(),
                std::inserter(out, out.end()),
                [this](const auto& item) {
                    return item.asBase58(ot_.Crypto());
                });

            return out;
        };
        const auto byServer = [&] {
            auto out = byServerOTX();
            auto bc = byServerBlockchain();
            std::move(bc.begin(), bc.end(), std::inserter(out, out.end()));

            return out;
        };
        const auto byUnitOTX = [&] {
            auto out = UnallocatedSet<UnallocatedCString>{};
            const auto accountids =
                session.Storage().AccountsByContract(unitID);
            std::transform(
                accountids.begin(),
                accountids.end(),
                std::inserter(out, out.end()),
                [this](const auto& item) {
                    return item.asBase58(ot_.Crypto());
                });

            return out;
        };
        const auto byUnitBlockchain = [&] {
            auto out = UnallocatedSet<UnallocatedCString>{};
            const auto chain = blockchain::Chain(session, unitID);
            const auto listids =
                session.Crypto().Blockchain().AccountList(chain);
            std::transform(
                listids.begin(),
                listids.end(),
                std::inserter(out, out.end()),
                [this](const auto& item) {
                    return item.asBase58(ot_.Crypto());
                });

            return out;
        };
        const auto byUnit = [&] {
            auto out = byUnitOTX();
            auto bc = byUnitBlockchain();
            std::move(bc.begin(), bc.end(), std::inserter(out, out.end()));

            return out;
        };

        if (all) {
            const auto bynym = byNym();
            const auto server = byServer();
            const auto unit = byUnit();
            auto temp = UnallocatedSet<UnallocatedCString>{};
            std::set_intersection(
                bynym.begin(),
                bynym.end(),
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
            const auto bynym = byNym();
            const auto server = byServer();
            std::set_intersection(
                bynym.begin(),
                bynym.end(),
                server.begin(),
                server.end(),
                std::back_inserter(ids));
        } else if (nymAndUnit) {
            const auto bynym = byNym();
            const auto unit = byUnit();
            std::set_intersection(
                bynym.begin(),
                bynym.end(),
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
                [this](const auto& item) {
                    return item.asBase58(ot_.Crypto());
                });
        }

        return reply(status(ids));
    } catch (...) {

        return reply(ResponseCode::bad_session);
    }
}
}  // namespace opentxs::rpc::implementation
