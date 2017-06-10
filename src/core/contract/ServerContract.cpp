/************************************************************
 *
 *                 OPEN TRANSACTIONS
 *
 *       Financial Cryptography and Digital Cash
 *       Library, Protocol, API, Server, CLI, GUI
 *
 *       -- Anonymous Numbered Accounts.
 *       -- Untraceable Digital Cash.
 *       -- Triple-Signed Receipts.
 *       -- Cheques, Vouchers, Transfers, Inboxes.
 *       -- Basket Currencies, Markets, Payment Plans.
 *       -- Signed, XML, Ricardian-style Contracts.
 *       -- Scripted smart contracts.
 *
 *  EMAIL:
 *  fellowtraveler@opentransactions.org
 *
 *  WEBSITE:
 *  http://www.opentransactions.org/
 *
 *  -----------------------------------------------------
 *
 *   LICENSE:
 *   This Source Code Form is subject to the terms of the
 *   Mozilla Public License, v. 2.0. If a copy of the MPL
 *   was not distributed with this file, You can obtain one
 *   at http://mozilla.org/MPL/2.0/.
 *
 *   DISCLAIMER:
 *   This program is distributed in the hope that it will
 *   be useful, but WITHOUT ANY WARRANTY; without even the
 *   implied warranty of MERCHANTABILITY or FITNESS FOR A
 *   PARTICULAR PURPOSE.  See the Mozilla Public License
 *   for more details.
 *
 ************************************************************/

#include "opentxs/core/stdafx.hpp"

#include "opentxs/core/contract/ServerContract.hpp"

#include "opentxs/api/OT.hpp"
#include "opentxs/api/Wallet.hpp"
#include "opentxs/core/contract/Signable.hpp"
#include "opentxs/core/util/Assert.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/Nym.hpp"
#include "opentxs/core/Proto.hpp"
#include "opentxs/core/String.hpp"

#include <stdint.h>
#include <list>
#include <memory>
#include <string>
#include <tuple>

