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

#ifndef OPENTXS_NETWORK_OPENDHT_HPP
#define OPENTXS_NETWORK_OPENDHT_HPP

#ifdef OT_DHT

#include "opentxs/core/Types.hpp"

#include <atomic>
#include <memory>
#include <mutex>

namespace dht
{
class DhtRunner;
}

namespace opentxs
{

class DhtConfig;
class Data;

//Low interface to OpenDHT. Does not depend on opentxs.
class OpenDHT
{
private:
    static OpenDHT* instance_;

    std::unique_ptr<const DhtConfig> config_;
    std::unique_ptr<dht::DhtRunner> node_;
    mutable std::atomic<bool> loaded_;
    mutable std::atomic<bool> ready_;
    mutable std::mutex init_;

    OpenDHT(const DhtConfig& config);
    OpenDHT() = delete;
    OpenDHT(const OpenDHT&) = delete;
    OpenDHT& operator=(const OpenDHT&) = delete;

    bool Init() const;

public:
    static OpenDHT& It(const DhtConfig& config);
    void Insert(
        const std::string& key,
        const std::string& value,
        DhtDoneCallback cb={}) const;
    void Retrieve(
        const std::string& key,
        DhtResultsCallback vcb,
        DhtDoneCallback dcb={}) const;
    void Cleanup();
    ~OpenDHT();
};

}  // namespace opentxs
#endif // OT_DHT
#endif // OPENTXS_NETWORK_OPENDHT_HPP
