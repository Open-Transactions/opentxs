// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>
#include <memory>

#include "internal/core/String.hpp"
#include "opentxs/Time.hpp"
#include "opentxs/otx/blind/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"
#include "otx/blind/token/Imp.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

namespace identity
{
class Nym;
}  // namespace identity

namespace otx
{
namespace blind
{
namespace internal
{
class Purse;
}  // namespace internal

class Mint;
}  // namespace blind
}  // namespace otx

namespace protobuf
{
class Ciphertext;
class LucreTokenData;
class Token;
}  // namespace protobuf

class PasswordPrompt;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::otx::blind::token
{
class Lucre final : public Token
{
public:
    auto GetSpendable(String& output, const PasswordPrompt& reason) const
        -> bool;
    auto ID(const PasswordPrompt& reason) const -> UnallocatedCString final;
    auto IsSpent(const PasswordPrompt& reason) const -> bool final;
    auto Serialize(protobuf::Token& out) const noexcept -> bool final;

    auto AddSignature(const String& signature) -> bool;
    auto ChangeOwner(
        blind::internal::Purse& oldOwner,
        blind::internal::Purse& newOwner,
        const PasswordPrompt& reason) -> bool final;
    auto GenerateTokenRequest(
        const identity::Nym& owner,
        const blind::Mint& mint,
        const PasswordPrompt& reason) -> bool final;
    auto GetPublicPrototoken(String& output, const PasswordPrompt& reason)
        -> bool;
    auto MarkSpent(const PasswordPrompt& reason) -> bool final;
    auto Process(
        const identity::Nym& owner,
        const blind::Mint& mint,
        const PasswordPrompt& reason) -> bool final;

    Lucre(
        const api::Session& api,
        blind::internal::Purse& purse,
        const protobuf::Token& serialized);
    Lucre(
        const api::Session& api,
        const identity::Nym& owner,
        const blind::Mint& mint,
        const Denomination value,
        blind::internal::Purse& purse,
        const PasswordPrompt& reason);
    Lucre(const Lucre& rhs, blind::internal::Purse& newOwner);
    Lucre() = delete;
    Lucre(const Lucre&);
    Lucre(Lucre&&) = delete;
    auto operator=(const Lucre&) -> Lucre& = delete;
    auto operator=(Lucre&&) -> Lucre& = delete;

    ~Lucre() final = default;

private:
    const VersionNumber lucre_version_;
    OTString signature_;
    std::shared_ptr<protobuf::Ciphertext> private_;
    std::shared_ptr<protobuf::Ciphertext> public_;
    std::shared_ptr<protobuf::Ciphertext> spend_;

    void serialize_private(protobuf::LucreTokenData& lucre) const;
    void serialize_public(protobuf::LucreTokenData& lucre) const;
    void serialize_signature(protobuf::LucreTokenData& lucre) const;
    void serialize_spendable(protobuf::LucreTokenData& lucre) const;

    auto clone() const noexcept -> Imp* final { return new Lucre(*this); }

    Lucre(
        const api::Session& api,
        blind::internal::Purse& purse,
        const VersionNumber version,
        const blind::TokenState state,
        const std::uint64_t series,
        const Denomination value,
        const Time validFrom,
        const Time validTo,
        const String& signature,
        const std::shared_ptr<protobuf::Ciphertext> publicKey,
        const std::shared_ptr<protobuf::Ciphertext> privateKey,
        const std::shared_ptr<protobuf::Ciphertext> spendable);
};
}  // namespace opentxs::otx::blind::token
