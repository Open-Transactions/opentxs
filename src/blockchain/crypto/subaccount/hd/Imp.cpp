// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "blockchain/crypto/subaccount/hd/Imp.hpp"  // IWYU pragma: associated

#include <opentxs/protobuf/BlockchainAddress.pb.h>
#include <opentxs/protobuf/BlockchainDeterministicAccountData.pb.h>
#include <opentxs/protobuf/BlockchainHDAccountData.pb.h>
#include <cstdint>
#include <memory>
#include <tuple>
#include <utility>

#include "blockchain/crypto/element/Element.hpp"
#include "blockchain/crypto/subaccount/deterministic/Imp.hpp"
#include "internal/api/crypto/Seed.hpp"
#include "internal/api/session/Storage.hpp"
#include "internal/blockchain/crypto/Element.hpp"
#include "opentxs/api/Session.hpp"
#include "opentxs/api/crypto/Config.hpp"
#include "opentxs/api/crypto/Seed.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Storage.hpp"
#include "opentxs/blockchain/crypto/Account.hpp"
#include "opentxs/blockchain/crypto/Bip44Subchain.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/crypto/Element.hpp"
#include "opentxs/blockchain/crypto/HD.hpp"
#include "opentxs/blockchain/crypto/SubaccountType.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/blockchain/crypto/Types.internal.hpp"
#include "opentxs/blockchain/crypto/Wallet.hpp"
#include "opentxs/core/Amount.hpp"  // IWYU pragma: keep
#include "opentxs/core/Data.hpp"
#include "opentxs/crypto/Bip32Child.hpp"    // IWYU pragma: keep
#include "opentxs/crypto/Bip43Purpose.hpp"  // IWYU pragma: keep
#include "opentxs/crypto/Types.hpp"
#include "opentxs/identifier/Account.hpp"
#include "opentxs/identifier/HDSeed.hpp"
#include "opentxs/protobuf/Types.internal.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::blockchain::crypto
{
HDPrivate::HDPrivate(
    const api::Session& api,
    const crypto::Account& parent,
    const identifier::Account& id,
    const protobuf::HDPath& path,
    const HDProtocol standard,
    const PasswordPrompt& reason,
    opentxs::crypto::SeedID seed) noexcept(false)
    : DeterministicPrivate(
          api,
          parent,
          SubaccountType::HD,
          id,
          seed,
          api.Crypto().Seed().SeedDescription(seed),
          get_name(path, standard),
          path,
          {api, internal_type_, false, external_type_, true})
    , standard_(standard)
    , version_(DefaultVersion)
    , cached_internal_()
    , cached_external_()
    , self_()
{
    init(reason);
}

HDPrivate::HDPrivate(
    const api::Session& api,
    const crypto::Account& parent,
    const identifier::Account& id,
    const SerializedType& serialized,
    opentxs::crypto::SeedID seed,
    HDProtocol standard) noexcept(false)
    : DeterministicPrivate(
          api,
          parent,
          SubaccountType::HD,
          id,
          serialized.deterministic(),
          seed,
          api.Crypto().Seed().SeedDescription(seed),
          get_name(serialized.deterministic().path(), standard),
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
                              Self(),
                              base_chain(parent.Target()),
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
                              Self(),
                              base_chain(parent.Target()),
                              external_type_,
                              address)));
              }

              return out;
          }())
    , standard_(standard)
    , version_(serialized.version())
    , cached_internal_()
    , cached_external_()
    , self_()
{
    init(true);
}

HDPrivate::HDPrivate(
    const api::Session& api,
    const crypto::Account& parent,
    const identifier::Account& id,
    const SerializedType& serialized,
    opentxs::crypto::SeedID seed) noexcept(false)
    : HDPrivate(api, parent, id, serialized, std::move(seed), [&] {
        if (serialized.has_hd() && (0 != serialized.hd().standard())) {

            return static_cast<HDProtocol>(serialized.hd().standard());
        } else {

            return get_standard(serialized.deterministic().path());
        }
    }())
{
}

auto HDPrivate::account_already_exists(const rLock&) const noexcept -> bool
{
    const auto existing = api_.Storage().Internal().BlockchainAccountList(
        parent_.NymID(), target_to_unit(target_));

    return existing.contains(id_);
}

auto HDPrivate::InitSelf(std::shared_ptr<Subaccount> me) noexcept -> void
{
    self_.emplace(me);
}

auto HDPrivate::PrivateKey(
    const Subchain type,
    const Bip32Index index,
    const PasswordPrompt& reason) const noexcept
    -> const opentxs::crypto::asymmetric::key::EllipticCurve
{
    switch (type) {
        case internal_type_:
        case external_type_: {
        } break;
        case Subchain::Error: {

            LogAbort()().Abort();
        }
        default: {
            LogError()()("Invalid subchain (")(print(type))("). Only ")(
                print(internal_type_))(" and ")(print(external_type_))(
                " are valid for this account.")
                .Flush();

            return opentxs::crypto::asymmetric::key::EllipticCurve::Blank();
        }
    }

    if (false == api::crypto::HaveHDKeys()) {
        return opentxs::crypto::asymmetric::key::EllipticCurve::Blank();
    }

    using enum Bip44Subchain;
    const auto change = (internal_type_ == type) ? internal : external;
    auto& key = (internal_type_ == type) ? cached_internal_ : cached_external_;
    auto lock = rLock{lock_};

    if (false == key.IsValid()) {
        key = api_.Crypto().Seed().Internal().AccountKey(path_, change, reason);

        if (false == key.IsValid()) {
            LogError()()("Failed to derive account key").Flush();

            return opentxs::crypto::asymmetric::key::EllipticCurve::Blank();
        }
    }

    return key.ChildKey(index, reason);
}

auto HDPrivate::save(const rLock& lock) const noexcept -> bool
{
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

    const bool saved = api_.Storage().Internal().Store(
        parent_.NymID(), target_to_unit(target_), serialized);

    if (saved) {
        LogTrace()()("Saved ")(print(parent_.Target()))(" HD subaccount ")(
            id_, api_.Crypto())
            .Flush();

        return true;
    } else {
        LogError()()("Failed to save HD account.").Flush();

        return false;
    }
}

HDPrivate::~HDPrivate() = default;
}  // namespace opentxs::blockchain::crypto
