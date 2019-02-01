// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_API_CLIENT_ISSUER_HPP
#define OPENTXS_API_CLIENT_ISSUER_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/Proto.hpp"

#include <set>
#include <string>
#include <tuple>
#include <vector>

namespace opentxs
{
namespace api
{
namespace client
{

class Issuer
{
public:
    typedef std::pair<OTIdentifier, proto::BailmentReply> BailmentDetails;
    typedef std::pair<OTIdentifier, proto::ConnectionInfoReply>
        ConnectionDetails;

    enum class RequestStatus : std::int32_t {
        All = -1,
        None = 0,
        Requested = 1,
        Replied = 2,
        Unused = 3,
    };

    virtual operator std::string() const = 0;

    virtual std::set<OTIdentifier> AccountList(
        const proto::ContactItemType type,
        const Identifier& unitID) const = 0;
    virtual bool BailmentInitiated(const Identifier& unitID) const = 0;
    virtual std::vector<BailmentDetails> BailmentInstructions(
        const Identifier& unitID,
        const bool onlyUnused = true) const = 0;
    virtual std::vector<ConnectionDetails> ConnectionInfo(
        const proto::ConnectionInfoType type) const = 0;
    virtual bool ConnectionInfoInitiated(
        const proto::ConnectionInfoType type) const = 0;
    virtual std::set<std::tuple<OTIdentifier, OTIdentifier, bool>> GetRequests(
        const proto::PeerRequestType type,
        const RequestStatus state = RequestStatus::All) const = 0;
    virtual const Identifier& IssuerID() const = 0;
    virtual const identifier::Nym& LocalNymID() const = 0;
    virtual bool Paired() const = 0;
    virtual const std::string& PairingCode() const = 0;
    virtual OTIdentifier PrimaryServer() const = 0;
    virtual std::set<proto::PeerRequestType> RequestTypes() const = 0;
    virtual proto::Issuer Serialize() const = 0;
    virtual bool StoreSecretComplete() const = 0;
    virtual bool StoreSecretInitiated() const = 0;

    virtual void AddAccount(
        const proto::ContactItemType type,
        const Identifier& unitID,
        const Identifier& accountID) = 0;
    virtual bool AddReply(
        const proto::PeerRequestType type,
        const Identifier& requestID,
        const Identifier& replyID) = 0;
    virtual bool AddRequest(
        const proto::PeerRequestType type,
        const Identifier& requestID) = 0;
    virtual bool RemoveAccount(
        const proto::ContactItemType type,
        const Identifier& unitID,
        const Identifier& accountID) = 0;
    virtual void SetPaired(const bool paired) = 0;
    virtual void SetPairingCode(const std::string& code) = 0;
    virtual bool SetUsed(
        const proto::PeerRequestType type,
        const Identifier& requestID,
        const bool isUsed = true) = 0;

    virtual ~Issuer() = default;

protected:
    Issuer() = default;

private:
    Issuer(const Issuer&) = delete;
    Issuer(Issuer&&) = delete;
    Issuer& operator=(const Issuer&) = delete;
    Issuer& operator=(Issuer&&) = delete;
};
}  // namespace client
}  // namespace api
}  // namespace opentxs
#endif
