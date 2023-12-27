// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::proto::CredentialRole

#pragma once

#include <Enums.pb.h>
#include <cstdint>
#include <memory>
#include <span>
#include <string_view>

#include "opentxs/Time.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/contract/Types.internal.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/identifier/Generic.hpp"
#include "opentxs/identity/Types.hpp"
#include "opentxs/identity/wot/Types.hpp"
#include "opentxs/identity/wot/verification/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace session
{
class Client;
}  // namespace session

class Crypto;
class Session;
}  // namespace api

namespace contract
{
namespace unit
{
class Basket;
class Currency;
class Security;
}  // namespace unit

class Server;
class Unit;
}  // namespace contract

namespace crypto
{
class Bip39;
class Bitcoin;
class Envelope;
class Parameters;
}  // namespace crypto

namespace display
{
class Definition;
}  // namespace display

namespace identity
{
namespace credential
{
namespace internal
{
struct Contact;
struct Primary;
struct Secondary;
struct Verification;
}  // namespace internal
}  // namespace credential

namespace internal
{
class Authority;
class Nym;
}  // namespace internal

namespace wot
{
namespace verification
{
namespace internal
{
struct Group;
struct Item;
struct Nym;
struct Set;
}  // namespace internal
}  // namespace verification
}  // namespace wot
}  // namespace identity

namespace internal
{
struct NymFile;
}  // namespace internal

namespace otx
{
namespace client
{
namespace internal
{
struct Operation;
}  // namespace internal
}  // namespace client
}  // namespace otx

namespace identifier
{
class Notary;
class Nym;
}  // namespace identifier

namespace identity
{
class Nym;
class Source;
}  // namespace identity

namespace proto
{
class Authority;
class Credential;
class Envelope;
class Nym;
class NymIDSource;
class ServerContract;
class UnitDefinition;
class VerificationGroup;
class VerificationIdentity;
class VerificationItem;
class VerificationSet;
}  // namespace proto

