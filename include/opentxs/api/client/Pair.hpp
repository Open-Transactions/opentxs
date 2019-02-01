// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_API_CLIENT_PAIR_HPP
#define OPENTXS_API_CLIENT_PAIR_HPP

#include "opentxs/Forward.hpp"

#include <set>
#include <string>

namespace opentxs
{
namespace api
{
namespace client
{
class Pair
{
public:
    virtual bool AddIssuer(
        const identifier::Nym& localNymID,
        const identifier::Nym& issuerNymID,
        const std::string& pairingCode) const = 0;
    virtual bool CheckIssuer(
        const identifier::Nym& localNymID,
        const Identifier& unitDefinitionID) const = 0;
    virtual std::string IssuerDetails(
        const identifier::Nym& localNymID,
        const identifier::Nym& issuerNymID) const = 0;
    virtual std::set<OTIdentifier> IssuerList(
        const identifier::Nym& localNymID,
        const bool onlyTrusted) const = 0;
    virtual void Update() const = 0;

    virtual ~Pair() = default;

protected:
    Pair() = default;

private:
    Pair(const Pair&) = delete;
    Pair(Pair&&) = delete;
    Pair& operator=(const Pair&) = delete;
    Pair& operator=(Pair&&) = delete;
};
}  // namespace client
}  // namespace api
}  // namespace opentxs
#endif
