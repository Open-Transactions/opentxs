// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"              // IWYU pragma: associated
#include "blockchain/crypto/HD.hpp"  // IWYU pragma: associated

#include <BlockchainAddress.pb.h>
#include <BlockchainHDAccountData.pb.h>
#include <HDAccount.pb.h>
#include <HDPath.pb.h>
#include <robin_hood.h>
#include <cstdint>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string_view>
#include <tuple>
#include <utility>

#include "Proto.hpp"
#include "blockchain/crypto/Deterministic.hpp"
#include "blockchain/crypto/Element.hpp"
#include "blockchain/crypto/Subaccount.hpp"
#include "internal/api/FactoryAPI.hpp"
#include "internal/api/crypto/Seed.hpp"
#include "internal/blockchain/crypto/Factory.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/crypto/Config.hpp"
#include "opentxs/api/crypto/Seed.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/api/session/Storage.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/crypto/Account.hpp"
#include "opentxs/blockchain/crypto/Element.hpp"
#include "opentxs/blockchain/crypto/HDProtocol.hpp"
#include "opentxs/blockchain/crypto/SubaccountType.hpp"
#include "opentxs/blockchain/crypto/Subchain.hpp"
#include "opentxs/blockchain/crypto/Wallet.hpp"
#include "opentxs/core/Amount.hpp"  // IWYU pragma: keep
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/crypto/Bip32.hpp"
#include "opentxs/crypto/Bip32Child.hpp"
#include "opentxs/crypto/Bip43Purpose.hpp"
#include "opentxs/identity/wot/claim/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "util/HDIndex.hpp"

namespace opentxs::factory
{
auto BlockchainHDSubaccount(
    const api::Session& api,
    const blockchain::crypto::Account& parent,
    const proto::HDPath& path,
    const blockchain::crypto::HDProtocol standard,
    const PasswordPrompt& reason,
    identifier::Generic& id) noexcept -> std::unique_ptr<blockchain::crypto::HD>
{
    using ReturnType = blockchain::crypto::implementation::HD;

    try {
        return std::make_unique<ReturnType>(
            api, parent, path, standard, reason, id);
    } catch (const std::exception& e) {
        LogVerbose()("opentxs::Factory::")(__func__)(": ")(e.what()).Flush();

        return nullptr;
    }
}

auto BlockchainHDSubaccount(
    const api::Session& api,
    const blockchain::crypto::Account& parent,
    const proto::HDAccount& serialized,
    identifier::Generic& id) noexcept -> std::unique_ptr<blockchain::crypto::HD>
{
    using ReturnType = blockchain::crypto::implementation::HD;

    try {
        return std::make_unique<ReturnType>(api, parent, serialized, id);
    } catch (const std::exception& e) {
        LogError()("opentxs::Factory::")(__func__)(": ")(e.what()).Flush();

        return nullptr;
    }
}
}  // namespace opentxs::factory

