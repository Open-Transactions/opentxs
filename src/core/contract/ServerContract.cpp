// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "core/contract/ServerContract.hpp"  // IWYU pragma: associated

#include <opentxs/protobuf/ListenAddress.pb.h>
#include <opentxs/protobuf/Nym.pb.h>
#include <opentxs/protobuf/ServerContract.pb.h>
#include <opentxs/protobuf/Signature.pb.h>
#include <algorithm>
#include <functional>
#include <iterator>
#include <memory>
#include <span>
#include <stdexcept>
#include <string_view>
#include <tuple>
#include <utility>

#include "core/contract/Signable.hpp"
#include "internal/core/String.hpp"
#include "internal/core/contract/Contract.hpp"
#include "internal/core/identifier/Identifier.hpp"
#include "internal/identity/Nym.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/Types.internal.hpp"
#include "opentxs/api/Factory.internal.hpp"
#include "opentxs/api/Session.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Wallet.hpp"
#include "opentxs/contract/Types.hpp"
#include "opentxs/contract/Types.internal.hpp"
#include "opentxs/crypto/SignatureRole.hpp"  // IWYU pragma: keep
#include "opentxs/crypto/Types.hpp"
#include "opentxs/identifier/Notary.hpp"
#include "opentxs/identifier/Nym.hpp"
#include "opentxs/identity/Nym.hpp"
#include "opentxs/identity/NymCapability.hpp"  // IWYU pragma: keep
#include "opentxs/internal.factory.hpp"
#include "opentxs/protobuf/Types.internal.hpp"
#include "opentxs/protobuf/syntax/ServerContract.hpp"  // IWYU pragma: keep
#include "opentxs/protobuf/syntax/Types.internal.tpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Writer.hpp"

namespace opentxs
{
auto Factory::ServerContract(const api::Session& api) noexcept
    -> std::unique_ptr<contract::Server>
{
    return std::make_unique<contract::blank::Server>(api);
}

auto Factory::ServerContract(
    const api::Session& api,
    const Nym_p& nym,
    const UnallocatedList<Endpoint>& endpoints,
    const UnallocatedCString& terms,
    const UnallocatedCString& name,
    const VersionNumber version,
    const opentxs::PasswordPrompt& reason) noexcept
    -> std::unique_ptr<contract::Server>
{
    using ReturnType = contract::implementation::Server;

    if (false == bool(nym)) { return {}; }
    if (false ==
        nym->HasCapability(identity::NymCapability::AUTHENTICATE_CONNECTION)) {
        return {};
    }

    auto list = UnallocatedList<contract::Server::Endpoint>{};
    std::ranges::transform(
        endpoints,
        std::back_inserter(list),
        [](const auto& in) -> contract::Server::Endpoint {
            return {
                static_cast<AddressType>(std::get<0>(in)),
                static_cast<contract::ProtocolVersion>(std::get<1>(in)),
                std::get<2>(in),
                std::get<3>(in),
                std::get<4>(in)};
        });

    try {
        auto key = api.Factory().Data();
        nym->TransportKey(key, reason);
        auto output = std::make_unique<ReturnType>(
            api, nym, version, terms, name, std::move(list), std::move(key));

        assert_false(nullptr == output);

        auto& contract = *output;

        if (false == contract.update_signature(reason)) {
            throw std::runtime_error{"Failed to sign contract"};
        }

        if (!contract.Validate()) {
            throw std::runtime_error{"Invalid contract"};
        }

        return output;
    } catch (const std::exception& e) {
        LogError()()(e.what()).Flush();

        return {};
    }
}

auto Factory::ServerContract(
    const api::Session& api,
    const Nym_p& nym,
    const protobuf::ServerContract& serialized) noexcept
    -> std::unique_ptr<contract::Server>
{
    using ReturnType = contract::implementation::Server;

    if (false == protobuf::syntax::check(LogError(), serialized)) {
        return nullptr;
    }

    auto contract = std::make_unique<ReturnType>(api, nym, serialized);

    if (!contract) { return nullptr; }

    if (!contract->Validate()) { return nullptr; }

    return contract;
}
}  // namespace opentxs

namespace opentxs::contract
{
const VersionNumber Server::DefaultVersion{2};
}  // namespace opentxs::contract

