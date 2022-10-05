// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <iosfwd>
#include <memory>
#include <optional>

#include "internal/otx/blind/Purse.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/Secret.hpp"
#include "opentxs/core/identifier/Notary.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/crypto/Envelope.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/crypto/key/Symmetric.hpp"
#include "opentxs/crypto/key/symmetric/Algorithm.hpp"
#include "opentxs/otx/blind/CashType.hpp"
#include "opentxs/otx/blind/Purse.hpp"
#include "opentxs/otx/blind/PurseType.hpp"
#include "opentxs/otx/blind/Token.hpp"
#include "opentxs/otx/blind/Types.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"
#include "opentxs/util/Time.hpp"

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
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::otx::blind
{
class Purse::Imp : virtual public internal::Purse
{
public:
    blind::Purse* parent_;

    virtual auto at(const std::size_t) const -> const Token&;
    virtual auto cbegin() const noexcept -> const_iterator { return {}; }
    virtual auto cend() const noexcept -> const_iterator { return {}; }
    virtual auto clone() const noexcept -> Imp*
    {
        return std::make_unique<Imp>(*this).release();
    }
    virtual auto EarliestValidTo() const -> Time { return {}; }
    virtual auto IsUnlocked() const -> bool { return {}; }
    virtual auto IsValid() const noexcept -> bool { return false; }
    virtual auto LatestValidFrom() const -> Time { return {}; }
    auto Notary() const -> const identifier::Notary& override;
    auto Process(const identity::Nym&, const Mint&, const PasswordPrompt&)
        -> bool override
    {
        return {};
    }
    auto Serialize(proto::Purse&) const noexcept -> bool override { return {}; }
    virtual auto Serialize(AllocateOutput) const noexcept -> bool { return {}; }
    virtual auto size() const noexcept -> std::size_t { return {}; }
    virtual auto State() const -> blind::PurseType { return {}; }
    auto Type() const -> blind::CashType override { return {}; }
    auto Unit() const -> const identifier::UnitDefinition& override;
    virtual auto Unlock(const identity::Nym&, const PasswordPrompt&) const
        -> bool
    {
        return {};
    }
    virtual auto Verify(const api::session::Notary&) const -> bool
    {
        return {};
    }
    virtual auto Value() const -> const Amount&;

    virtual auto AddNym(const identity::Nym&, const PasswordPrompt&) -> bool
    {
        return {};
    }
    virtual auto at(const std::size_t) -> Token&;
    virtual auto begin() noexcept -> iterator { return {}; }
    virtual auto end() noexcept -> iterator { return {}; }
    virtual auto GeneratePrototokens(
        const identity::Nym&,
        const Mint&,
        const Amount&,
        const PasswordPrompt&) -> bool
    {
        return {};
    }
    auto PrimaryKey(PasswordPrompt&) -> crypto::key::Symmetric& override;
    virtual auto Pop() -> Token { return {}; }
    virtual auto Push(Token&&, const PasswordPrompt&) -> bool { return {}; }
    auto SecondaryKey(const identity::Nym&, PasswordPrompt&)
        -> const crypto::key::Symmetric& override;

    Imp() noexcept
        : parent_(nullptr)
    {
    }
    Imp(const Imp& rhs) noexcept
        : parent_(rhs.parent_)
    {
    }
    Imp(Imp&&) = delete;
    auto operator=(const Imp&) -> Imp& = delete;
    auto operator=(Imp&& rhs) -> Imp& = delete;

    ~Imp() override = default;
};
}  // namespace opentxs::otx::blind
