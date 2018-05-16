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

#include "opentxs/stdafx.hpp"

#include "opentxs/core/Identifier.hpp"

#include "opentxs/api/crypto/Crypto.hpp"
#include "opentxs/api/crypto/Encode.hpp"
#include "opentxs/api/crypto/Hash.hpp"
#include "opentxs/api/Native.hpp"
#include "opentxs/core/crypto/OTCachedKey.hpp"
#include "opentxs/core/crypto/OTPassword.hpp"
#include "opentxs/core/crypto/OTSymmetricKey.hpp"
#include "opentxs/core/util/Assert.hpp"
#include "opentxs/core/Cheque.hpp"
#include "opentxs/core/Contract.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Nym.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/OT.hpp"

template class opentxs::Pimpl<opentxs::Identifier>;
template class std::set<opentxs::OTIdentifier>;
template class std::map<opentxs::OTIdentifier, std::set<opentxs::OTIdentifier>>;

namespace opentxs
{
bool operator==(const Identifier& lhs, const Identifier& rhs)
{
    return lhs.get() == rhs;
}

bool operator!=(const Identifier& lhs, const Identifier& rhs)
{
    return lhs.get() != rhs;
}

bool operator<(const Identifier& lhs, const Identifier& rhs)
{
    return lhs.get() < rhs.get();
}

OTIdentifier Identifier::Factory() { return OTIdentifier(new Identifier()); }

OTIdentifier Identifier::Factory(const Identifier& rhs)
{
    return OTIdentifier(new Identifier(rhs));
}

OTIdentifier Identifier::Factory(const std::string& rhs)
{
    return OTIdentifier(new Identifier(rhs));
}

OTIdentifier Identifier::Factory(const String& rhs)
{
    return OTIdentifier(new Identifier(rhs));
}

OTIdentifier Identifier::Factory(const Nym& nym)
{
    return OTIdentifier(new Identifier(nym));
}

OTIdentifier Identifier::Factory(const Contract& contract)
{
    return OTIdentifier(new Identifier(contract));
}

OTIdentifier Identifier::Factory(const Cheque& cheque)
{
    auto output = OTIdentifier(new Identifier());
    output->CalculateDigest(String(cheque));

    return output;
}

OTIdentifier Identifier::Factory(const OTSymmetricKey& key)
{
    return OTIdentifier(new Identifier(key));
}

OTIdentifier Identifier::Factory(const OTCachedKey& key)
{
    return OTIdentifier(new Identifier(key));
}

OTIdentifier Identifier::Factory(
    const proto::ContactItemType type,
    const proto::HDPath& path)
{
    return OTIdentifier(new Identifier(type, path));
}

OTIdentifier Identifier::Random()
{
    auto output = OTIdentifier(new Identifier);
    auto nonce = Data::Factory();
    OT::App().Crypto().Encode().Nonce(32, nonce);
    output->CalculateDigest(nonce);

    return output;
}

// static
bool Identifier::validateID(const std::string& strPurportedID)
{
    if (strPurportedID.empty()) {
        return false;
    }
    Identifier theID(strPurportedID);

    return (0 < theID.GetSize());
}

proto::HashType Identifier::IDToHashType(const ID type)
{
    switch (type) {
        case (ID::SHA256): {
            return proto::HASHTYPE_SHA256;
        }
        case (ID::BLAKE2B): {
            return proto::HASHTYPE_BLAKE2B160;
        }
        default: {
            return proto::HASHTYPE_NONE;
        }
    }
}

Identifier::Identifier()
    : ot_super()
{
}

Identifier::Identifier(const Identifier& theID)
    : opentxs::Data()
    , ot_super(theID)
    , type_(theID.Type())
{
}

Identifier::Identifier(const std::string& theStr)
    : ot_super()
{
    SetString(theStr);
}

Identifier::Identifier(const String& theStr)
    : ot_super()
{
    SetString(theStr);
}

Identifier::Identifier(const Contract& theContract)
    : ot_super()  // Get the contract's ID into this identifier.
{
    (const_cast<Contract&>(theContract)).GetIdentifier(*this);
}

Identifier::Identifier(const Nym& theNym)
    : ot_super()  // Get the Nym's ID into this identifier.
{
    (const_cast<Nym&>(theNym)).GetIdentifier(*this);
}

Identifier::Identifier(const OTSymmetricKey& theKey)
    : ot_super()  // Get the Symmetric Key's ID into *this. (It's a hash of the
                  // encrypted form of the symmetric key.)
{
    (const_cast<OTSymmetricKey&>(theKey)).GetIdentifier(*this);
}

Identifier::Identifier(const OTCachedKey& theKey)
    : ot_super()  // Cached Key stores a symmetric key inside, so this actually
                  // captures the ID for that symmetrickey.
{
    const bool bSuccess =
        (const_cast<OTCachedKey&>(theKey)).GetIdentifier(*this);

    OT_ASSERT(bSuccess);  // should never fail. If it does, then we are calling
    // this function at a time we shouldn't, when we aren't
    // sure the master key has even been generated yet. (If
    // this asserts, need to examine the line of code that
    // tried to do this, and figure out where its logic
    // went wrong, since it should have made sure this
    // would not happen, before constructing like this.)
}

Identifier::Identifier(
    const proto::ContactItemType type,
    const proto::HDPath& path)
    : ot_super()
{
    CalculateDigest(path_to_data(type, path), DefaultType);
}

Identifier& Identifier::operator=(const Identifier& rhs)
{
    Assign(rhs);
    type_ = rhs.type_;

    return *this;
}

Identifier& Identifier::operator=(Identifier&& rhs)
{
    swap(rhs);

    return *this;
}

bool Identifier::operator==(const Identifier& s2) const
{
    const String ots1(*this), ots2(s2);
    return ots1.Compare(ots2);
}

bool Identifier::operator!=(const Identifier& s2) const
{
    const String ots1(*this), ots2(s2);
    return !(ots1.Compare(ots2));
}

bool Identifier::operator>(const Identifier& s2) const
{
    const String ots1(*this), ots2(s2);
    return ots1.operator>(ots2);
}

bool Identifier::operator<(const Identifier& s2) const
{
    const String ots1(*this), ots2(s2);
    return ots1.operator<(ots2);
}

bool Identifier::operator<=(const Identifier& s2) const
{
    const String ots1(*this), ots2(s2);
    return ots1.operator<=(ots2);
}

bool Identifier::operator>=(const Identifier& s2) const
{
    const String ots1(*this), ots2(s2);
    return ots1.operator>=(ots2);
}

bool Identifier::CalculateDigest(const String& strInput, const ID type)
{
    type_ = type;

    return OT::App().Crypto().Hash().Digest(
        IDToHashType(type_), strInput, *this);
}

bool Identifier::CalculateDigest(const opentxs::Data& dataInput, const ID type)
{
    type_ = type;

    return OT::App().Crypto().Hash().Digest(
        IDToHashType(type_), dataInput, *this);
}

Identifier* Identifier::clone() const { return new Identifier(*this); }

// SET (binary id) FROM ENCODED STRING
void Identifier::SetString(const String& encoded)
{
    return SetString(std::string(encoded.Get()));
}

void Identifier::SetString(const std::string& encoded)
{
    empty();

    if (MinimumSize > encoded.size()) {
        return;
    }

    if ('o' != encoded.at(0)) {
        return;
    }
    if ('t' != encoded.at(1)) {
        return;
    }

    std::string input(encoded.data() + 2, encoded.size() - 2);
    auto data = OT::App().Crypto().Encode().IdentifierDecode(input);

    if (!data.empty()) {
        type_ = static_cast<ID>(data[0]);

        switch (type_) {
            case (ID::SHA256): {
                break;
            }
            case (ID::BLAKE2B): {
                break;
            }
            default: {
                type_ = ID::ERROR;

                return;
            }
        }

        Assign(
            (reinterpret_cast<const std::uint8_t*>(data.data()) + 1),
            (data.size() - 1));
    }
}

// This Identifier is stored in binary form.
// But what if you want a pretty string version of it?
// Just call this function.
void Identifier::GetString(String& id) const
{
    auto data = Data::Factory();
    data->Assign(&type_, sizeof(type_));

    OT_ASSERT(1 == data->GetSize());

    if (0 == GetSize()) {
        return;
    }

    data->Concatenate(GetPointer(), GetSize());

    String output("ot");
    output.Concatenate(
        String(OT::App().Crypto().Encode().IdentifierEncode(data).c_str()));
    id.swap(output);
}

std::string Identifier::str() const
{
    auto data = Data::Factory();
    data->Assign(&type_, sizeof(type_));

    OT_ASSERT(1 == data->GetSize());

    if (0 == GetSize()) {
        return {};
    }

    data->Concatenate(GetPointer(), GetSize());

    std::string output("ot");
    output.append(OT::App().Crypto().Encode().IdentifierEncode(data).c_str());
    return output;
}

OTData Identifier::path_to_data(
    const proto::ContactItemType type,
    const proto::HDPath& path)
{
    auto output = Data::Factory(static_cast<const void*>(&type), sizeof(type));
    output += Data::Factory(path.root().c_str(), path.root().size());

    for (const auto& child : path.child()) {
        output += Data::Factory(&child, sizeof(child));
    }

    return output;
}

void Identifier::swap(Identifier&& rhs)
{
    ot_super::swap(rhs);
    type_ = rhs.type_;
    rhs.type_ = ID::ERROR;
}
}  // namespace opentxs
