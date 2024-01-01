// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "blockchain/crypto/subaccount/base/Imp.hpp"

#include <string_view>

#include "blockchain/crypto/element/Element.hpp"
#include "internal/blockchain/crypto/Imported.hpp"
#include "internal/util/Mutex.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/blockchain/crypto/Element.hpp"
#include "opentxs/blockchain/crypto/Subchain.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/crypto/asymmetric/key/EllipticCurve.hpp"
#include "opentxs/identifier/Account.hpp"
#include "opentxs/identifier/Generic.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

namespace blockchain
{
namespace crypto
{
class Account;
}  // namespace crypto
}  // namespace blockchain

namespace protobuf
{
class BlockchainImportedAccountData;
}  // namespace protobuf

class PasswordPrompt;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::crypto
{
class ImportedPrivate : virtual public internal::Imported,
                        public SubaccountPrivate
{
public:
    using SerializedType = protobuf::BlockchainImportedAccountData;

    auto AllowedSubchains() const noexcept -> Set<Subchain> final;
    auto BalanceElement(const Subchain type, const Bip32Index index) const
        noexcept(false) -> const crypto::Element& final;
    auto asImported() const noexcept -> const internal::Imported& final
    {
        return *this;
    }
    auto PrivateKey(
        const implementation::Element& element,
        const Subchain type,
        const Bip32Index index,
        const PasswordPrompt& reason) const noexcept
        -> const opentxs::crypto::asymmetric::key::EllipticCurve& final;
    auto ScanProgress(Subchain type) const noexcept -> block::Position final;

    auto asImported() noexcept -> internal::Imported& final { return *this; }
    auto SetScanProgress(
        const block::Position& progress,
        Subchain type) noexcept -> void final;

    ImportedPrivate(
        const api::Session& api,
        const crypto::Account& parent,
        const identifier::Account& id,
        const opentxs::crypto::asymmetric::key::EllipticCurve& key,
        identifier::Generic source,
        std::string_view sourceName,
        std::string_view name) noexcept(false);
    ImportedPrivate(
        const api::Session& api,
        const crypto::Account& parent,
        const identifier::Account& id,
        const SerializedType& proto,
        identifier::Generic source,
        std::string_view sourceName,
        std::string_view name) noexcept(false);
    ImportedPrivate(const ImportedPrivate&) = delete;
    ImportedPrivate(ImportedPrivate&&) = delete;
    auto operator=(const ImportedPrivate&) -> ImportedPrivate& = delete;
    auto operator=(ImportedPrivate&&) -> ImportedPrivate& = delete;

    ~ImportedPrivate() override = default;

protected:
    static constexpr auto imported_version_ = VersionNumber{1};
    static constexpr auto subchain_ = Subchain::None;
    static constexpr auto index_ = Bip32Index{0};

    implementation::Element element_;
    block::Position position_;

    auto serialize_imported(const rLock& lock, SerializedType& out)
        const noexcept -> void;

    auto mutable_element(
        const rLock& lock,
        const Subchain type,
        const Bip32Index index) noexcept(false) -> crypto::Element& final;
};
}  // namespace opentxs::blockchain::crypto
