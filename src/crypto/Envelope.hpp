// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/crypto/asymmetric/Algorithm.hpp"
// IWYU pragma: no_include "opentxs/crypto/asymmetric/Key.hpp"

#pragma once

#include <Ciphertext.pb.h>
#include <Enums.pb.h>
#include <algorithm>
#include <cstdint>
#include <memory>
#include <tuple>

#include "internal/crypto/Envelope.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "opentxs/crypto/asymmetric/Types.hpp"
#include "opentxs/identity/Nym.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"
#include "opentxs/util/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

namespace crypto
{
namespace symmetric
{
class Key;
}  // namespace symmetric
}  // namespace crypto

namespace identifier
{
class Generic;
class Nym;
}  // namespace identifier

namespace identity
{
class Authority;
}  // namespace identity

namespace proto
{
class Ciphertext;
}  // namespace proto

class Armored;
class PasswordPrompt;
class Writer;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::crypto::implementation
{
class Envelope final : public crypto::Envelope
{
public:
    auto Armored(opentxs::Armored& ciphertext) const noexcept -> bool final;
    auto Open(
        const identity::Nym& recipient,
        Writer&& plaintext,
        const PasswordPrompt& reason) const noexcept -> bool final;
    auto Serialize(Writer&& destination) const noexcept -> bool final;
    auto Serialize(SerializedType& serialized) const noexcept -> bool final;

    auto Seal(
        const Recipients& recipients,
        const ReadView plaintext,
        const PasswordPrompt& reason) noexcept -> bool final;
    auto Seal(
        const identity::Nym& theRecipient,
        const ReadView plaintext,
        const PasswordPrompt& reason) noexcept -> bool final;

    Envelope(const api::Session& api) noexcept;
    Envelope(
        const api::Session& api,
        const SerializedType& serialized) noexcept(false);
    Envelope(const api::Session& api, const ReadView& serialized) noexcept(
        false);
    Envelope() = delete;
    Envelope(const Envelope&) noexcept;
    Envelope(Envelope&&) = delete;
    auto operator=(const Envelope&) -> Envelope& = delete;
    auto operator=(Envelope&&) -> Envelope& = delete;

    ~Envelope() final;

private:
    friend OTEnvelope;

    using Ciphertext = std::unique_ptr<proto::Ciphertext>;
    using DHMap = std::map<
        crypto::asymmetric::Algorithm,
        UnallocatedVector<crypto::asymmetric::Key>>;
    using Nyms = UnallocatedVector<const identity::Nym*>;
    using Tag = std::uint32_t;
    using SessionKey =
        std::tuple<Tag, crypto::asymmetric::Algorithm, symmetric::Key>;
    using SessionKeys = UnallocatedVector<SessionKey>;
    using SupportedKeys = UnallocatedVector<crypto::asymmetric::Algorithm>;
    using Weight = unsigned int;
    using WeightMap = UnallocatedMap<crypto::asymmetric::Algorithm, Weight>;
    using Solution = UnallocatedMap<
        identifier::Nym,
        UnallocatedMap<identifier::Generic, crypto::asymmetric::Algorithm>>;
    using Solutions = UnallocatedMap<Weight, SupportedKeys>;
    using Requirements = UnallocatedVector<identity::Nym::NymKeys>;

    static const VersionNumber default_version_;
    static const VersionNumber tagged_key_version_;
    static const SupportedKeys supported_;
    static const WeightMap key_weights_;
    static const Solutions solutions_;

    const api::Session& api_;
    const VersionNumber version_;
    DHMap dh_keys_;
    SessionKeys session_keys_;
    Ciphertext ciphertext_;

    static auto calculate_requirements(const Nyms& recipients) noexcept(false)
        -> Requirements;
    static auto calculate_solutions() noexcept -> Solutions;
    static auto clone(const Ciphertext& rhs) noexcept -> Ciphertext;
    static auto clone(const DHMap& rhs) noexcept -> DHMap;
    static auto clone(const SessionKeys& rhs) noexcept -> SessionKeys;
    static auto find_solution(const Nyms& recipients, Solution& map) noexcept
        -> SupportedKeys;
    static auto read_dh(
        const api::Session& api,
        const SerializedType& rhs) noexcept -> DHMap;
    static auto read_sk(
        const api::Session& api,
        const SerializedType& rhs) noexcept -> SessionKeys;
    static auto read_ct(const SerializedType& rhs) noexcept -> Ciphertext;
    static auto set_default_password(
        const api::Session& api,
        PasswordPrompt& password) noexcept -> bool;
    static auto test_solution(
        const SupportedKeys& solution,
        const Requirements& requirements,
        Solution& map) noexcept -> bool;

    auto clone() const noexcept -> Envelope* final
    {
        return new Envelope{*this};
    }
    auto unlock_session_key(
        const identity::Nym& recipient,
        PasswordPrompt& reason) const noexcept(false) -> const symmetric::Key&;

    auto attach_session_keys(
        const identity::Nym& nym,
        const Solution& solution,
        const PasswordPrompt& previousPassword,
        const symmetric::Key& masterKey,
        const PasswordPrompt& reason) noexcept -> bool;
    auto get_dh_key(
        const crypto::asymmetric::Algorithm type,
        const identity::Authority& nym,
        const PasswordPrompt& reason) noexcept -> const asymmetric::Key&;
    auto seal(
        const Nyms recipients,
        const ReadView plaintext,
        const PasswordPrompt& reason) noexcept -> bool;
};
}  // namespace opentxs::crypto::implementation