class Armored;
class Data;
class PasswordCallback;
class PasswordPrompt;
class String;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs
{
class Factory
{
public:
    static auto Armored(const api::Crypto& crypto) -> opentxs::Armored*;
    static auto Armored(const api::Crypto& crypto, const opentxs::Data& input)
        -> opentxs::Armored*;
    static auto Armored(const api::Crypto& crypto, const opentxs::String& input)
        -> opentxs::Armored*;
    static auto Armored(
        const api::Crypto& crypto,
        const opentxs::crypto::Envelope& input) -> opentxs::Armored*;
    static auto Authority(
        const api::Session& api,
        const identity::Nym& parent,
        const identity::Source& source,
        const proto::KeyMode mode,
        const proto::Authority& serialized) -> identity::internal::Authority*;
    static auto Authority(
        const api::Session& api,
        const identity::Nym& parent,
        const identity::Source& source,
        const crypto::Parameters& nymParameters,
        const VersionNumber nymVersion,
        const opentxs::PasswordPrompt& reason)
        -> identity::internal::Authority*;
    static auto BasketContract(
        const api::Session& api,
        const Nym_p& nym,
        const UnallocatedCString& shortname,
        const UnallocatedCString& terms,
        const std::uint64_t weight,
        const UnitType unitOfAccount,
        const VersionNumber version,
        const display::Definition& displayDefinition,
        const Amount& redemptionIncrement) noexcept
        -> std::shared_ptr<contract::unit::Basket>;
    static auto BasketContract(
        const api::Session& api,
        const Nym_p& nym,
        const proto::UnitDefinition serialized) noexcept
        -> std::shared_ptr<contract::unit::Basket>;
    static auto Bitcoin(const api::Crypto& crypto) -> crypto::Bitcoin*;
    static auto Bip39(const api::Crypto& api) noexcept
        -> std::unique_ptr<crypto::Bip39>;
    static auto ContactCredential(
        const api::Session& api,
        identity::internal::Authority& parent,
        const identity::Source& source,
        const identity::credential::internal::Primary& master,
        const crypto::Parameters& nymParameters,
        const VersionNumber version,
        const opentxs::PasswordPrompt& reason)
        -> identity::credential::internal::Contact*;
    static auto ContactCredential(
        const api::Session& api,
        identity::internal::Authority& parent,
        const identity::Source& source,
        const identity::credential::internal::Primary& master,
        const proto::Credential& credential)
        -> identity::credential::internal::Contact*;
    template <class C>
    static auto Credential(
        const api::Session& api,
        identity::internal::Authority& parent,
        const identity::Source& source,
        const identity::credential::internal::Primary& master,
        const VersionNumber version,
        const crypto::Parameters& parameters,
        const proto::CredentialRole role,
        const opentxs::PasswordPrompt& reason) -> C*;
    template <class C>
    static auto Credential(
        const api::Session& api,
        identity::internal::Authority& parent,
        const identity::Source& source,
        const identity::credential::internal::Primary& master,
        const proto::Credential& serialized,
        const proto::KeyMode mode,
        const proto::CredentialRole role) -> C*;
    static auto CurrencyContract(
        const api::Session& api,
        const Nym_p& nym,
        const UnallocatedCString& shortname,
        const UnallocatedCString& terms,
        const UnitType unitOfAccount,
        const VersionNumber version,
        const opentxs::PasswordPrompt& reason,
        const display::Definition& displayDefinition,
        const Amount& redemptionIncrement) noexcept
        -> std::shared_ptr<contract::unit::Currency>;
    static auto CurrencyContract(
        const api::Session& api,
        const Nym_p& nym,
        const proto::UnitDefinition serialized) noexcept
        -> std::shared_ptr<contract::unit::Currency>;
    static auto Envelope(const api::Session& api) noexcept
        -> std::unique_ptr<crypto::Envelope>;
    static auto Envelope(
        const api::Session& api,
        const proto::Envelope& serialized) noexcept(false)
        -> std::unique_ptr<crypto::Envelope>;
    static auto Envelope(
        const api::Session& api,
        const ReadView& serialized) noexcept(false)
        -> std::unique_ptr<crypto::Envelope>;
    static auto NullCallback() -> PasswordCallback*;
    static auto Nym(
        const api::Session& api,
        const crypto::Parameters& nymParameters,
        const identity::Type type,
        const UnallocatedCString name,
        const opentxs::PasswordPrompt& reason) -> identity::internal::Nym*;
    static auto Nym(
        const api::Session& api,
        const proto::Nym& serialized,
        std::string_view alias) -> identity::internal::Nym*;
    static auto Nym(
        const api::Session& api,
        const ReadView& serialized,
        std::string_view alias) -> identity::internal::Nym*;
    static auto NymFile(
        const api::Session& api,
        Nym_p targetNym,
        Nym_p signerNym) -> internal::NymFile*;
    static auto NymIDSource(
        const api::Session& api,
        crypto::Parameters& parameters,
        const opentxs::PasswordPrompt& reason) -> identity::Source*;
    static auto NymIDSource(
        const api::Session& api,
        const proto::NymIDSource& serialized) -> identity::Source*;
    static auto Operation(
        const api::session::Client& api,
        const identifier::Nym& nym,
        const identifier::Notary& server,
        const opentxs::PasswordPrompt& reason)
        -> otx::client::internal::Operation*;
    static auto PrimaryCredential(
        const api::Session& api,
        identity::internal::Authority& parent,
        const identity::Source& source,
        const crypto::Parameters& nymParameters,
        const VersionNumber version,
        const opentxs::PasswordPrompt& reason)
        -> identity::credential::internal::Primary*;
    static auto PrimaryCredential(
        const api::Session& api,
        identity::internal::Authority& parent,
        const identity::Source& source,
        const proto::Credential& credential)
        -> identity::credential::internal::Primary*;
    static auto SecondaryCredential(
        const api::Session& api,
        identity::internal::Authority& parent,
        const identity::Source& source,
        const identity::credential::internal::Primary& master,
        const crypto::Parameters& nymParameters,
        const VersionNumber version,
        const opentxs::PasswordPrompt& reason)
        -> identity::credential::internal::Secondary*;
    static auto SecondaryCredential(
        const api::Session& api,
        identity::internal::Authority& parent,
        const identity::Source& source,
        const identity::credential::internal::Primary& master,
        const proto::Credential& credential)
        -> identity::credential::internal::Secondary*;
    static auto SecurityContract(
        const api::Session& api,
        const Nym_p& nym,
        const UnallocatedCString& shortname,
        const UnallocatedCString& terms,
        const UnitType unitOfAccount,
        const VersionNumber version,
        const opentxs::PasswordPrompt& reason,
        const display::Definition& displayDefinition,
        const Amount& redemptionIncrement) noexcept
        -> std::shared_ptr<contract::unit::Security>;
    static auto SecurityContract(
        const api::Session& api,
        const Nym_p& nym,
        const proto::UnitDefinition serialized) noexcept
        -> std::shared_ptr<contract::unit::Security>;
    static auto ServerContract(const api::Session& api) noexcept
        -> std::unique_ptr<contract::Server>;
    static auto ServerContract(
        const api::Session& api,
        const Nym_p& nym,
        const UnallocatedList<Endpoint>& endpoints,
        const UnallocatedCString& terms,
        const UnallocatedCString& name,
        const VersionNumber version,
        const opentxs::PasswordPrompt& reason) noexcept
        -> std::unique_ptr<contract::Server>;
    static auto ServerContract(
        const api::Session& api,
        const Nym_p& nym,
        const proto::ServerContract& serialized) noexcept
        -> std::unique_ptr<contract::Server>;
    static auto UnitDefinition(const api::Session& api) noexcept
        -> std::shared_ptr<contract::Unit>;
    static auto UnitDefinition(
        const api::Session& api,
        const Nym_p& nym,
        const proto::UnitDefinition serialized) noexcept
        -> std::shared_ptr<contract::Unit>;
    static auto VerificationCredential(
        const api::Session& api,
        identity::internal::Authority& parent,
        const identity::Source& source,
        const identity::credential::internal::Primary& master,
        const crypto::Parameters& nymParameters,
        const VersionNumber version,
        const opentxs::PasswordPrompt& reason)
        -> identity::credential::internal::Verification*;
    static auto VerificationCredential(
        const api::Session& api,
        identity::internal::Authority& parent,
        const identity::Source& source,
        const identity::credential::internal::Primary& master,
        const proto::Credential& credential)
        -> identity::credential::internal::Verification*;
    static auto VerificationGroup(
        identity::wot::verification::internal::Set& parent,
        const VersionNumber version,
        bool external) -> identity::wot::verification::internal::Group*;
    static auto VerificationGroup(
        identity::wot::verification::internal::Set& parent,
        const proto::VerificationGroup& serialized,
        bool external) -> identity::wot::verification::internal::Group*;
    static auto VerificationItem(
        const identity::wot::verification::internal::Nym& parent,
        const identifier::Generic& claim,
        const identity::Nym& signer,
        const opentxs::PasswordPrompt& reason,
        identity::wot::verification::Type value,
        Time start,
        Time end,
        VersionNumber version,
        std::span<const identity::wot::VerificationID> superscedes)
        -> identity::wot::verification::internal::Item*;
    static auto VerificationItem(
        const identity::wot::verification::internal::Nym& parent,
        const proto::VerificationItem& serialized)
        -> identity::wot::verification::internal::Item*;
    static auto VerificationNym(
        identity::wot::verification::internal::Group& parent,
        const identifier::Nym& nym,
        const VersionNumber version)
        -> identity::wot::verification::internal::Nym*;
    static auto VerificationNym(
        identity::wot::verification::internal::Group& parent,
        const proto::VerificationIdentity& serialized)
        -> identity::wot::verification::internal::Nym*;
    static auto VerificationSet(
        const api::Session& api,
        const identifier::Nym& nym,
        const VersionNumber version)
        -> identity::wot::verification::internal::Set*;
    static auto VerificationSet(
        const api::Session& api,
        const identifier::Nym& nym,
        const proto::VerificationSet& serialized)
        -> identity::wot::verification::internal::Set*;
};
}  // namespace opentxs
