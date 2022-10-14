// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <HDAccount.pb.h>
#include <atomic>
#include <cstdint>
#include <memory>
#include <optional>

#include "blockchain/crypto/Deterministic.hpp"
#include "blockchain/crypto/Element.hpp"
#include "blockchain/crypto/Subaccount.hpp"
#include "internal/blockchain/crypto/Crypto.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "internal/util/Mutex.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/crypto/HDProtocol.hpp"
#include "opentxs/blockchain/crypto/Subaccount.hpp"
#include "opentxs/blockchain/crypto/Subchain.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
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
namespace crypto
{
class Blockchain;
}  // namespace crypto

class Session;
}  // namespace api

namespace blockchain
{
namespace crypto
{
class Account;
class HD;
}  // namespace crypto
}  // namespace blockchain

namespace identifier
{
class Generic;
}  // namespace identifier

namespace proto
{
class HDAccount;
class HDPath;
}  // namespace proto

class PasswordPrompt;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::crypto::implementation
{
class HD final : public internal::HD, public Deterministic
{
public:
    using Element = implementation::Element;
    using SerializedType = proto::HDAccount;

    auto InternalHD() const noexcept -> internal::HD& final
    {
        return const_cast<HD&>(*this);
    }
    auto Name() const noexcept -> UnallocatedCString final;
    auto PrivateKey(
        const Subchain type,
        const Bip32Index index,
        const PasswordPrompt& reason) const noexcept
        -> opentxs::crypto::asymmetric::key::EllipticCurve final;
    auto Standard() const noexcept -> HDProtocol final { return standard_; }

    HD(const api::Session& api,
       const crypto::Account& parent,
       const proto::HDPath& path,
       const HDProtocol standard,
       const PasswordPrompt& reason,
       identifier::Generic& id)
    noexcept(false);
    HD(const api::Session& api,
       const crypto::Account& parent,
       const SerializedType& serialized,
       identifier::Generic& id)
    noexcept(false);
    HD(const HD&) = delete;
    HD(HD&&) = delete;
    auto operator=(const HD&) -> HD& = delete;
    auto operator=(HD&&) -> HD& = delete;

    ~HD() final = default;

private:
    static constexpr auto internal_type_{Subchain::Internal};
    static constexpr auto external_type_{Subchain::External};
    static constexpr VersionNumber DefaultVersion{1};
    static constexpr auto proto_hd_version_ = VersionNumber{1};

    const HDProtocol standard_;
    VersionNumber version_;
    mutable opentxs::crypto::asymmetric::key::HD cached_internal_;
    mutable opentxs::crypto::asymmetric::key::HD cached_external_;
    mutable std::optional<UnallocatedCString> name_;

    auto account_already_exists(const rLock& lock) const noexcept -> bool final;
    auto save(const rLock& lock) const noexcept -> bool final;
};
}  // namespace opentxs::blockchain::crypto::implementation
