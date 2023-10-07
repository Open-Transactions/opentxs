// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "interface/rpc/RPC.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <iterator>
#include <utility>

#include "internal/api/session/Storage.hpp"
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
        const auto& blockchain = session.Crypto().Blockchain();
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
            const auto accountids =
                session.Storage().Internal().AccountsByOwner(nym);
            std::ranges::transform(
                accountids,
                std::inserter(out, out.end()),
                [this](const auto& item) {
                    return item.asBase58(ot_.Crypto());
                });

            return out;
        };
        const auto byNymBlockchain = [&] {
            auto out = UnallocatedSet<UnallocatedCString>{};

            for (const auto& id : blockchain.AccountList(nym)) {
                const auto [chain, owner] = blockchain.LookupAccount(id);
                const auto items = blockchain.SubaccountList(nym, chain);

                if (items.empty()) { continue; }

                out.emplace(id.asBase58(ot_.Crypto()));
            }

            return out;
        };
        const auto byNym = [&] {
            auto out = byNymOTX();
            auto bc = byNymBlockchain();
            std::ranges::move(bc, std::inserter(out, out.end()));

            return out;
        };
        const auto byServerOTX = [&] {
            auto out = UnallocatedSet<UnallocatedCString>{};
            const auto accountids =
                session.Storage().Internal().AccountsByServer(notary);
            std::ranges::transform(
                accountids,
                std::inserter(out, out.end()),
                [this](const auto& item) {
                    return item.asBase58(ot_.Crypto());
                });

            return out;
        };
        const auto byServerBlockchain = [&] {
            auto out = UnallocatedSet<UnallocatedCString>{};
            const auto chain = blockchain::Chain(session, notary);

            for (const auto& id : blockchain.AccountList(chain)) {
                const auto items =
                    blockchain.SubaccountList(blockchain.Owner(id), chain);

                if (items.empty()) { continue; }

                out.emplace(id.asBase58(ot_.Crypto()));
            }

            return out;
        };
        const auto byServer = [&] {
            auto out = byServerOTX();
            auto bc = byServerBlockchain();
            std::ranges::move(bc, std::inserter(out, out.end()));

            return out;
        };
        const auto byUnitOTX = [&] {
            auto out = UnallocatedSet<UnallocatedCString>{};
            const auto accountids =
                session.Storage().Internal().AccountsByContract(unitID);
            std::ranges::transform(
                accountids,
                std::inserter(out, out.end()),
                [this](const auto& item) {
                    return item.asBase58(ot_.Crypto());
                });

            return out;
        };
        const auto byUnitBlockchain = [&] {
            auto out = UnallocatedSet<UnallocatedCString>{};
            const auto chain = blockchain::Chain(session, unitID);

            for (const auto& id : blockchain.AccountList(chain)) {
                const auto items =
                    blockchain.SubaccountList(blockchain.Owner(id), chain);

                if (items.empty()) { continue; }

                out.emplace(id.asBase58(ot_.Crypto()));
            }

            return out;
        };
        const auto byUnit = [&] {
            auto out = byUnitOTX();
            auto bc = byUnitBlockchain();
            std::ranges::move(bc, std::inserter(out, out.end()));

            return out;
        };

        if (all) {
            const auto bynym = byNym();
            const auto server = byServer();
            const auto unit = byUnit();
            auto temp = UnallocatedSet<UnallocatedCString>{};
            std::ranges::set_intersection(
                bynym, server, std::inserter(temp, temp.end()));
            std::ranges::set_intersection(temp, unit, std::back_inserter(ids));
        } else if (nymAndServer) {
            const auto bynym = byNym();
            const auto server = byServer();
            std::ranges::set_intersection(
                bynym, server, std::back_inserter(ids));
        } else if (nymAndUnit) {
            const auto bynym = byNym();
            const auto unit = byUnit();
            std::ranges::set_intersection(bynym, unit, std::back_inserter(ids));
        } else if (serverAndUnit) {
            const auto server = byServer();
            const auto unit = byUnit();
            std::ranges::set_intersection(
                server, unit, std::back_inserter(ids));
        } else if (nymOnly) {
            auto data = byNym();
            std::ranges::move(data, std::back_inserter(ids));
        } else if (serverOnly) {
            auto data = byServer();
            std::ranges::move(data, std::back_inserter(ids));
        } else if (unitOnly) {
            auto data = byUnit();
            std::ranges::move(data, std::back_inserter(ids));
        } else {
            const auto otx = session.Storage().Internal().AccountList();
            const auto bc = blockchain.AccountList();
            std::ranges::transform(
                otx, std::back_inserter(ids), [](const auto& item) {
                    return item.first;
                });

            for (const auto& id : blockchain.AccountList()) {
                const auto [chain, owner] = blockchain.LookupAccount(id);
                const auto items = blockchain.SubaccountList(owner, chain);

                if (items.empty()) { continue; }

                ids.emplace_back(id.asBase58(ot_.Crypto()));
            }
        }

        return reply(status(ids));
    } catch (...) {

        return reply(ResponseCode::bad_session);
    }
}
}  // namespace opentxs::rpc::implementation
