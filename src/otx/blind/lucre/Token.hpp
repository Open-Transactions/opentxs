// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>
#include <memory>
#include <stdexcept>

#include "Proto.hpp"
#include "Proto.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/otx/blind/Token.hpp"
#include "opentxs/otx/blind/TokenState.hpp"
#include "opentxs/otx/blind/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"
#include "opentxs/util/Time.hpp"
#include "otx/blind/token/Imp.hpp"
#include "serialization/protobuf/Token.pb.h"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
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
class Purse;
}  // namespace blind
}  // namespace otx

namespace proto
{
class Ciphertext;
class LucreTokenData;
class Token;
}  // namespace proto

class PasswordPrompt;
// }  // namespace v1
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
    auto Serialize(proto::Token& out) const noexcept -> bool final;

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
        const proto::Token& serialized);
    Lucre(
        const api::Session& api,
        const identity::Nym& owner,
        const blind::Mint& mint,
        const Denomination value,
        blind::internal::Purse& purse,
        const PasswordPrompt& reason);
    Lucre(const Lucre& rhs, blind::internal::Purse& newOwner);

    ~Lucre() final = default;

private:
    const VersionNumber lucre_version_;
    OTString signature_;
    std::shared_ptr<proto::Ciphertext> private_;
    std::shared_ptr<proto::Ciphertext> public_;
    std::shared_ptr<proto::Ciphertext> spend_;

    void serialize_private(proto::LucreTokenData& lucre) const;
    void serialize_public(proto::LucreTokenData& lucre) const;
    void serialize_signature(proto::LucreTokenData& lucre) const;
    void serialize_spendable(proto::LucreTokenData& lucre) const;

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
        const std::shared_ptr<proto::Ciphertext> publicKey,
        const std::shared_ptr<proto::Ciphertext> privateKey,
        const std::shared_ptr<proto::Ciphertext> spendable);
    Lucre() = delete;
    Lucre(const Lucre&);
    Lucre(Lucre&&) = delete;
    auto operator=(const Lucre&) -> Lucre& = delete;
    auto operator=(Lucre&&) -> Lucre& = delete;
};
}  // namespace opentxs::otx::blind::token
