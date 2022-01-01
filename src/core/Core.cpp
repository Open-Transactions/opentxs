// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
#include "0_stdafx.hpp"            // IWYU pragma: associated
#include "1_Internal.hpp"          // IWYU pragma: associated
#include "internal/core/Core.hpp"  // IWYU pragma: associated

#include <robin_hood.h>
#include <cstdint>
#include <map>
#include <mutex>
#include <set>
#include <utility>

#include "internal/blockchain/Params.hpp"
#include "internal/protobuf/verify/VerifyContacts.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/core/AddressType.hpp"
#include "opentxs/core/Types.hpp"
#include "opentxs/core/UnitType.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/identifier/Server.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "serialization/protobuf/ContractEnums.pb.h"
#include "util/Container.hpp"

namespace opentxs::blockchain
{
auto AccountName([[maybe_unused]] const blockchain::Type chain) noexcept
    -> std::string
{
    return "This device";
}

auto Chain(const api::Session& api, const identifier::Nym& id) noexcept
    -> blockchain::Type
{
    static const auto data = [&] {
        auto out = std::pmr::map<OTNymID, blockchain::Type>{};

        for (const auto& chain : blockchain::DefinedChains()) {
            out.emplace(IssuerID(api, chain), chain);
        }

        return out;
    }();

    try {

        return data.at(id);
    } catch (...) {

        return blockchain::Type::Unknown;
    }
}

auto Chain(const api::Session& api, const identifier::Server& id) noexcept
    -> blockchain::Type
{
    static const auto data = [&] {
        auto out = std::pmr::map<OTServerID, blockchain::Type>{};

        for (const auto& chain : blockchain::DefinedChains()) {
            out.emplace(NotaryID(api, chain), chain);
        }

        return out;
    }();

    try {

        return data.at(id);
    } catch (...) {

        return blockchain::Type::Unknown;
    }
}

auto Chain(
    const api::Session& api,
    const identifier::UnitDefinition& id) noexcept -> blockchain::Type
{
    static const auto data = [&] {
        auto out = std::pmr::map<OTUnitID, blockchain::Type>{};

        for (const auto& chain : blockchain::DefinedChains()) {
            out.emplace(UnitID(api, chain), chain);
        }

        return out;
    }();

    try {

        return data.at(id);
    } catch (...) {

        return blockchain::Type::Unknown;
    }
}

auto IssuerID(const api::Session& api, const blockchain::Type chain) noexcept
    -> const identifier::Nym&
{
    static auto mutex = std::mutex{};
    static auto map = std::pmr::map<blockchain::Type, OTNymID>{};

    auto lock = Lock{mutex};

    {
        auto it = map.find(chain);

        if (map.end() != it) { return it->second; }
    }

    auto [it, notUsed] = map.emplace(chain, api.Factory().NymID());
    auto& output = it->second;

    try {
        const auto& hex =
            blockchain::params::Data::Chains().at(chain).genesis_hash_hex_;
        const auto genesis = api.Factory().Data(hex, StringStyle::Hex);
        output->CalculateDigest(genesis->Bytes());
    } catch (...) {
    }

    return output;
}

auto NotaryID(const api::Session& api, const blockchain::Type chain) noexcept
    -> const identifier::Server&
{
    static auto mutex = std::mutex{};
    static auto map = std::pmr::map<blockchain::Type, OTServerID>{};

    auto lock = Lock{mutex};

    {
        auto it = map.find(chain);

        if (map.end() != it) { return it->second; }
    }

    auto [it, notUsed] = map.emplace(chain, api.Factory().ServerID());
    auto& output = it->second;
    const auto preimage = std::string{"blockchain-"} +
                          std::to_string(static_cast<std::uint32_t>(chain));
    output->CalculateDigest(preimage);

    return output;
}

auto UnitID(const api::Session& api, const blockchain::Type chain) noexcept
    -> const identifier::UnitDefinition&
{
    static auto mutex = std::mutex{};
    static auto map = std::pmr::map<blockchain::Type, OTUnitID>{};

    auto lock = Lock{mutex};

    {
        auto it = map.find(chain);

        if (map.end() != it) { return it->second; }
    }

    auto [it, notUsed] = map.emplace(chain, api.Factory().UnitID());
    auto& output = it->second;

    try {
        const auto preimage =
            blockchain::params::Data::Chains().at(chain).display_ticker_;
        output->CalculateDigest(preimage);
    } catch (...) {
    }

    return output;
}
}  // namespace opentxs::blockchain

namespace opentxs::core
{
using AddressTypeMap =
    robin_hood::unordered_flat_map<AddressType, proto::AddressType>;
using AddressTypeReverseMap =
    robin_hood::unordered_flat_map<proto::AddressType, AddressType>;

auto addresstype_map() noexcept -> const AddressTypeMap&;
}  // namespace opentxs::core

namespace opentxs::core
{
auto addresstype_map() noexcept -> const AddressTypeMap&
{
    static const auto map = AddressTypeMap{
        {AddressType::Error, proto::ADDRESSTYPE_ERROR},
        {AddressType::IPV4, proto::ADDRESSTYPE_IPV4},
        {AddressType::IPV6, proto::ADDRESSTYPE_IPV6},
        {AddressType::Onion, proto::ADDRESSTYPE_ONION},
        {AddressType::EEP, proto::ADDRESSTYPE_EEP},
        {AddressType::Inproc, proto::ADDRESSTYPE_INPROC},
    };

    return map;
}
}  // namespace opentxs::core

namespace opentxs
{
auto print(core::UnitType in) noexcept -> std::string
{
    return proto::TranslateItemType(static_cast<std::uint32_t>(in));
}

auto translate(core::AddressType in) noexcept -> proto::AddressType
{
    try {
        return core::addresstype_map().at(in);
    } catch (...) {
        return proto::ADDRESSTYPE_ERROR;
    }
}

auto translate(proto::AddressType in) noexcept -> core::AddressType
{
    static const auto map = reverse_arbitrary_map<
        core::AddressType,
        proto::AddressType,
        core::AddressTypeReverseMap>(core::addresstype_map());

    try {
        return map.at(in);
    } catch (...) {
        return core::AddressType::Error;
    }
}
}  // namespace opentxs