namespace opentxs::blockchain::crypto::implementation
{
HD::HD(
    const api::Session& api,
    const crypto::Account& parent,
    const proto::HDPath& path,
    const HDProtocol standard,
    const PasswordPrompt& reason,
    identifier::Generic& id) noexcept(false)
    : Deterministic(
          api,
          parent,
          SubaccountType::HD,
          api.Factory().Internal().Identifier(
              UnitToClaim(BlockchainToUnit(parent.Chain())),
              path),
          path,
          {api, internal_type_, false, external_type_, true},
          id)
    , standard_(standard)
    , version_(DefaultVersion)
    , cached_internal_()
    , cached_external_()
    , name_()
{
    init(reason);
}

HD::HD(
    const api::Session& api,
    const crypto::Account& parent,
    const SerializedType& serialized,
    identifier::Generic& id) noexcept(false)
    : Deterministic(
          api,
          parent,
          SubaccountType::HD,
          serialized.deterministic(),
          serialized.internaladdress().size(),
          serialized.externaladdress().size(),
          [&] {
              auto out =
                  ChainData{api, internal_type_, false, external_type_, true};
              auto& internal = out.internal_.map_;
              auto& external = out.external_.map_;

              for (const auto& address : serialized.internaladdress()) {
                  internal.emplace(
                      std::piecewise_construct,
                      std::forward_as_tuple(address.index()),
                      std::forward_as_tuple(
                          std::make_unique<implementation::Element>(
                              api,
                              parent.Parent().Parent(),
                              *this,
                              parent.Chain(),
                              internal_type_,
                              address)));
              }

              for (const auto& address : serialized.externaladdress()) {
                  external.emplace(
                      std::piecewise_construct,
                      std::forward_as_tuple(address.index()),
                      std::forward_as_tuple(
                          std::make_unique<implementation::Element>(
                              api,
                              parent.Parent().Parent(),
                              *this,
                              parent.Chain(),
                              external_type_,
                              address)));
              }

              return out;
          }(),
          id)
    , standard_([&] {
        if (serialized.has_hd() && (0 != serialized.hd().standard())) {

            return static_cast<HDProtocol>(serialized.hd().standard());
        }

        if (0 < path_.child().size()) {
            using Index = opentxs::HDIndex<Bip43Purpose>;

            static const auto map =
                robin_hood::unordered_flat_map<Bip32Index, HDProtocol>{
                    {Index{Bip43Purpose::HDWALLET, Bip32Child::HARDENED},
                     HDProtocol::BIP_44},
                    {Index{Bip43Purpose::P2SH_P2WPKH, Bip32Child::HARDENED},
                     HDProtocol::BIP_49},
                    {Index{Bip43Purpose::P2WPKH, Bip32Child::HARDENED},
                     HDProtocol::BIP_84},
                };

            try {

                return map.at(path_.child(0));
            } catch (...) {
            }
        }

        return HDProtocol::BIP_32;
    }())
    , version_(serialized.version())
    , cached_internal_()
    , cached_external_()
    , name_()
{
    init();
}

auto HD::account_already_exists(const rLock&) const noexcept -> bool
{
    const auto existing = api_.Storage().BlockchainAccountList(
        parent_.NymID(), BlockchainToUnit(chain_));

    return 0 < existing.count(id_.asBase58(api_.Crypto()));
}

auto HD::Name() const noexcept -> UnallocatedCString
{
    auto lock = rLock{lock_};

    if (false == name_.has_value()) {
        auto name = std::stringstream{};
        name << print(standard_);
        name << ": ";
        name << opentxs::crypto::Print(path_, false);
        name_ = name.str();
    }

    OT_ASSERT(name_.has_value());

    return name_.value();
}

auto HD::PrivateKey(
    const Subchain type,
    const Bip32Index index,
    const PasswordPrompt& reason) const noexcept -> ECKey
{
    switch (type) {
        case internal_type_:
        case external_type_: {
        } break;
        case Subchain::Error: {

            OT_FAIL;
        }
        default: {
            LogError()(OT_PRETTY_CLASS())("Invalid subchain (")(print(type))(
                "). Only ")(print(internal_type_))(" and ")(
                print(external_type_))(" are valid for this account.")
                .Flush();

            return {};
        }
    }

    if (false == api::crypto::HaveHDKeys()) { return {}; }

    const auto change =
        (internal_type_ == type) ? INTERNAL_CHAIN : EXTERNAL_CHAIN;
    auto& pKey = (internal_type_ == type) ? cached_internal_ : cached_external_;
    auto lock = rLock{lock_};

    if (!pKey) {
        pKey =
            api_.Crypto().Seed().Internal().AccountKey(path_, change, reason);

        if (!pKey) {
            LogError()(OT_PRETTY_CLASS())("Failed to derive account key")
                .Flush();

            return {};
        }
    }

    OT_ASSERT(pKey);

    const auto& key = *pKey;

    return key.ChildKey(index, reason);
}

auto HD::save(const rLock& lock) const noexcept -> bool
{
    const auto type = BlockchainToUnit(chain_);
    auto serialized = SerializedType{};
    serialized.set_version(version_);
    serialize_deterministic(lock, *serialized.mutable_deterministic());

    for (const auto& [index, address] : data_.internal_.map_) {
        *serialized.add_internaladdress() = address->Internal().Serialize();
    }

    for (const auto& [index, address] : data_.external_.map_) {
        *serialized.add_externaladdress() = address->Internal().Serialize();
    }

    {
        auto& hd = *serialized.mutable_hd();
        hd.set_version(proto_hd_version_);
        hd.set_standard(static_cast<std::uint16_t>(standard_));
    }

    const bool saved =
        api_.Storage().Store(parent_.NymID(), UnitToClaim(type), serialized);

    if (false == saved) {
        LogError()(OT_PRETTY_CLASS())("Failed to save HD account.").Flush();

        return false;
    }

    return saved;
}
}  // namespace opentxs::blockchain::crypto::implementation
