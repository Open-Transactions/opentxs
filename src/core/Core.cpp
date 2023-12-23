// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/core/Core.hpp"  // IWYU pragma: associated

#include <cstdint>
#include <mutex>
#include <span>
#include <sstream>
#include <utility>

#include "internal/blockchain/params/ChainData.hpp"
#include "internal/util/Mutex.hpp"
#include "opentxs/api/Session.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/identifier/Notary.hpp"
#include "opentxs/identifier/Nym.hpp"
#include "opentxs/identifier/UnitDefinition.hpp"
#include "opentxs/util/Container.hpp"

namespace opentxs::blockchain
{
auto AccountName(const blockchain::Type chain) noexcept -> UnallocatedCString
{
    auto out = std::stringstream{};
    out << "On chain ";
    out << ticker_symbol(chain);
    out << " (this device)";

    return out.str();
}

auto Chain(const api::Session& api, const identifier::Nym& id) noexcept
    -> blockchain::Type
{
    static const auto data = [&] {
        auto out = UnallocatedMap<identifier::Nym, blockchain::Type>{};

        for (const auto& chain : blockchain::defined_chains()) {
            out.emplace(IssuerID(api, chain), chain);
        }

        return out;
    }();

    try {

        return data.at(id);
    } catch (...) {

        return blockchain::Type::UnknownBlockchain;
    }
}

auto Chain(const api::Session& api, const identifier::Notary& id) noexcept
    -> blockchain::Type
{
    static const auto data = [&] {
        auto out = UnallocatedMap<identifier::Notary, blockchain::Type>{};

        for (const auto& chain : blockchain::defined_chains()) {
            out.emplace(NotaryID(api, chain), chain);
        }

        return out;
    }();

    try {

        return data.at(id);
    } catch (...) {

        return blockchain::Type::UnknownBlockchain;
    }
}

auto Chain(
    const api::Session& api,
    const identifier::UnitDefinition& id) noexcept -> blockchain::Type
{
    static const auto data = [&] {
        auto out =
            UnallocatedMap<identifier::UnitDefinition, blockchain::Type>{};

        for (const auto& chain : blockchain::defined_chains()) {
            out.emplace(UnitID(api, chain), chain);
        }

        return out;
    }();

    try {

        return data.at(id);
    } catch (...) {

        return blockchain::Type::UnknownBlockchain;
    }
}

auto IssuerID(const api::Session& api, const blockchain::Type chain) noexcept
    -> const identifier::Nym&
{
    static auto mutex = std::mutex{};
    static auto map = UnallocatedMap<blockchain::Type, identifier::Nym>{};

    auto lock = Lock{mutex};

    {
        auto it = map.find(chain);

        if (map.end() != it) { return it->second; }
    }

    auto [it, notUsed] = map.emplace(chain, identifier::Nym{});
    auto& output = it->second;

    try {
        const auto& genesis = params::get(chain).GenesisHash();
        output = api.Factory().NymIDFromPreimage(genesis.Bytes());
    } catch (...) {
    }

    return output;
}

auto NotaryID(const api::Session& api, const blockchain::Type chain) noexcept
    -> const identifier::Notary&
{
    static auto mutex = std::mutex{};
    static auto map = UnallocatedMap<blockchain::Type, identifier::Notary>{};

    auto lock = Lock{mutex};

    {
        auto it = map.find(chain);

        if (map.end() != it) { return it->second; }
    }

    auto [it, notUsed] = map.emplace(chain, identifier::Notary{});
    auto& output = it->second;
    const auto preimage = UnallocatedCString{"blockchain-"} +
                          std::to_string(static_cast<std::uint32_t>(chain));
    output = api.Factory().NotaryIDFromPreimage(preimage);

    return output;
}

auto UnitID(const api::Session& api, const blockchain::Type chain) noexcept
    -> const identifier::UnitDefinition&
{
    static auto mutex = std::mutex{};
    static auto map =
        UnallocatedMap<blockchain::Type, identifier::UnitDefinition>{};

    auto lock = Lock{mutex};

    {
        auto it = map.find(chain);

        if (map.end() != it) { return it->second; }
    }

    auto [it, notUsed] = map.emplace(chain, identifier::UnitDefinition{});
    auto& output = it->second;

    try {
        const auto preimage = ticker_symbol(chain);
        output = api.Factory().UnitIDFromPreimage(preimage);
    } catch (...) {
    }

    return output;
}
}  // namespace opentxs::blockchain
