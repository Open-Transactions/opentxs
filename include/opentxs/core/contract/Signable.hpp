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

#ifndef OPENTXS_CORE_SIGNABLE_HPP
#define OPENTXS_CORE_SIGNABLE_HPP

#include "opentxs/core/Data.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Proto.hpp"

#include <cstdint>
#include <list>
#include <memory>
#include <mutex>
#include <string>

namespace opentxs
{

typedef std::shared_ptr<const class Nym> ConstNym;
typedef std::shared_ptr<proto::Signature> SerializedSignature;

class Nym;

class Signable
{
protected:
    typedef std::unique_lock<std::mutex> Lock;
    typedef std::list<SerializedSignature> Signatures;

    std::string alias_;
    Identifier id_;
    const ConstNym nym_;
    Signatures signatures_;
    std::uint32_t version_ = 0;
    std::string conditions_;  // Human-readable portion
    mutable std::mutex lock_;

    /** Calculate the ID and verify that it matches the existing id_ value */
    bool CheckID(const Lock& lock) const;
    virtual Identifier id(const Lock& lock) const;
    virtual bool validate(const Lock& lock) const = 0;
    virtual bool verify_signature(
        const Lock& lock,
        const proto::Signature& signature) const;
    bool verify_write_lock(const Lock& lock) const;

    /** Calculate and unconditionally set id_ */
    bool CalculateID(const Lock& lock);
    virtual bool update_signature(const Lock& lock);

    /** Calculate identifier */
    virtual Identifier GetID(const Lock& lock) const = 0;

    Signable() = delete;
    explicit Signable(
        const ConstNym& nym);
    explicit Signable(
        const ConstNym& nym,
        const std::uint32_t version);
    explicit Signable(
        const ConstNym& nym,
        const std::uint32_t version,
        const std::string& conditions);
    Signable(const Signable&) = delete;
    Signable(Signable&&) = delete;
    Signable& operator=(const Signable&) = delete;
    Signable& operator=(Signable&&) = delete;

public:
    virtual std::string Alias() const;
    Identifier ID() const;
    virtual std::string Name() const = 0;
    ConstNym Nym() const;
    virtual const std::string& Terms() const;
    virtual Data Serialize() const = 0;
    bool Validate() const;

    virtual void SetAlias(const std::string& alias);

    virtual ~Signable() = default;
};
}  // namespace opentxs
#endif  // OPENTXS_CORE_SIGNABLE_HPP
