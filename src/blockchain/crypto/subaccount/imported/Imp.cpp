// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "blockchain/crypto/subaccount/imported/Imp.hpp"  // IWYU pragma: associated

#include <BlockchainAddress.pb.h>
#include <BlockchainImportedAccountData.pb.h>
#include <stdexcept>
#include <utility>

#include "opentxs/api/Session.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/blockchain/crypto/Account.hpp"
#include "opentxs/blockchain/crypto/SubaccountType.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/identifier/AccountSubtype.hpp"  // IWYU pragma: keep

namespace opentxs::blockchain::crypto
{
using namespace std::literals;

ImportedPrivate::ImportedPrivate(
    const api::Session& api,
    const crypto::Account& parent,
    const identifier::Account& id,
    const opentxs::crypto::asymmetric::key::EllipticCurve& key,
    identifier::Generic source,
    std::string_view sourceName,
    std::string_view name) noexcept(false)
    : SubaccountPrivate(
          api,
          parent,
          SubaccountType::Imported,
          id,
          std::move(source),
          sourceName,
          name)
    , element_(
          api_,
          api_.Crypto().Blockchain(),
          Self(),
          base_chain(parent.Target()),
          subchain_,
          index_,
          key,
          {})
    , position_()
{
}

ImportedPrivate::ImportedPrivate(
    const api::Session& api,
    const crypto::Account& parent,
    const identifier::Account& id,
    const SerializedType& serialized,
    identifier::Generic source,
    std::string_view sourceName,
    std::string_view name) noexcept(false)
    : SubaccountPrivate(
          api,
          parent,
          SubaccountType::Imported,
          id,
          std::move(source),
          sourceName,
          name,
          serialized.common())
    , element_(
          api_,
          api_.Crypto().Blockchain(),
          Self(),
          base_chain(parent.Target()),
          subchain_,
          serialized.key())
    , position_()
{
}

auto ImportedPrivate::AllowedSubchains() const noexcept -> Set<Subchain>
{
    using enum Subchain;
    static const auto allowed = Set<Subchain>{subchain_};

    return allowed;
}

auto ImportedPrivate::BalanceElement(
    const Subchain type,
    const Bip32Index index) const noexcept(false) -> const crypto::Element&
{
    if (type != subchain_) {
        throw std::runtime_error{"invalid subchain "s.append(print(type))};
    }

    if (index != index_) {
        throw std::runtime_error{
            "invalid index "s.append(std::to_string(index))};
    }

    return element_;
}

auto ImportedPrivate::mutable_element(
    const rLock& lock,
    const Subchain type,
    const Bip32Index index) noexcept(false) -> crypto::Element&
{
    if (type != subchain_) {
        throw std::runtime_error{"invalid subchain "s.append(print(type))};
    }

    if (index != index_) {
        throw std::runtime_error{
            "invalid index "s.append(std::to_string(index))};
    }

    return element_;
}

auto ImportedPrivate::PrivateKey(
    const implementation::Element& element,
    const Subchain,
    const Bip32Index,
    const PasswordPrompt&) const noexcept
    -> const opentxs::crypto::asymmetric::key::EllipticCurve&
{
    return element.Key();
}

auto ImportedPrivate::ScanProgress(Subchain type) const noexcept
    -> block::Position
{
    static const auto blank = block::Position{};

    if (subchain_ != type) { return blank; }

    auto lock = rLock{lock_};

    return position_;
}

auto ImportedPrivate::serialize_imported(const rLock& lock, SerializedType& out)
    const noexcept -> void
{
    out.set_version(imported_version_);
    serialize_common(lock, *out.mutable_common());
    *out.mutable_key() = element_.Serialize(true);
}

auto ImportedPrivate::SetScanProgress(
    const block::Position& progress,
    Subchain type) noexcept -> void
{
    if (subchain_ == type) {
        auto lock = rLock{lock_};
        position_ = progress;
    }
}
}  // namespace opentxs::blockchain::crypto
