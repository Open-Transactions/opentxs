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

#ifndef OPENTXS_CORE_CONTRACT_SERVERCONTRACT_HPP
#define OPENTXS_CORE_CONTRACT_SERVERCONTRACT_HPP

#include "opentxs/core/contract/Signable.hpp"
#include "opentxs/core/Nym.hpp"
#include "opentxs/core/Proto.hpp"
#include "opentxs/network/ZMQ.hpp"

#include <cstdint>
#include <list>
#include <memory>
#include <string>
#include <tuple>

namespace opentxs
{

class Data;
class String;

class ServerContract : public Signable
{
public:
    /** A server listen address */
    typedef std::tuple<proto::AddressType,
                       proto::ProtocolVersion,
                       std::string,    // hostname / address
                       std::uint32_t,  // port
                       std::uint32_t>  // version
        Endpoint;

private:
    typedef Signable ot_super;

    std::list<Endpoint> listen_params_;
    std::string name_;
    Data transport_key_;

    proto::ServerContract contract(const Lock& lock) const;
    Identifier GetID(const Lock& lock) const override;
    proto::ServerContract IDVersion(const Lock& lock) const;
    proto::ServerContract SigVersion(const Lock& lock) const;
    bool validate(const Lock& lock) const override;
    bool verify_signature(
        const Lock& lock,
        const proto::Signature& signature) const override;

    bool update_signature(const Lock& lock) override;

    ServerContract() = delete;
    explicit ServerContract(const ConstNym& nym);
    ServerContract(
        const ConstNym& nym,
        const proto::ServerContract& serialized);

public:
    static ServerContract* Create(
        const ConstNym& nym,
        const std::list<Endpoint>& endpoints,
        const std::string& terms,
        const std::string& name);
    static ServerContract* Factory(
        const ConstNym& nym,
        const proto::ServerContract& serialized);

    bool ConnectInfo(
        std::string& strHostname,
        uint32_t& nPort,
        const proto::AddressType& preferred = proto::ADDRESSTYPE_IPV4) const;
    proto::ServerContract Contract() const;
    proto::ServerContract PublicContract() const;
    bool Statistics(String& strContents) const;
    const unsigned char* PublicTransportKey() const;
    zcert_t* PrivateTransportKey() const;

    std::string Name() const override { return name_; }
    Data Serialize() const override;

    void SetAlias(const std::string& alias) override;

    EXPORT ~ServerContract() = default;
};

}  // namespace opentxs

#endif  // OPENTXS_CORE_CONTRACT_SERVERCONTRACT_HPP
