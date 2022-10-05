// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"         // IWYU pragma: associated
#include "api/crypto/Seed.hpp"  // IWYU pragma: associated

#include <HDPath.pb.h>
#include <Seed.pb.h>
#include <iosfwd>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <type_traits>
#include <utility>

#include "internal/api/Crypto.hpp"
#include "internal/api/crypto/Asymmetric.hpp"
#include "internal/api/crypto/Factory.hpp"
#include "internal/crypto/Factory.hpp"
#include "internal/crypto/Seed.hpp"
#include "internal/crypto/key/Factory.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "internal/serialization/protobuf/Proto.tpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/OT.hpp"
#include "opentxs/api/Context.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/crypto/Asymmetric.hpp"
#include "opentxs/api/crypto/Hash.hpp"
#include "opentxs/api/crypto/Symmetric.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Endpoints.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/api/session/Storage.hpp"
#include "opentxs/blockchain/block/Hash.hpp"  // IWYU pragma: keep
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Secret.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/crypto/Bip32.hpp"
#include "opentxs/crypto/Bip32Child.hpp"
#include "opentxs/crypto/Bip39.hpp"
#include "opentxs/crypto/Bip43Purpose.hpp"
#include "opentxs/crypto/Bip44Type.hpp"
#include "opentxs/crypto/HashType.hpp"
#include "opentxs/crypto/Language.hpp"  // IWYU pragma: keep
#include "opentxs/crypto/Seed.hpp"
#include "opentxs/crypto/SeedStrength.hpp"  // IWYU pragma: keep
#include "opentxs/crypto/SeedStyle.hpp"     // IWYU pragma: keep
#include "opentxs/crypto/key/EllipticCurve.hpp"
#include "opentxs/crypto/key/HD.hpp"
#include "opentxs/crypto/key/Secp256k1.hpp"
#include "opentxs/crypto/key/Symmetric.hpp"
#include "opentxs/crypto/key/asymmetric/Algorithm.hpp"
#include "opentxs/crypto/key/asymmetric/Role.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "opentxs/util/Types.hpp"
#include "opentxs/util/WorkType.hpp"
#include "util/HDIndex.hpp"  // IWYU pragma: keep
#include "util/Work.hpp"

namespace opentxs::factory
{
auto SeedAPI(
    const api::Session& api,
    const api::session::Endpoints& endpoints,
    const api::session::Factory& factory,
    const api::crypto::Asymmetric& asymmetric,
    const api::crypto::Symmetric& symmetric,
    const api::session::Storage& storage,
    const crypto::Bip32& bip32,
    const crypto::Bip39& bip39,
    const network::zeromq::Context& zmq) noexcept
    -> std::unique_ptr<api::crypto::Seed>
{
    using ReturnType = api::crypto::imp::Seed;

    return std::make_unique<ReturnType>(
        api,
        endpoints,
        factory,
        asymmetric,
        symmetric,
        storage,
        bip32,
        bip39,
        zmq);
}
}  // namespace opentxs::factory

