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

#ifndef OPENTXS_CORE_DATA_HPP
#define OPENTXS_CORE_DATA_HPP

#include <cstdint>
#include <vector>

namespace opentxs
{

class OTASCIIArmor;

class Data
{
private:
    typedef std::vector<std::uint8_t> Vector;

    Vector data_{};
    std::size_t position_{};

    void concatenate(const Vector& data);

protected:
    void Initialize();
    void swap(Data& rhs);

public:
    EXPORT Data() = default;
    EXPORT explicit Data(const void* data, std::size_t size);
    EXPORT explicit Data(const OTASCIIArmor& source);
    EXPORT explicit Data(const std::vector<unsigned char>& sourceVector);
    EXPORT Data(const Data& rhs);
    EXPORT Data(Data&& rhs);
    EXPORT Data& operator=(const Data& rhs);
    EXPORT Data& operator=(Data&& rhs);

    EXPORT bool operator==(const Data& rhs) const;
    EXPORT bool operator!=(const Data& rhs) const;
    EXPORT Data& operator+=(const Data& rhs);

    EXPORT bool empty() const;
    EXPORT bool IsEmpty() const;
    EXPORT const void* GetPointer() const;
    EXPORT std::size_t GetSize() const;

    EXPORT void Assign(const Data& source);
    EXPORT void Assign(const void* data, const std::size_t& size);
    EXPORT void Concatenate(const void* data, const std::size_t& size);
    EXPORT std::size_t OTfread(std::uint8_t* data, const std::size_t& size);
    EXPORT bool Randomize(const std::size_t& size);
    EXPORT void Release();
    EXPORT void reset();
    EXPORT void SetSize(const std::size_t& size);
    EXPORT void swap(Data&& rhs);
    EXPORT void zeroMemory();

    EXPORT virtual ~Data() = default;
};
}  // namespace opentxs

#endif  // OPENTXS_CORE_DATA_HPP