namespace opentxs
{

ServerContract::ServerContract(const ConstNym& nym)
    : ot_super(nym)
{
}

ServerContract::ServerContract(
    const ConstNym& nym,
    const proto::ServerContract& serialized)
    : ServerContract(nym)
{
    id_ = Identifier(serialized.id());
    signatures_.push_front(
        SerializedSignature(
            std::make_shared<proto::Signature>(serialized.signature())));
    version_ = serialized.version();
    conditions_ = serialized.terms();

    for (auto& listen : serialized.address()) {
        ServerContract::Endpoint endpoint{listen.type(),
                                          listen.protocol(),
                                          listen.host(),
                                          listen.port(),
                                          listen.version()};
        // WARNING: preserve the order of this list, or signature verfication
        // will fail!
        listen_params_.push_back(endpoint);
    }

    name_ = serialized.name();
    transport_key_.Assign(
        serialized.transportkey().c_str(), serialized.transportkey().size());
}

ServerContract* ServerContract::Create(
    const ConstNym& nym,
    const std::list<ServerContract::Endpoint>& endpoints,
    const std::string& terms,
    const std::string& name)
{
    OT_ASSERT(nullptr != nym);

    OT_ASSERT(nym->hasCapability(NymCapability::AUTHENTICATE_CONNECTION));

    ServerContract* contract = new ServerContract(nym);

    if (nullptr != contract) {
        contract->version_ = 1;
        contract->listen_params_ = endpoints;
        contract->conditions_ = terms;

        // TODO:: find the right defined constant. 32 is the correct size
        // according to https://github.com/zeromq/czmq
        contract->transport_key_.Assign(
            zcert_public_key(contract->PrivateTransportKey()), 32);
        contract->name_ = name;

        Lock lock(contract->lock_);

        if (!contract->CalculateID(lock)) { return nullptr; }

        if (contract->nym_) {
            contract->update_signature(lock);
        }

        if (!contract->validate(lock)) { return nullptr; }

        contract->alias_ = contract->name_;
    } else {
        otErr << __FUNCTION__ << ": Failed to create server contract."
              << std::endl;
    }

    return contract;
}

ServerContract* ServerContract::Factory(
    const ConstNym& nym,
    const proto::ServerContract& serialized)
{
    if (!proto::Check<proto::ServerContract>(serialized, 0, 0xFFFFFFFF)) {
        return nullptr;
    }

    std::unique_ptr<ServerContract> contract(
        new ServerContract(nym, serialized));

    if (!contract) { return nullptr; }

    Lock lock(contract->lock_);

    if (!contract->validate(lock)) { return nullptr; }

    contract->alias_ = contract->name_;

    return contract.release();
}

Identifier ServerContract::GetID(const Lock& lock) const
{
    auto contract = IDVersion(lock);
    Identifier id;
    id.CalculateDigest(proto::ProtoAsData<proto::ServerContract>(contract));
    return id;
}

bool ServerContract::ConnectInfo(
    std::string& strHostname,
    uint32_t& nPort,
    const proto::AddressType& preferred) const
{
    if (0 < listen_params_.size()) {
        for (auto& endpoint : listen_params_) {
            const auto& type = std::get<0>(endpoint);
            const auto& url = std::get<2>(endpoint);
            const auto& port = std::get<3>(endpoint);

            if (preferred == type) {
                strHostname = url;
                nPort = port;

                return true;
            }
        }

        // If we didn't find the preferred type, return the first result
        const auto& endpoint = listen_params_.front();
        const auto& url = std::get<2>(endpoint);
        const auto& port = std::get<3>(endpoint);
        strHostname = url;
        nPort = port;

        return true;
    }

    return false;
}

proto::ServerContract ServerContract::contract(const Lock& lock) const
{
    auto contract = SigVersion(lock);
    *(contract.mutable_signature()) = *(signatures_.front());

    return contract;
}

proto::ServerContract ServerContract::Contract() const
{
    Lock lock(lock_);

    return contract(lock);
}

proto::ServerContract ServerContract::IDVersion(const Lock& lock) const
{
    OT_ASSERT(verify_write_lock(lock));

    proto::ServerContract contract;
    contract.set_version(version_);
    contract.clear_id();         // reinforcing that this field must be blank.
    contract.clear_signature();  // reinforcing that this field must be blank.
    contract.clear_publicnym();  // reinforcing that this field must be blank.

    if (nullptr != nym_) {
        String nymID;
        nym_->GetIdentifier(nymID);
        contract.set_nymid(nymID.Get());
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
        addr.set_type(type);
        addr.set_protocol(protocol);
        addr.set_host(url);
        addr.set_port(port);
    }

    contract.set_terms(conditions_);
    contract.set_transportkey(
        transport_key_.GetPointer(), transport_key_.GetSize());

    return contract;
}

void ServerContract::SetAlias(const std::string& alias)
{
    ot_super::SetAlias(alias);

    OT::App().Contract().SetServerAlias(id_, alias);
}

proto::ServerContract ServerContract::SigVersion(const Lock& lock) const
{
    auto contract = IDVersion(lock);
    contract.set_id(String(id(lock)).Get());

    return contract;
}

proto::ServerContract ServerContract::PublicContract() const
{
    Lock lock(lock_);

    auto serialized = contract(lock);

    if (nym_) {
        auto publicNym = nym_->asPublicNym();
        *(serialized.mutable_publicnym()) = publicNym;
    }

    return serialized;
}

bool ServerContract::Statistics(String& strContents) const
{
    const String strID(id_);

    strContents.Concatenate(
        " Notary Provider: %s\n"
        " NotaryID: %s\n"
        "\n",
        nym_->Alias().c_str(),
        strID.Get());

    return true;
}

const unsigned char* ServerContract::PublicTransportKey() const
{
    return static_cast<const unsigned char*>(transport_key_.GetPointer());
}

zcert_t* ServerContract::PrivateTransportKey() const
{
    OT_ASSERT(nym_);

    return nym_->TransportKey();
}

Data ServerContract::Serialize() const
{
    Lock lock(lock_);

    return proto::ProtoAsData(contract(lock));
}

bool ServerContract::update_signature(const Lock& lock)
{
    if (!ot_super::update_signature(lock)) { return false; }

    bool success = false;
    signatures_.clear();
    auto serialized = SigVersion(lock);
    auto& signature = *serialized.mutable_signature();
    signature.set_role(proto::SIGROLE_SERVERCONTRACT);
    success = nym_->SignProto(serialized, signature);

    if (success) {
        signatures_.emplace_front(new proto::Signature(signature));
    } else {
        otErr << __FUNCTION__ << ": failed to create signature."
              << std::endl;
    }

    return success;
}

bool ServerContract::validate(const Lock& lock) const
{
    bool validNym = false;

    if (nym_) {
        validNym = nym_->VerifyPseudonym();
    }

    if (!validNym) {
        otErr << __FUNCTION__ << ": Invalid nym." << std::endl;

        return false;
    }

    const bool validSyntax = proto::Check(contract(lock), 0, 0xFFFFFFFF);

    if (!validSyntax) {
        otErr << __FUNCTION__ << ": Invalid syntax." << std::endl;

        return false;
    }

    if (1 > signatures_.size()) {
        otErr << __FUNCTION__ << ": Missing signature." << std::endl;

        return false;
    }

    bool validSig = false;
    auto& signature = *signatures_.cbegin();

    if (signature) {
        validSig = verify_signature(lock, *signature);
    }

    if (!validSig) {
        otErr << __FUNCTION__ << ": Invalid signature." << std::endl;

        return false;
    }

    return true;
}

bool ServerContract::verify_signature(
    const Lock& lock,
    const proto::Signature& signature) const
{
    if (!ot_super::verify_signature(lock, signature)) { return false; }

    auto serialized = SigVersion(lock);
    auto& sigProto = *serialized.mutable_signature();
    sigProto.CopyFrom(signature);

    return nym_->VerifyProto(serialized, sigProto);
}
}  // namespace opentxs
