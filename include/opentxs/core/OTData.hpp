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

#ifndef OPENTXS_CORE_OTDATA_HPP
#define OPENTXS_CORE_OTDATA_HPP

#include <cstdint>
#include <vector>

namespace opentxs
{

class OTASCIIArmor;

class OTData
{
private:
    void* data_{nullptr};
    std::uint32_t position_{0};
    std::size_t size_={0};

protected:
    void Initialize();

public:
    EXPORT OTData() = default;
    EXPORT explicit OTData(const std::uint32_t num);
    EXPORT explicit OTData(const std::int64_t num);
    EXPORT explicit OTData(const void* data, std::size_t size);
    EXPORT explicit OTData(const OTASCIIArmor& source);
    EXPORT explicit OTData(const std::vector<unsigned char>& sourceVector);
    EXPORT OTData(const OTData& rhs);
    EXPORT OTData(OTData&& rhs);
    EXPORT OTData& operator=(const OTData& rhs);
    EXPORT OTData& operator=(OTData&& rhs);

    EXPORT bool operator==(const OTData& rhs) const;
    EXPORT bool operator!=(const OTData& rhs) const;
    EXPORT OTData& operator+=(const OTData& rhs);

    EXPORT bool empty() const;
    EXPORT bool IsEmpty() const;
    EXPORT const void* GetPointer() const;
    EXPORT std::size_t GetSize() const;

    EXPORT void Assign(const OTData& source);
    EXPORT void Assign(const void* data, const std::size_t& size);
    EXPORT void Concatenate(const void* data, const std::size_t& size);
    EXPORT std::size_t OTfread(std::uint8_t* data, const std::size_t& size);
    EXPORT bool Randomize(const std::size_t& size);
    EXPORT void Release();
    EXPORT void reset();
    EXPORT void SetSize(const std::size_t& size);
    EXPORT void swap(OTData& rhs);
    EXPORT void zeroMemory();

    EXPORT virtual ~OTData();
};
} // namespace opentxs

#endif // OPENTXS_CORE_OTDATA_HPP
