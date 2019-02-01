// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_API_CLIENT_MANAGER_HPP
#define OPENTXS_API_CLIENT_MANAGER_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/api/Core.hpp"

#include <string>

namespace opentxs
{
namespace api
{
namespace client
{
class Manager : virtual public api::Core
{
public:
    EXPORT virtual const api::client::Activity& Activity() const = 0;
#if OT_CRYPTO_SUPPORTED_KEY_HD
    EXPORT virtual const api::client::Blockchain& Blockchain() const = 0;
#endif
    EXPORT virtual const api::client::Contacts& Contacts() const = 0;
    EXPORT virtual const OTAPI_Exec& Exec(
        const std::string& wallet = "") const = 0;
    EXPORT virtual std::recursive_mutex& Lock(
        const identifier::Nym& nymID,
        const Identifier& serverID) const = 0;
    EXPORT virtual const OT_API& OTAPI(
        const std::string& wallet = "") const = 0;
    EXPORT virtual const client::OTX& OTX() const = 0;
    EXPORT virtual const client::Pair& Pair() const = 0;
    EXPORT virtual const client::ServerAction& ServerAction() const = 0;
    EXPORT virtual const api::client::UI& UI() const = 0;
    EXPORT virtual const client::Workflow& Workflow() const = 0;
    EXPORT virtual const network::ZMQ& ZMQ() const = 0;

    EXPORT virtual ~Manager() = default;

protected:
    Manager() = default;

private:
    Manager(const Manager&) = delete;
    Manager(Manager&&) = delete;
    Manager& operator=(const Manager&) = delete;
    Manager& operator=(Manager&&) = delete;
};
}  // namespace client
}  // namespace api
}  // namespace opentxs
#endif