namespace opentxs::api::crypto::imp
{
Seed::Seed(
    const api::Session& api,
    const api::session::Endpoints& endpoints,
    const api::session::Factory& factory,
    const api::crypto::Asymmetric& asymmetric,
    const api::crypto::Symmetric& symmetric,
    const api::session::Storage& storage,
    const opentxs::crypto::Bip32& bip32,
    const opentxs::crypto::Bip39& bip39,
    const opentxs::network::zeromq::Context& zmq)
    : api_(api)  // WARNING do not access during construction
    , factory_(factory)
    , symmetric_(symmetric)
    , asymmetric_(asymmetric)
    , storage_(storage)
    , bip32_(bip32)
    , bip39_(bip39)
    , socket_([&] {
        auto out = zmq.PublishSocket();
        const auto rc = out->Start(endpoints.SeedUpdated().data());

        OT_ASSERT(rc);

        return out;
    }())
    , seed_lock_()
    , seeds_()
{
}

auto Seed::AccountChildKey(
    const proto::HDPath& rootPath,
    const BIP44Chain internal,
    const Bip32Index index,
    const PasswordPrompt& reason) const
    -> std::unique_ptr<opentxs::crypto::key::HD>
{
    auto parent = AccountKey(rootPath, internal, reason);

    if (!parent) { return {}; }

    return parent->ChildKey(index, reason);
}

auto Seed::AccountChildKey(
    const ReadView& view,
    const BIP44Chain internal,
    const Bip32Index index,
    const PasswordPrompt& reason) const
    -> std::unique_ptr<opentxs::crypto::key::HD>
{
    return AccountChildKey(
        proto::Factory<proto::HDPath>(view), internal, index, reason);
}

auto Seed::AccountKey(
    const proto::HDPath& rootPath,
    const BIP44Chain internal,
    const PasswordPrompt& reason) const
    -> std::unique_ptr<opentxs::crypto::key::HD>
{
    auto fingerprint{rootPath.root()};
    const auto change =
        (INTERNAL_CHAIN == internal) ? Bip32Index{1u} : Bip32Index{0u};
    auto path = UnallocatedVector<Bip32Index>{};

    for (const auto& child : rootPath.child()) { path.emplace_back(child); }

    path.emplace_back(change);

    return GetHDKey(
        fingerprint, opentxs::crypto::EcdsaCurve::secp256k1, path, reason);
}

auto Seed::AllowedSeedTypes() const noexcept
    -> const UnallocatedMap<opentxs::crypto::SeedStyle, UnallocatedCString>&
{
    static const auto map =
        UnallocatedMap<opentxs::crypto::SeedStyle, UnallocatedCString>{
            {opentxs::crypto::SeedStyle::BIP39, "BIP-39"},
            {opentxs::crypto::SeedStyle::PKT, "Legacy pktwallet"},
        };

    return map;
}

auto Seed::AllowedLanguages(
    const opentxs::crypto::SeedStyle type) const noexcept
    -> const UnallocatedMap<opentxs::crypto::Language, UnallocatedCString>&
{
    static const auto null =
        UnallocatedMap<opentxs::crypto::Language, UnallocatedCString>{};
    static const auto map = UnallocatedMap<
        opentxs::crypto::SeedStyle,
        UnallocatedMap<opentxs::crypto::Language, UnallocatedCString>>{
        {opentxs::crypto::SeedStyle::BIP39,
         {
             {opentxs::crypto::Language::en, "English"},
         }},
        {opentxs::crypto::SeedStyle::PKT,
         {
             {opentxs::crypto::Language::en, "English"},
         }},
    };

    try {
        return map.at(type);
    } catch (...) {

        return null;
    }
}

auto Seed::AllowedSeedStrength(
    const opentxs::crypto::SeedStyle type) const noexcept
    -> const UnallocatedMap<opentxs::crypto::SeedStrength, UnallocatedCString>&
{
    static const auto null =
        UnallocatedMap<opentxs::crypto::SeedStrength, UnallocatedCString>{};
    static const auto map = UnallocatedMap<
        opentxs::crypto::SeedStyle,
        UnallocatedMap<opentxs::crypto::SeedStrength, UnallocatedCString>>{
        {opentxs::crypto::SeedStyle::BIP39,
         {
             {opentxs::crypto::SeedStrength::Twelve, "12"},
             {opentxs::crypto::SeedStrength::Fifteen, "15"},
             {opentxs::crypto::SeedStrength::Eighteen, "18"},
             {opentxs::crypto::SeedStrength::TwentyOne, "21"},
             {opentxs::crypto::SeedStrength::TwentyFour, "24"},
         }},
        {opentxs::crypto::SeedStyle::PKT,
         {
             {opentxs::crypto::SeedStrength::Fifteen, "15"},
         }},
    };

    try {
        return map.at(type);
    } catch (...) {

        return null;
    }
}

auto Seed::Bip32Root(
    const UnallocatedCString& seedID,
    const PasswordPrompt& reason) const -> UnallocatedCString
{
    auto lock = Lock{seed_lock_};

    try {
        const auto& seed = get_seed(lock, seedID, reason);
        const auto entropy = factory_.DataFromBytes(seed.Entropy().Bytes());

        return entropy.asHex();
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return {};
    }
}

auto Seed::DefaultSeed() const -> std::pair<UnallocatedCString, std::size_t>
{
    auto lock = Lock{seed_lock_};
    const auto count = storage_.SeedList();

    return std::make_pair(storage_.DefaultSeed(), count.size());
}

auto Seed::GetHDKey(
    const UnallocatedCString& fingerprint,
    const opentxs::crypto::EcdsaCurve& curve,
    const UnallocatedVector<Bip32Index>& path,
    const PasswordPrompt& reason) const
    -> std::unique_ptr<opentxs::crypto::key::HD>
{
    return GetHDKey(
        fingerprint,
        curve,
        path,
        opentxs::crypto::key::asymmetric::Role::Sign,
        opentxs::crypto::key::EllipticCurve::DefaultVersion,
        reason);
}

auto Seed::GetHDKey(
    const UnallocatedCString& fingerprint,
    const opentxs::crypto::EcdsaCurve& curve,
    const UnallocatedVector<Bip32Index>& path,
    const opentxs::crypto::key::asymmetric::Role role,
    const PasswordPrompt& reason) const
    -> std::unique_ptr<opentxs::crypto::key::HD>
{
    return GetHDKey(
        fingerprint,
        curve,
        path,
        role,
        opentxs::crypto::key::EllipticCurve::DefaultVersion,
        reason);
}

auto Seed::GetHDKey(
    const UnallocatedCString& fingerprint,
    const opentxs::crypto::EcdsaCurve& curve,
    const UnallocatedVector<Bip32Index>& path,
    const VersionNumber version,
    const PasswordPrompt& reason) const
    -> std::unique_ptr<opentxs::crypto::key::HD>
{
    return GetHDKey(
        fingerprint,
        curve,
        path,
        opentxs::crypto::key::asymmetric::Role::Sign,
        version,
        reason);
}

auto Seed::GetHDKey(
    const UnallocatedCString& fingerprint,
    const opentxs::crypto::EcdsaCurve& curve,
    const UnallocatedVector<Bip32Index>& path,
    const opentxs::crypto::key::asymmetric::Role role,
    const VersionNumber version,
    const PasswordPrompt& reason) const
    -> std::unique_ptr<opentxs::crypto::key::HD>
{
    Bip32Index notUsed{0};
    auto seed = GetSeed(fingerprint, notUsed, reason);

    if (seed->empty()) { return {}; }

    return asymmetric_.NewHDKey(
        fingerprint, seed, curve, path, role, version, reason);
}

auto Seed::GetOrCreateDefaultSeed(
    UnallocatedCString& seedID,
    opentxs::crypto::SeedStyle& type,
    opentxs::crypto::Language& lang,
    Bip32Index& index,
    const opentxs::crypto::SeedStrength strength,
    const PasswordPrompt& reason) const -> OTSecret
{
    auto lock = Lock{seed_lock_};
    seedID = storage_.DefaultSeed();

    if (seedID.empty()) {
        seedID = new_seed(lock, type, lang, strength, {}, reason);
    }

    try {
        if (seedID.empty()) {
            throw std::runtime_error{"Failed to create seed"};
        }

        const auto& seed = get_seed(lock, seedID, reason);

        return seed.Entropy();
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return factory_.Secret(0);
    }
}

auto Seed::GetPaymentCode(
    const UnallocatedCString& fingerprint,
    const Bip32Index nym,
    const std::uint8_t version,
    const PasswordPrompt& reason) const
    -> std::unique_ptr<opentxs::crypto::key::Secp256k1>
{
    Bip32Index notUsed{0};
    auto seed = GetSeed(fingerprint, notUsed, reason);

    if (seed->empty()) { return {}; }

    auto pKey = asymmetric_.NewSecp256k1Key(
        fingerprint,
        seed,
        {HDIndex{Bip43Purpose::PAYCODE, Bip32Child::HARDENED},
         HDIndex{Bip44Type::BITCOIN, Bip32Child::HARDENED},
         HDIndex{nym, Bip32Child::HARDENED}},
        reason);

    if (!pKey) { return pKey; }

    const auto& key = *pKey;

    switch (version) {
        case 3: {
        } break;
        case 1:
        case 2:
        default: {
            return pKey;
        }
    }

    const auto& api = asymmetric_.Internal().API();
    const auto code = [&] {
        auto out = factory_.Secret(0);
        api.Crypto().Hash().Digest(
            opentxs::crypto::HashType::Sha256D,
            key.PublicKey(),
            out->WriteInto());

        return out;
    }();
    const auto path = [&] {
        auto out = proto::HDPath{};
        key.Path(out);

        return out;
    }();
    using Type = opentxs::crypto::key::asymmetric::Algorithm;

    return factory::Secp256k1Key(
        api,
        api.Crypto().Internal().EllipticProvider(Type::Secp256k1),
        factory_.SecretFromBytes(key.PrivateKey(reason)),
        code,
        factory_.DataFromBytes(key.PublicKey()),
        path,
        key.Parent(),
        key.Role(),
        key.Version(),
        reason);
}

auto Seed::GetStorageKey(
    const UnallocatedCString& fingerprint,
    const PasswordPrompt& reason) const -> OTSymmetricKey
{
    auto pKey = GetHDKey(
        fingerprint,
        opentxs::crypto::EcdsaCurve::secp256k1,
        {HDIndex{Bip43Purpose::FS, Bip32Child::HARDENED},
         HDIndex{Bip32Child::ENCRYPT_KEY, Bip32Child::HARDENED}},
        reason);

    if (false == bool(pKey)) {
        LogError()(OT_PRETTY_CLASS())("Failed to derive storage key.").Flush();

        return OTSymmetricKey{opentxs::factory::SymmetricKey()};
    }

    const auto& key = *pKey;

    return symmetric_.Key(
        opentxs::Context().Factory().SecretFromBytes(key.PrivateKey(reason)));
}

auto Seed::GetSeed(
    const UnallocatedCString& seedID,
    Bip32Index& index,
    const PasswordPrompt& reason) const -> OTSecret
{
    auto lock = Lock{seed_lock_};

    try {
        const auto& seed = get_seed(lock, seedID, reason);
        index = seed.Index();

        return seed.Entropy();
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return factory_.Secret(0);
    }
}

auto Seed::GetSeed(const identifier::Generic& id, const PasswordPrompt& reason)
    const noexcept -> opentxs::crypto::Seed
{
    auto lock = Lock{seed_lock_};

    try {

        return get_seed(lock, id.asBase58(api_.Crypto()), reason);
    } catch (...) {

        return {};
    }
}

auto Seed::get_seed(
    const Lock& lock,
    const UnallocatedCString& seedID,
    const PasswordPrompt& reason) const noexcept(false)
    -> opentxs::crypto::Seed&
{
    auto proto = proto::Seed{};

    if (auto it{seeds_.find(seedID)}; it != seeds_.end()) { return it->second; }

    if (false == storage_.Load(seedID, proto)) {
        throw std::runtime_error{
            UnallocatedCString{"Failed to load seed "} + seedID};
    }

    auto seed = factory::Seed(
        api_, bip39_, symmetric_, factory_, storage_, proto, reason);
    auto id = seed.ID().asBase58(api_.Crypto());

    OT_ASSERT(id == seedID);

    const auto [it, added] = seeds_.try_emplace(std::move(id), std::move(seed));

    if (false == added) {
        throw std::runtime_error{"failed to instantiate seed"};
    }

    return it->second;
}

auto Seed::ImportRaw(const Secret& entropy, const PasswordPrompt& reason) const
    -> UnallocatedCString
{
    auto lock = Lock{seed_lock_};

    try {
        return new_seed(
            lock,
            {},  // NOTE: no comment
            factory::Seed(
                bip32_,
                bip39_,
                symmetric_,
                factory_,
                storage_,
                entropy,
                reason));
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return {};
    }
}

auto Seed::ImportSeed(
    const Secret& words,
    const Secret& passphrase,
    const opentxs::crypto::SeedStyle type,
    const opentxs::crypto::Language lang,
    const PasswordPrompt& reason,
    const std::string_view comment) const -> UnallocatedCString
{
    switch (type) {
        case opentxs::crypto::SeedStyle::BIP39:
        case opentxs::crypto::SeedStyle::PKT: {
        } break;
        case opentxs::crypto::SeedStyle::Error:
        case opentxs::crypto::SeedStyle::BIP32:
        default: {
            LogError()(OT_PRETTY_CLASS())("Unsupported seed type").Flush();

            return {};
        }
    }

    auto lock = Lock{seed_lock_};

    try {
        return new_seed(
            lock,
            comment,
            factory::Seed(
                api_,
                bip32_,
                bip39_,
                symmetric_,
                factory_,
                storage_,
                type,
                lang,
                words,
                passphrase,
                reason));
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return {};
    }
}

auto Seed::LongestWord(
    const opentxs::crypto::SeedStyle type,
    const opentxs::crypto::Language lang) const noexcept -> std::size_t
{
    using Type = opentxs::crypto::SeedStyle;

    switch (type) {
        case Type::BIP39:
        case Type::PKT: {

            return bip39_.LongestWord(lang);
        }
        case Type::BIP32: {

            return 130_uz;
        }
        case Type::Error:
        default: {

            return {};
        }
    }
}

auto Seed::NewSeed(
    const opentxs::crypto::SeedStyle type,
    const opentxs::crypto::Language lang,
    const opentxs::crypto::SeedStrength strength,
    const PasswordPrompt& reason,
    const std::string_view comment) const -> UnallocatedCString
{
    auto lock = Lock{seed_lock_};

    return new_seed(lock, type, lang, strength, comment, reason);
}

auto Seed::new_seed(
    const Lock& lock,
    const opentxs::crypto::SeedStyle type,
    const opentxs::crypto::Language lang,
    const opentxs::crypto::SeedStrength strength,
    const std::string_view comment,
    const PasswordPrompt& reason) const noexcept -> UnallocatedCString
{
    switch (type) {
        case opentxs::crypto::SeedStyle::BIP39: {
        } break;
        case opentxs::crypto::SeedStyle::Error:
        case opentxs::crypto::SeedStyle::BIP32:
        case opentxs::crypto::SeedStyle::PKT:
        default: {
            LogError()(OT_PRETTY_CLASS())("Unsupported seed type").Flush();

            return {};
        }
    }

    try {
        auto seed = factory::Seed(
            api_,
            bip32_,
            bip39_,
            symmetric_,
            factory_,
            storage_,
            lang,
            strength,
            reason);
        const auto check = factory::Seed(
            api_,
            bip32_,
            bip39_,
            symmetric_,
            factory_,
            storage_,
            seed.Type(),
            lang,
            seed.Words(),
            seed.Phrase(),
            reason);

        OT_ASSERT(seed.ID() == check.ID());

        return new_seed(lock, comment, std::move(seed));
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return {};
    }
}

auto Seed::new_seed(
    const Lock&,
    const std::string_view comment,
    opentxs::crypto::Seed&& seed) const noexcept -> UnallocatedCString
{
    const auto& id = seed.ID();
    const auto sID = id.asBase58(api_.Crypto());
    seeds_.try_emplace(sID, std::move(seed));

    if (valid(comment)) { SetSeedComment(id, comment); }

    publish(id);

    return sID;
}

auto Seed::Passphrase(
    const UnallocatedCString& seedID,
    const PasswordPrompt& reason) const -> UnallocatedCString
{
    auto lock = Lock{seed_lock_};

    try {
        const auto& seed = get_seed(lock, seedID, reason);

        return UnallocatedCString{seed.Phrase().Bytes()};
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return {};
    }
}

auto Seed::publish(const UnallocatedCString& id) const noexcept -> void
{
    publish(api_.Factory().IdentifierFromBase58(id));
}

auto Seed::publish(const identifier::Generic& id) const noexcept -> void
{
    socket_->Send([&] {
        auto out = MakeWork(WorkType::SeedUpdated);
        out.AddFrame(id);

        return out;
    }());
}

auto Seed::SeedDescription(UnallocatedCString seedID) const noexcept
    -> UnallocatedCString
{
    auto lock = Lock{seed_lock_};
    const auto primary = storage_.DefaultSeed();

    if (seedID.empty()) { seedID = primary; }

    const auto isDefault = (seedID == primary);

    try {
        const auto [type, alias] = [&] {
            auto proto = proto::Seed{};
            auto name = UnallocatedCString{};

            if (false == storage_.Load(seedID, proto, name)) {
                throw std::runtime_error{
                    UnallocatedCString{"Failed to load seed "} + seedID};
            }

            return std::make_pair(
                opentxs::crypto::internal::Seed::Translate(proto.type()),
                std::move(name));
        }();
        auto out = std::stringstream{};

        if (alias.empty()) {
            out << "Unnamed seed";
        } else {
            out << alias;
        }

        out << ": ";
        out << opentxs::print(type);

        if (isDefault) { out << " (default)"; }

        return out.str();
    } catch (...) {

        return "Invalid seed";
    }
}

auto Seed::SetDefault(const identifier::Generic& id) const noexcept -> bool
{
    if (id.empty()) {
        LogError()(OT_PRETTY_CLASS())("Invalid id").Flush();

        return false;
    }

    const auto seedID = id.asBase58(api_.Crypto());
    const auto exists = [&] {
        for (const auto& [value, alias] : api_.Storage().SeedList()) {
            if (value == seedID) { return true; }
        }

        return false;
    }();

    if (false == exists) {
        LogError()(OT_PRETTY_CLASS())("Seed ")(id)(" does not exist").Flush();

        return false;
    }

    const auto out = api_.Storage().SetDefaultSeed(seedID);

    if (out) { publish(id); }

    return out;
}

auto Seed::SetSeedComment(
    const identifier::Generic& id,
    const std::string_view comment) const noexcept -> bool
{
    const auto alias = std::string{comment};

    if (api_.Storage().SetSeedAlias(id.asBase58(api_.Crypto()), alias)) {
        LogVerbose()(OT_PRETTY_CLASS())("Changed seed comment for ")(
            id)(" to ")(alias)
            .Flush();
        publish(id);

        return true;
    } else {
        LogError()(OT_PRETTY_CLASS())("Failed to set seed comment for ")(
            id)(" to ")(alias)
            .Flush();

        return false;
    }
}

auto Seed::UpdateIndex(
    const UnallocatedCString& seedID,
    const Bip32Index index,
    const PasswordPrompt& reason) const -> bool
{
    auto lock = Lock{seed_lock_};

    try {
        auto& seed = get_seed(lock, seedID, reason);

        return seed.Internal().IncrementIndex(index);
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return false;
    }
}

auto Seed::ValidateWord(
    const opentxs::crypto::SeedStyle type,
    const opentxs::crypto::Language lang,
    const std::string_view word) const noexcept
    -> UnallocatedVector<UnallocatedCString>
{
    switch (type) {
        case opentxs::crypto::SeedStyle::BIP39:
        case opentxs::crypto::SeedStyle::PKT: {

            return bip39_.GetSuggestions(lang, word);
        }
        case opentxs::crypto::SeedStyle::Error:
        case opentxs::crypto::SeedStyle::BIP32:
        default: {

            return {};
        }
    }
}

auto Seed::WordCount(
    const opentxs::crypto::SeedStyle type,
    const opentxs::crypto::SeedStrength strength) const noexcept -> std::size_t
{
    switch (type) {
        case opentxs::crypto::SeedStyle::BIP39:
        case opentxs::crypto::SeedStyle::PKT: {
        } break;
        case opentxs::crypto::SeedStyle::Error:
        case opentxs::crypto::SeedStyle::BIP32:
        default: {
            LogError()(OT_PRETTY_CLASS())("Unsupported seed type").Flush();

            return {};
        }
    }

    static const auto map =
        UnallocatedMap<opentxs::crypto::SeedStrength, std::size_t>{
            {opentxs::crypto::SeedStrength::Twelve, 12},
            {opentxs::crypto::SeedStrength::Fifteen, 15},
            {opentxs::crypto::SeedStrength::Eighteen, 18},
            {opentxs::crypto::SeedStrength::TwentyOne, 21},
            {opentxs::crypto::SeedStrength::TwentyFour, 24},
        };

    try {

        return map.at(strength);
    } catch (...) {

        return {};
    }
}

auto Seed::Words(const UnallocatedCString& seedID, const PasswordPrompt& reason)
    const -> UnallocatedCString
{
    auto lock = Lock{seed_lock_};

    try {
        const auto& seed = get_seed(lock, seedID, reason);

        return UnallocatedCString{seed.Words().Bytes()};
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return {};
    }
}
}  // namespace opentxs::api::crypto::imp
