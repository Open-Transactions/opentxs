// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <HDAccount.pb.h>
#include <memory>
#include <optional>

#include "blockchain/crypto/element/Element.hpp"
#include "blockchain/crypto/subaccount/base/Imp.hpp"
#include "blockchain/crypto/subaccount/deterministic/Imp.hpp"
#include "internal/blockchain/crypto/HD.hpp"
#include "internal/util/Mutex.hpp"
#include "opentxs/blockchain/crypto/Deterministic.hpp"
#include "opentxs/blockchain/crypto/HD.hpp"
#include "opentxs/blockchain/crypto/Subaccount.hpp"
#include "opentxs/blockchain/crypto/Subchain.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/core/identifier/Account.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/crypto/asymmetric/key/EllipticCurve.hpp"
#include "opentxs/crypto/asymmetric/key/HD.hpp"
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
namespace internal
{
class Subaccount;
}  // namespace internal

class Account;
}  // namespace crypto
}  // namespace blockchain

namespace proto
{
class HDPath;
}  // namespace proto

class PasswordPrompt;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::crypto
{
class HDPrivate final : public internal::HD, public DeterministicPrivate
{
public:
    using Element = implementation::Element;
    using SerializedType = proto::HDAccount;

    auto asDeterministicPublic() const noexcept
        -> const crypto::Deterministic& final
    {
        return const_cast<HDPrivate*>(this)->asHDPublic();
    }
    auto asHD() const noexcept -> const internal::HD& final { return *this; }
    auto asHDPublic() const noexcept -> const crypto::HD& final
    {
        return const_cast<HDPrivate*>(this)->asHDPublic();
    }
    using SubaccountPrivate::PrivateKey;
    auto PrivateKey(
        const Subchain type,
        const Bip32Index index,
        const PasswordPrompt& reason) const noexcept
        -> const opentxs::crypto::asymmetric::key::EllipticCurve final;
    auto Self() const noexcept -> const crypto::Subaccount& final
    {
        return asHDPublic();
    }
    auto Standard() const noexcept -> HDProtocol final { return standard_; }

    auto asDeterministicPublic() noexcept -> crypto::Deterministic& final
    {
        return asHDPublic();
    }
    auto asHD() noexcept -> internal::HD& final { return *this; }
    auto asHDPublic() noexcept -> crypto::HD& final { return *self_; }
    auto InitSelf(std::shared_ptr<Subaccount> me) noexcept -> void final;
    auto Self() noexcept -> crypto::Subaccount& final { return asHDPublic(); }

    HDPrivate(
        const api::Session& api,
        const crypto::Account& parent,
        const identifier::Account& id,
        const proto::HDPath& path,
        const HDProtocol standard,
        const PasswordPrompt& reason,
        opentxs::crypto::SeedID seed) noexcept(false);
    HDPrivate(
        const api::Session& api,
        const crypto::Account& parent,
        const identifier::Account& id,
        const SerializedType& serialized,
        opentxs::crypto::SeedID seed) noexcept(false);
    HDPrivate(const HDPrivate&) = delete;
    HDPrivate(HDPrivate&&) = delete;
    auto operator=(const HDPrivate&) -> HDPrivate& = delete;
    auto operator=(HDPrivate&&) -> HDPrivate& = delete;

    ~HDPrivate() final;

private:
    using Me = std::optional<crypto::HD>;

    static constexpr auto internal_type_{Subchain::Internal};
    static constexpr auto external_type_{Subchain::External};
    static constexpr VersionNumber DefaultVersion{1};
    static constexpr auto proto_hd_version_ = VersionNumber{1};

    const HDProtocol standard_;
    VersionNumber version_;
    mutable opentxs::crypto::asymmetric::key::HD cached_internal_;
    mutable opentxs::crypto::asymmetric::key::HD cached_external_;
    mutable Me self_;

    static auto name(const proto::HDPath& path, HDProtocol type) noexcept
        -> UnallocatedCString;

    auto account_already_exists(const rLock& lock) const noexcept -> bool final;
    auto save(const rLock& lock) const noexcept -> bool final;

    HDPrivate(
        const api::Session& api,
        const crypto::Account& parent,
        const identifier::Account& id,
        const SerializedType& serialized,
        opentxs::crypto::SeedID seed,
        HDProtocol standard) noexcept(false);
};
}  // namespace opentxs::blockchain::crypto