namespace opentxs::contract::implementation
{
using namespace std::literals;

Server::Server(
    const api::Session& api,
    const Nym_p& nym,
    const VersionNumber version,
    const UnallocatedCString& terms,
    const UnallocatedCString& name,
    UnallocatedList<contract::Server::Endpoint>&& endpoints,
    ByteArray&& key,
    Signatures&& signatures)
    : Signable(
          api,
          nym,
          version,
          terms,
          nym ? nym->Name() : "",
          std::move(signatures))
    , listen_params_(std::move(endpoints))
    , name_(name)
    , transport_key_(std::move(key))
{
    first_time_init();
}

// NOLINTBEGIN(clang-analyzer-cplusplus.NewDeleteLeaks)
Server::Server(
    const api::Session& api,
    const Nym_p& nym,
    const protobuf::ServerContract& serialized)
    : Signable(
          api,
          nym,
          serialized.version(),
          serialized.terms(),
          serialized.name(),
          ""s,
          api.Factory().Internal().NotaryID(serialized.id()),
          serialized.has_signature()
              ? Signatures{std::make_shared<protobuf::Signature>(
                    serialized.signature())}
              : Signatures{})
    , listen_params_(extract_endpoints(serialized))
    , name_(serialized.name())
    , transport_key_(api.Factory().DataFromBytes(serialized.transportkey()))
{
    init_serialized();
}
// NOLINTEND(clang-analyzer-cplusplus.NewDeleteLeaks)

Server::Server(const Server& rhs)
    : Signable(rhs)
    , listen_params_(rhs.listen_params_)
    , name_(rhs.name_)
    , transport_key_(rhs.transport_key_)
{
}

auto Server::calculate_id() const -> identifier_type
{
    return api_.Factory().Internal().NotaryIDFromPreimage(IDVersion());
}

auto Server::EffectiveName() const -> UnallocatedCString
{
    assert_false(nullptr == Signer());

    // TODO The version stored in Signer() might be out of date so load it from
    // the wallet. This can be fixed correctly by implementing in-place updates
    // of Nym credentials
    const auto nym = api_.Wallet().Nym(Signer()->ID());
    const auto output = nym->Name();

    if (output.empty()) { return name_; }

    return output;
}

auto Server::extract_endpoints(
    const protobuf::ServerContract& serialized) noexcept
    -> UnallocatedList<contract::Server::Endpoint>
{
    auto output = UnallocatedList<contract::Server::Endpoint>{};

    for (const auto& listen : serialized.address()) {
        // WARNING: preserve the order of this list, or signature verfication
        // will fail!
        output.emplace_back(
            translate(listen.type()),
            translate(listen.protocol()),
            listen.host(),
            listen.port(),
            listen.version());
    }

    return output;
}

auto Server::ConnectInfo(
    UnallocatedCString& strHostname,
    std::uint32_t& nPort,
    AddressType& actual,
    const AddressType& preferred) const -> bool
{
    if (0 < listen_params_.size()) {
        for (const auto& endpoint : listen_params_) {
            const auto& type = std::get<0>(endpoint);
            const auto& url = std::get<2>(endpoint);
            const auto& port = std::get<3>(endpoint);

            if (preferred == type) {
                strHostname = url;
                nPort = port;
                actual = type;

                return true;
            }
        }

        // If we didn't find the preferred type, return the first result
        const auto& endpoint = listen_params_.front();
        const auto& type = std::get<0>(endpoint);
        const auto& url = std::get<2>(endpoint);
        const auto& port = std::get<3>(endpoint);
        strHostname = url;
        nPort = port;
        actual = type;

        return true;
    }

    return false;
}

auto Server::contract() const -> protobuf::ServerContract
{
    auto contract = SigVersion();
    const auto sigs = signatures();

    if (false == sigs.empty()) {
        contract.mutable_signature()->CopyFrom(*sigs.front());
    }

    return contract;
}

auto Server::IDVersion() const -> protobuf::ServerContract
{
    protobuf::ServerContract contract;
    contract.set_version(Version());
    contract.clear_id();         // reinforcing that this field must be blank.
    contract.clear_signature();  // reinforcing that this field must be blank.
    contract.clear_publicnym();  // reinforcing that this field must be blank.

    if (Signer()) {
        Signer()->ID().Internal().Serialize(*contract.mutable_nymid());
    }

    contract.set_name(name_);

    for (const auto& endpoint : listen_params_) {
        auto& addr = *contract.add_address();
        const auto& version = std::get<4>(endpoint);
        const auto& type = std::get<0>(endpoint);
        const auto& protocol = std::get<1>(endpoint);
        const auto& url = std::get<2>(endpoint);
        const auto& port = std::get<3>(endpoint);
        addr.set_version(version);
        addr.set_type(translate(type));
        addr.set_protocol(translate(protocol));
        addr.set_host(url);
        addr.set_port(port);
    }

    contract.set_terms(UnallocatedCString{Terms()});
    contract.set_transportkey(transport_key_.data(), transport_key_.size());

    return contract;
}

auto Server::SetAlias(std::string_view alias) noexcept -> bool
{
    InitAlias(alias);
    api_.Wallet().SetServerAlias(ID(), alias);

    return true;
}

auto Server::SigVersion() const -> protobuf::ServerContract
{
    auto contract = IDVersion();
    ID().Internal().Serialize(*contract.mutable_id());

    return contract;
}

auto Server::Serialize(Writer&& out) const noexcept -> bool
{
    return serialize(contract(), std::move(out));
}

auto Server::Serialize(Writer&& destination, bool includeNym) const -> bool
{
    auto serialized = protobuf::ServerContract{};
    if (false == Serialize(serialized, includeNym)) {
        LogError()()("Failed to serialize server.").Flush();
        return false;
    }

    write(serialized, std::move(destination));

    return true;
}

auto Server::Serialize(protobuf::ServerContract& serialized, bool includeNym)
    const -> bool
{
    serialized = contract();

    if (includeNym && Signer()) {
        auto publicNym = protobuf::Nym{};
        if (false == Signer()->Internal().Serialize(publicNym)) {
            return false;
        }
        *(serialized.mutable_publicnym()) = publicNym;
    }

    return true;
}

auto Server::Statistics(String& strContents) const -> bool
{
    strContents.Concatenate(" Notary Provider: "sv)
        .Concatenate(Signer()->Alias())
        .Concatenate(" NotaryID: "sv)
        .Concatenate(ID().asBase58(api_.Crypto()))
        .Concatenate("\n\n"sv);

    return true;
}

auto Server::TransportKey() const -> const Data& { return transport_key_; }

auto Server::TransportKey(Data& pubkey, const PasswordPrompt& reason) const
    -> Secret
{
    assert_false(nullptr == Signer());

    return Signer()->TransportKey(pubkey, reason);
}

auto Server::update_signature(const PasswordPrompt& reason) -> bool
{
    if (!Signable::update_signature(reason)) { return false; }

    bool success = false;
    auto sigs = Signatures{};
    auto serialized = SigVersion();
    auto& signature = *serialized.mutable_signature();
    success = Signer()->Internal().Sign(
        serialized, crypto::SignatureRole::ServerContract, signature, reason);

    if (success) {
        sigs.emplace_back(new protobuf::Signature(signature));
        add_signatures(std::move(sigs));
    } else {
        LogError()()("failed to create signature.").Flush();
    }

    return success;
}

auto Server::validate() const -> bool
{
    auto validNym = false;

    if (Signer()) { validNym = Signer()->VerifyPseudonym(); }

    if (!validNym) {
        LogError()()("Invalid nym.").Flush();

        return false;
    }

    const bool validSyntax = protobuf::syntax::check(LogError(), contract());

    if (!validSyntax) {
        LogError()()("Invalid syntax.").Flush();

        return false;
    }

    const auto sigs = signatures();

    if (1_uz != sigs.size()) {
        LogError()()("Missing signature.").Flush();

        return false;
    }

    bool validSig = false;
    const auto& signature = sigs.front();

    if (signature) { validSig = verify_signature(*signature); }

    if (!validSig) {
        LogError()()("Invalid signature.").Flush();

        return false;
    }

    return true;
}

auto Server::verify_signature(const protobuf::Signature& signature) const
    -> bool
{
    if (!Signable::verify_signature(signature)) { return false; }

    auto serialized = SigVersion();
    serialized.mutable_signature()->CopyFrom(signature);
    auto& sigProto = *serialized.mutable_signature();
    sigProto.CopyFrom(signature);

    return Signer()->Internal().Verify(serialized, sigProto);
}
}  // namespace opentxs::contract::implementation
