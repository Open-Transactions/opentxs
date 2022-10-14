// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"         // IWYU pragma: associated
#include "crypto/Envelope.hpp"  // IWYU pragma: associated

#include <AsymmetricKey.pb.h>
#include <Ciphertext.pb.h>
#include <Envelope.pb.h>
#include <TaggedKey.pb.h>
#include <algorithm>
#include <cstddef>
#include <iterator>
#include <numeric>
#include <stdexcept>
#include <type_traits>
#include <utility>

#include "2_Factory.hpp"
#include "internal/api/crypto/Symmetric.hpp"
#include "internal/api/session/FactoryAPI.hpp"
#include "internal/core/Armored.hpp"
#include "internal/crypto/Envelope.hpp"
#include "internal/crypto/asymmetric/Key.hpp"
#include "internal/crypto/key/Key.hpp"
#include "internal/crypto/symmetric/Key.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "internal/serialization/protobuf/Proto.tpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "internal/util/PasswordPrompt.hpp"
#include "opentxs/api/crypto/Config.hpp"
#include "opentxs/api/crypto/Symmetric.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/identifier/Generic.hpp"  // IWYU pragma: keep
#include "opentxs/core/identifier/Nym.hpp"      // IWYU pragma: keep
#include "opentxs/crypto/Parameters.hpp"
#include "opentxs/crypto/asymmetric/Algorithm.hpp"
#include "opentxs/crypto/asymmetric/Key.hpp"
#include "opentxs/crypto/asymmetric/Role.hpp"
#include "opentxs/crypto/symmetric/Algorithm.hpp"
#include "opentxs/crypto/symmetric/Key.hpp"
#include "opentxs/identity/Authority.hpp"
#include "opentxs/identity/Nym.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Iterator.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/PasswordPrompt.hpp"
#include "opentxs/util/Writer.hpp"

namespace opentxs
{
auto Factory::Envelope(const api::Session& api) noexcept
    -> std::unique_ptr<crypto::Envelope>
{
    using ReturnType = crypto::implementation::Envelope;

    return std::make_unique<ReturnType>(api);
}

auto Factory::Envelope(
    const api::Session& api,
    const proto::Envelope& serialized) noexcept(false)
    -> std::unique_ptr<crypto::Envelope>
{
    using ReturnType = crypto::implementation::Envelope;

    return std::make_unique<ReturnType>(api, serialized);
}

auto Factory::Envelope(
    const api::Session& api,
    const ReadView& serialized) noexcept(false)
    -> std::unique_ptr<crypto::Envelope>
{
    using ReturnType = crypto::implementation::Envelope;

    return std::make_unique<ReturnType>(api, serialized);
}
}  // namespace opentxs

