// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
#include "0_stdafx.hpp"            // IWYU pragma: associated
#include "1_Internal.hpp"          // IWYU pragma: associated
#include "internal/core/Core.hpp"  // IWYU pragma: associated

#include <robin_hood.h>
#include <cstdint>
#include <iosfwd>
#include <mutex>
#include <sstream>
#include <string_view>
#include <utility>

#include "internal/blockchain/Params.hpp"
#include "internal/identity/wot/claim/Types.hpp"
#include "internal/serialization/protobuf/verify/VerifyContacts.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/core/AccountType.hpp"
#include "opentxs/core/AddressType.hpp"
#include "opentxs/core/Types.hpp"
#include "opentxs/core/UnitType.hpp"
#include "opentxs/core/identifier/Notary.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/identity/wot/claim/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "serialization/protobuf/ContractEnums.pb.h"
#include "util/Container.hpp"

namespace opentxs::blockchain
{
auto AccountName(const blockchain::Type chain) noexcept -> UnallocatedCString
{
    auto out = std::stringstream{};
    out << "On chain ";
    out << blockchain::TickerSymbol(chain);
    out << " (this device)";

    return out.str();
}

auto Chain(const api::Session& api, const identifier::Nym& id) noexcept
    -> blockchain::Type
{
    static const auto data = [&] {
        auto out = UnallocatedMap<OTNymID, blockchain::Type>{};

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

auto Chain(const api::Session& api, const identifier::Notary& id) noexcept
    -> blockchain::Type
{
    static const auto data = [&] {
        auto out = UnallocatedMap<OTNotaryID, blockchain::Type>{};

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
        auto out = UnallocatedMap<OTUnitID, blockchain::Type>{};

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
    static auto map = UnallocatedMap<blockchain::Type, OTNymID>{};

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
    -> const identifier::Notary&
{
    static auto mutex = std::mutex{};
    static auto map = UnallocatedMap<blockchain::Type, OTNotaryID>{};

    auto lock = Lock{mutex};

    {
        auto it = map.find(chain);

        if (map.end() != it) { return it->second; }
    }

    auto [it, notUsed] = map.emplace(chain, api.Factory().ServerID());
    auto& output = it->second;
    const auto preimage = UnallocatedCString{"blockchain-"} +
                          std::to_string(static_cast<std::uint32_t>(chain));
    output->CalculateDigest(preimage);

    return output;
}

auto UnitID(const api::Session& api, const blockchain::Type chain) noexcept
    -> const identifier::UnitDefinition&
{
    static auto mutex = std::mutex{};
    static auto map = UnallocatedMap<blockchain::Type, OTUnitID>{};

    auto lock = Lock{mutex};

    {
        auto it = map.find(chain);

        if (map.end() != it) { return it->second; }
    }

    auto [it, notUsed] = map.emplace(chain, api.Factory().UnitID());
    auto& output = it->second;

    try {
        const auto preimage = TickerSymbol(chain);
        output->CalculateDigest(preimage);
    } catch (...) {
    }

    return output;
}
}  // namespace opentxs::blockchain

namespace opentxs
{
using AddressTypeMap =
    robin_hood::unordered_flat_map<AddressType, proto::AddressType>;
using AddressTypeReverseMap =
    robin_hood::unordered_flat_map<proto::AddressType, AddressType>;

static auto addresstype_map() noexcept -> const AddressTypeMap&
{
    static const auto map = AddressTypeMap{
        {AddressType::Error, proto::ADDRESSTYPE_ERROR},
        {AddressType::IPV4, proto::ADDRESSTYPE_IPV4},
        {AddressType::IPV6, proto::ADDRESSTYPE_IPV6},
        {AddressType::Onion2, proto::ADDRESSTYPE_ONION},
        {AddressType::EEP, proto::ADDRESSTYPE_EEP},
        {AddressType::Inproc, proto::ADDRESSTYPE_INPROC},
    };

    return map;
}
}  // namespace opentxs

namespace opentxs
{
using namespace std::literals;

auto print(AccountType in) noexcept -> std::string_view
{
    static const auto map =
        robin_hood::unordered_flat_map<AccountType, std::string_view>{
            {AccountType::Blockchain, "blockchain"sv},
            {AccountType::Custodial, "custodial"sv},
        };

    try {

        return map.at(in);
    } catch (...) {

        return "invalid"sv;
    }
}

auto print(AddressType in) noexcept -> std::string_view
{
    static const auto map =
        robin_hood::unordered_flat_map<AddressType, std::string_view>{
            {AddressType::IPV4, "ipv4"sv},
            {AddressType::IPV6, "ipv6"sv},
            {AddressType::Onion2, "onion"sv},
            {AddressType::EEP, "eep"sv},
            {AddressType::Inproc, "inproc"sv},
        };

    try {

        return map.at(in);
    } catch (...) {

        return "invalid"sv;
    }
}

auto print(UnitType in) noexcept -> std::string_view
{
    return proto::TranslateItemType(translate(UnitToClaim(in)));
}

auto translate(AddressType in) noexcept -> proto::AddressType
{
    try {
        return addresstype_map().at(in);
    } catch (...) {
        return proto::ADDRESSTYPE_ERROR;
    }
}

auto translate(proto::AddressType in) noexcept -> AddressType
{
    static const auto map = reverse_arbitrary_map<
        AddressType,
        proto::AddressType,
        AddressTypeReverseMap>(addresstype_map());

    try {
        return map.at(in);
    } catch (...) {
        return AddressType::Error;
    }
}
}  // namespace opentxs
