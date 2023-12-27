// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/crypto/symmetric/Key.hpp"

#pragma once

#include <Envelope.pb.h>
#include <cstddef>
#include <memory>
#include <optional>

#include "internal/crypto/Envelope.hpp"
#include "opentxs/Time.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/Secret.hpp"
#include "opentxs/crypto/symmetric/Types.hpp"
#include "opentxs/identifier/Notary.hpp"
#include "opentxs/identifier/UnitDefinition.hpp"
#include "opentxs/otx/blind/Purse.hpp"
#include "opentxs/otx/blind/Token.hpp"
#include "opentxs/otx/blind/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"
#include "otx/blind/purse/Purse.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace session
{
class Notary;
}  // namespace session

class Session;
}  // namespace api

namespace identifier
{
class Nym;
}  // namespace identifier

namespace identity
{
class Nym;
}  // namespace identity

namespace otx
{
namespace blind
{
class Mint;
}  // namespace blind
}  // namespace otx

namespace proto
{
class Purse;
}  // namespace proto

class PasswordPrompt;
class Writer;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::otx::blind::purse
{
class Purse final : public blind::Purse::Imp
{
public:
    using iterator = blind::Purse::iterator;
    using const_iterator = blind::Purse::const_iterator;

    auto at(const std::size_t position) const -> const Token& final
    {
        return tokens_.at(position);
    }
    auto cbegin() const noexcept -> const_iterator final;
    auto cend() const noexcept -> const_iterator final;
    auto clone() const noexcept -> Imp* final
    {
        return std::make_unique<Purse>(*this).release();
    }
    auto EarliestValidTo() const -> Time final { return earliest_valid_to_; }
    auto IsUnlocked() const -> bool final { return unlocked_; }
    auto IsValid() const noexcept -> bool final { return true; }
    auto LatestValidFrom() const -> Time final { return latest_valid_from_; }
    auto Notary() const -> const identifier::Notary& final { return notary_; }
    auto Process(
        const identity::Nym& owner,
        const Mint& mint,
        const PasswordPrompt& reason) -> bool final;
    auto Serialize(proto::Purse& out) const noexcept -> bool final;
    auto Serialize(Writer&& destination) const noexcept -> bool final;
    auto size() const noexcept -> std::size_t final { return tokens_.size(); }
    auto State() const -> blind::PurseType final { return state_; }
    auto Type() const -> blind::CashType final { return type_; }
    auto Unit() const -> const identifier::UnitDefinition& final
    {
        return unit_;
    }
    auto Unlock(const identity::Nym& nym, const PasswordPrompt& reason) const
        -> bool final;
    auto Verify(const api::session::Notary& server) const -> bool final;
    auto Value() const -> const Amount& final { return total_value_; }

    auto AddNym(const identity::Nym& nym, const PasswordPrompt& reason)
        -> bool final;
    auto at(const std::size_t position) -> Token& final
    {
        return tokens_.at(position);
    }
    auto begin() noexcept -> iterator final;
    auto DeserializeTokens(const proto::Purse& in) noexcept -> void;
    auto end() noexcept -> iterator final;
    auto GeneratePrototokens(
        const identity::Nym& owner,
        const Mint& mint,
        const Amount& amount,
        const PasswordPrompt& reason) -> bool final;
    auto PrimaryKey(PasswordPrompt& password) -> crypto::symmetric::Key& final;
    auto Pop() -> Token final;
    auto Push(Token&& token, const PasswordPrompt& reason) -> bool final;
    auto SecondaryKey(const identity::Nym& owner, PasswordPrompt& password)
        -> const crypto::symmetric::Key& final;

    Purse(
        const api::Session& api,
        const identifier::Nym& owner,
        const identifier::Notary& server,
        const blind::CashType type,
        const Mint& mint,
        Secret&& secondaryKeyPassword,
        std::unique_ptr<const crypto::symmetric::Key> secondaryKey,
        std::unique_ptr<const OTEnvelope> secondaryEncrypted) noexcept;
    Purse(
        const api::Session& api,
        const identifier::Notary& server,
        const identifier::UnitDefinition& unit,
        const blind::CashType type) noexcept;
    Purse(
        const api::Session& api,
        const VersionNumber version,
        const blind::CashType type,
        const identifier::Notary& notary,
        const identifier::UnitDefinition& unit,
        const blind::PurseType state,
        const Amount& totalValue,
        const Time validFrom,
        const Time validTo,
        const UnallocatedVector<blind::Token>& tokens,
        const std::shared_ptr<crypto::symmetric::Key> primary,
        const UnallocatedVector<proto::Envelope>& primaryPasswords,
        const std::shared_ptr<const crypto::symmetric::Key> secondaryKey,
        const std::shared_ptr<const OTEnvelope> secondaryEncrypted,
        std::optional<Secret> secondaryKeyPassword) noexcept;
    Purse(const api::Session& api, const Purse& owner) noexcept;
    Purse(const api::Session& api, const proto::Purse& serialized) noexcept;
    Purse() = delete;
    Purse(const Purse&) noexcept;
    Purse(Purse&&) = delete;
    auto operator=(const Purse&) -> Purse& = delete;
    auto operator=(Purse&&) -> Purse& = delete;

    ~Purse() final;

private:
    static const opentxs::crypto::symmetric::Algorithm mode_;

    const api::Session& api_;
    const VersionNumber version_;
    const blind::CashType type_;
    const identifier::Notary notary_;
    const identifier::UnitDefinition unit_;
    blind::PurseType state_;
    Amount total_value_;
    Time latest_valid_from_;
    Time earliest_valid_to_;
    UnallocatedVector<blind::Token> tokens_;
    mutable bool unlocked_;
    mutable Secret primary_key_password_;
    std::shared_ptr<crypto::symmetric::Key> primary_;
    UnallocatedVector<proto::Envelope> primary_passwords_;
    Secret secondary_key_password_;
    const std::shared_ptr<const crypto::symmetric::Key> secondary_;
    const std::shared_ptr<const OTEnvelope> secondary_password_;

    static auto deserialize_secondary_key(
        const api::Session& api,
        const proto::Purse& serialized) noexcept(false)
        -> std::unique_ptr<const crypto::symmetric::Key>;
    static auto deserialize_secondary_password(
        const api::Session& api,
        const proto::Purse& serialized) noexcept(false)
        -> std::unique_ptr<const OTEnvelope>;
    static auto get_passwords(const proto::Purse& in)
        -> UnallocatedVector<proto::Envelope>;

    auto generate_key(Secret& password) const -> crypto::symmetric::Key;

    auto apply_times(const Token& token) -> void;
    auto recalculate_times() -> void;
};
}  // namespace opentxs::otx::blind::purse