namespace opentxs::crypto::implementation
{
const VersionNumber Envelope::default_version_{2};
const VersionNumber Envelope::tagged_key_version_{1};
// NOTE: elements in supported_ must be added in sorted order or else
// test_solution() will not produce the correct result
const Envelope::SupportedKeys Envelope::supported_{[] {
    auto out = SupportedKeys{};
    using Type = crypto::asymmetric::Algorithm;

    if (api::crypto::HaveSupport(Type::Legacy)) {
        out.emplace_back(Type::Legacy);
    }

    if (api::crypto::HaveSupport(Type::Secp256k1)) {
        out.emplace_back(Type::Secp256k1);
    }

    if (api::crypto::HaveSupport(Type::ED25519)) {
        out.emplace_back(Type::ED25519);
    }

    std::sort(out.begin(), out.end());

    return out;
}()};
const Envelope::WeightMap Envelope::key_weights_{
    {crypto::asymmetric::Algorithm::ED25519, 1},
    {crypto::asymmetric::Algorithm::Secp256k1, 2},
    {crypto::asymmetric::Algorithm::Legacy, 4},
};
const Envelope::Solutions Envelope::solutions_{calculate_solutions()};

Envelope::Envelope(const api::Session& api) noexcept
    : api_(api)
    , version_(default_version_)
    , dh_keys_()
    , session_keys_()
    , ciphertext_()
{
}

Envelope::Envelope(const api::Session& api, const SerializedType& in) noexcept(
    false)
    : api_(api)
    , version_(in.version())
    , dh_keys_(read_dh(api_, in))
    , session_keys_(read_sk(api_, in))
    , ciphertext_(read_ct(in))
{
}

Envelope::Envelope(const api::Session& api, const ReadView& in) noexcept(false)
    : Envelope(api, proto::Factory<proto::Envelope>(in))
{
}

Envelope::Envelope(const Envelope& rhs) noexcept
    : api_(rhs.api_)
    , version_(rhs.version_)
    , dh_keys_(clone(rhs.dh_keys_))
    , session_keys_(clone(rhs.session_keys_))
    , ciphertext_(clone(rhs.ciphertext_))
{
}

auto Envelope::Armored(opentxs::Armored& ciphertext) const noexcept -> bool
{
    auto serialized = proto::Envelope{};
    if (false == Serialize(serialized)) { return false; }

    return ciphertext.SetData(
        api_.Factory().InternalSession().Data(serialized));
}

auto Envelope::attach_session_keys(
    const identity::Nym& nym,
    const Solution& solution,
    const PasswordPrompt& previousPassword,
    const symmetric::Key& masterKey,
    const PasswordPrompt& reason) noexcept -> bool
{
    LogVerbose()(OT_PRETTY_CLASS())("Recipient ")(nym.ID())(" has ")(
        nym.size())(" master credentials")
        .Flush();

    for (const auto& authority : nym) {
        const auto type = solution.at(nym.ID()).at(authority.GetMasterCredID());
        auto tag = Tag{};
        auto password = api_.Factory().Secret(0);
        const auto& dhKey = get_dh_key(type, authority, reason);
        const auto haveTag = dhKey.Internal().CalculateTag(
            authority, type, reason, tag, password);

        if (false == haveTag) {
            LogError()(OT_PRETTY_CLASS())(
                "Failed to calculate session password")
                .Flush();

            return false;
        }

        auto& key = std::get<2>(
            session_keys_.emplace_back(tag, type, symmetric::Key(masterKey)));
        const auto locked = key.ChangePassword(password, previousPassword);

        if (false == locked) {
            LogError()(OT_PRETTY_CLASS())("Failed to lock session key").Flush();

            return false;
        }
    }

    return true;
}

auto Envelope::calculate_requirements(const Nyms& recipients) noexcept(false)
    -> Requirements
{
    auto output = Requirements{};

    for (const auto& nym : recipients) {
        const auto& targets = output.emplace_back(nym->EncryptionTargets());

        if (targets.second.empty()) {
            LogError()(OT_PRETTY_STATIC(Envelope))("Invalid recipient nym ")(
                nym->ID())
                .Flush();

            throw std::runtime_error("Invalid recipient nym");
        }
    }

    return output;
}

auto Envelope::calculate_solutions() noexcept -> Solutions
{
    constexpr auto one = 1_uz;
    auto output = Solutions{};

    for (auto row = one; row < (one << supported_.size()); ++row) {
        auto solution = std::pair<Weight, SupportedKeys>{};
        auto& [weight, keys] = solution;

        for (auto key = supported_.cbegin(); key != supported_.cend(); ++key) {
            const auto column = static_cast<std::size_t>(
                std::distance(supported_.cbegin(), key));

            if (0 != (row & (one << column))) { keys.emplace_back(*key); }
        }

        weight = std::accumulate(
            std::begin(keys),
            std::end(keys),
            0u,
            [](const auto& sum, const auto& key) -> Weight {
                return sum + key_weights_.at(key);
            });

        if (0 < weight) { output.emplace(std::move(solution)); }
    }

    return output;
}

auto Envelope::clone(const Ciphertext& rhs) noexcept -> Ciphertext
{
    if (rhs) { return std::make_unique<proto::Ciphertext>(*rhs); }

    return {};
}

auto Envelope::clone(const DHMap& rhs) noexcept -> DHMap
{
    auto output = DHMap{};

    for (const auto& [type, key] : rhs) { output.emplace(type, key); }

    return output;
}

auto Envelope::clone(const SessionKeys& rhs) noexcept -> SessionKeys
{
    auto output = SessionKeys{};

    for (const auto& [tag, type, key] : rhs) {
        output.emplace_back(tag, type, key);
    }

    return output;
}

auto Envelope::find_solution(const Nyms& recipients, Solution& map) noexcept
    -> SupportedKeys
{
    try {
        const auto requirements = calculate_requirements(recipients);

        for (const auto& [weight, keys] : solutions_) {
            if (test_solution(keys, requirements, map)) { return keys; }
        }
    } catch (...) {
    }

    return {};
}

auto Envelope::get_dh_key(
    const crypto::asymmetric::Algorithm type,
    const identity::Authority& nym,
    const PasswordPrompt& reason) noexcept -> const asymmetric::Key&
{
    if (crypto::asymmetric::Algorithm::Legacy != type) {
        const auto& set = dh_keys_.at(type);

        OT_ASSERT(1 == set.size());

        return *set.cbegin();
    } else {
        OT_ASSERT(
            api::crypto::HaveSupport(crypto::asymmetric::Algorithm::Legacy));

        auto params = Parameters{type};
        params.SetDHParams(nym.Params(type));
        auto& set = dh_keys_[type];
        set.emplace_back(api_.Factory().AsymmetricKey(
            crypto::asymmetric::Role::Encrypt, params, reason));
        const auto& key = *set.crbegin();

        OT_ASSERT(key.Type() == type);
        OT_ASSERT(key.Role() == crypto::asymmetric::Role::Encrypt);
        OT_ASSERT(0 < key.Internal().Params().size());

        return key;
    }
}

auto Envelope::Open(
    const identity::Nym& nym,
    Writer&& plaintext,
    const PasswordPrompt& reason) const noexcept -> bool
{
    if (false == bool(ciphertext_)) {
        LogError()(OT_PRETTY_CLASS())("Nothing to decrypt").Flush();

        return false;
    }

    const auto& ciphertext = *ciphertext_;

    try {
        auto password =
            api_.Factory().PasswordPrompt(reason.GetDisplayString());
        const auto& key = unlock_session_key(nym, password);

        return key.Internal().Decrypt(
            ciphertext, std::move(plaintext), password);
    } catch (...) {
        LogVerbose()(OT_PRETTY_CLASS())("No session keys for this nym").Flush();

        return false;
    }
}

auto Envelope::read_dh(
    const api::Session& api,
    const SerializedType& rhs) noexcept -> DHMap
{
    auto output = DHMap{};

    for (const auto& key : rhs.dhkey()) {
        auto& set = output[translate(key.type())];
        set.emplace_back(api.Factory().InternalSession().AsymmetricKey(key));
    }

    return output;
}

auto Envelope::read_sk(
    const api::Session& api,
    const SerializedType& rhs) noexcept -> SessionKeys
{
    auto output = SessionKeys{};

    for (const auto& tagged : rhs.sessionkey()) {
        output.emplace_back(SessionKey{
            tagged.tag(),
            translate(tagged.type()),
            api.Crypto().Symmetric().InternalSymmetric().Key(
                tagged.key(),
                opentxs::crypto::symmetric::Algorithm::ChaCha20Poly1305)});
    }

    return output;
}

auto Envelope::read_ct(const SerializedType& rhs) noexcept -> Ciphertext
{
    if (rhs.has_ciphertext()) {
        return std::make_unique<proto::Ciphertext>(rhs.ciphertext());
    }

    return {};
}

auto Envelope::Seal(
    const Recipients& recipients,
    const ReadView plaintext,
    const PasswordPrompt& reason) noexcept -> bool
{
    auto nyms = Nyms{};

    for (const auto& nym : recipients) {
        OT_ASSERT(nym);

        nyms.emplace_back(nym.get());
    }

    return seal(nyms, plaintext, reason);
}

auto Envelope::Seal(
    const identity::Nym& recipient,
    const ReadView plaintext,
    const PasswordPrompt& reason) noexcept -> bool
{
    return seal({&recipient}, plaintext, reason);
}

auto Envelope::seal(
    const Nyms recipients,
    const ReadView plaintext,
    const PasswordPrompt& reason) noexcept -> bool
{
    struct Cleanup {
        bool success_{false};

        Cleanup(Envelope& parent)
            : parent_(parent)
        {
        }

        ~Cleanup()
        {
            if (false == success_) {
                parent_.ciphertext_.reset();
                parent_.session_keys_.clear();
                parent_.dh_keys_.clear();
            }
        }

    private:
        Envelope& parent_;
    };

    if (ciphertext_) {
        LogError()(OT_PRETTY_CLASS())("Envelope has already been sealed")
            .Flush();

        return false;
    }

    if (0 == recipients.size()) {
        LogVerbose()(OT_PRETTY_CLASS())("No recipients").Flush();

        return false;
    } else {
        LogVerbose()(OT_PRETTY_CLASS())(recipients.size())(" recipient(s)")
            .Flush();
    }

    auto solution = Solution{};
    const auto dhkeys = find_solution(recipients, solution);

    if (0 == dhkeys.size()) {
        LogError()(OT_PRETTY_CLASS())(
            "A recipient requires an unsupported key type")
            .Flush();

        return false;
    } else {
        LogVerbose()(OT_PRETTY_CLASS())(dhkeys.size())(
            " dhkeys will be created")
            .Flush();
    }

    auto cleanup = Cleanup(*this);

    for (const auto& type : dhkeys) {
        try {
            const auto params = Parameters{type};

            if (crypto::asymmetric::Algorithm::Legacy != type) {
                auto& set = dh_keys_[type];
                set.emplace_back(api_.Factory().AsymmetricKey(
                    opentxs::crypto::asymmetric::Role::Encrypt,
                    params,
                    reason));
                const auto& key = *set.crbegin();

                OT_ASSERT(key.Type() == type);
                OT_ASSERT(
                    key.Role() == opentxs::crypto::asymmetric::Role::Encrypt);
            }
        } catch (...) {
            LogError()(OT_PRETTY_CLASS())("Failed to generate DH key").Flush();

            return false;
        }
    }

    auto password = api_.Factory().PasswordPrompt(reason);
    set_default_password(api_, password);
    auto masterKey = api_.Crypto().Symmetric().Key(password);
    ciphertext_ = std::make_unique<proto::Ciphertext>();

    OT_ASSERT(ciphertext_);

    const auto encrypted =
        masterKey.Internal().Encrypt(plaintext, *ciphertext_, password, false);

    if (false == encrypted) {
        LogError()(OT_PRETTY_CLASS())("Failed to encrypt plaintext").Flush();

        return false;
    }

    for (const auto& nym : recipients) {
        if (false ==
            attach_session_keys(*nym, solution, password, masterKey, reason)) {
            return false;
        }
    }

    cleanup.success_ = true;

    return cleanup.success_;
}

auto Envelope::set_default_password(
    const api::Session& api,
    PasswordPrompt& password) noexcept -> bool
{
    return password.Internal().SetPassword(
        api.Factory().SecretFromText("opentxs"));
}

auto Envelope::Serialize(Writer&& destination) const noexcept -> bool
{
    auto serialized = proto::Envelope{};
    if (false == Serialize(serialized)) { return false; }

    return write(serialized, std::move(destination));
}

auto Envelope::Serialize(SerializedType& output) const noexcept -> bool
{
    output.set_version(version_);

    for (const auto& [type, set] : dh_keys_) {
        for (const auto& key : set) {
            auto serialized = proto::AsymmetricKey{};
            if (false == key.asPublic().Internal().Serialize(serialized)) {
                return false;
            }
            *output.add_dhkey() = serialized;
        }
    }

    for (const auto& [tag, type, key] : session_keys_) {
        auto& tagged = *output.add_sessionkey();
        tagged.set_version(tagged_key_version_);
        tagged.set_tag(tag);
        tagged.set_type(translate(type));
        if (false == key.Internal().Serialize(*tagged.mutable_key())) {
            return false;
        }
    }

    if (ciphertext_) { *output.mutable_ciphertext() = *ciphertext_; }

    return true;
}

auto Envelope::test_solution(
    const SupportedKeys& solution,
    const Requirements& requirements,
    Solution& map) noexcept -> bool
{
    map.clear();

    for (const auto& [nymID, credentials] : requirements) {
        auto& row = map.emplace(nymID, Solution::mapped_type{}).first->second;

        for (const auto& [credID, keys] : credentials) {
            if (0 == keys.size()) { return false; }

            auto test = SupportedKeys{};
            std::set_intersection(
                std::begin(solution),
                std::end(solution),
                std::begin(keys),
                std::end(keys),
                std::back_inserter(test));

            if (test.size() != keys.size()) { return false; }

            row.emplace(credID, *keys.cbegin());
        }
    }

    return true;
}

auto Envelope::unlock_session_key(
    const identity::Nym& nym,
    PasswordPrompt& reason) const noexcept(false) -> const symmetric::Key&
{
    for (const auto& [tag, type, key] : session_keys_) {
        try {
            for (const auto& dhKey : dh_keys_.at(type)) {
                if (nym.Unlock(dhKey, tag, type, key, reason)) { return key; }
            }
        } catch (...) {
            throw std::runtime_error("Missing dhkey");
        }
    }

    throw std::runtime_error("No session key usable by this nym");
}

Envelope::~Envelope() = default;
}  // namespace opentxs::crypto::implementation
