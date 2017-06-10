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

#ifndef OPENTXS_CORE_CRYPTO_ASYMMETRICKEYEC_HPP
#define OPENTXS_CORE_CRYPTO_ASYMMETRICKEYEC_HPP

#include "opentxs/core/crypto/OTAsymmetricKey.hpp"

#include <memory>

namespace opentxs
{

class Ecdsa;
class Data;
class OTPassword;
class String;

class AsymmetricKeyEC : public OTAsymmetricKey
{
private:
    friend class Ecdsa;

    typedef OTAsymmetricKey ot_super;

protected:
    std::unique_ptr<Data> key_;
    std::unique_ptr<proto::Ciphertext> encrypted_key_;
    std::shared_ptr<proto::HDPath> path_;
    std::unique_ptr<proto::Ciphertext> chain_code_;

    AsymmetricKeyEC() = delete;
    explicit AsymmetricKeyEC(
        const proto::AsymmetricKeyType keyType,
        const proto::KeyRole role);
    explicit AsymmetricKeyEC(const proto::AsymmetricKey& serializedKey);
    explicit AsymmetricKeyEC(
        const proto::AsymmetricKeyType keyType,
        const String& publicKey);

    void ReleaseKeyLowLevel_Hook() const override {}

public:
    bool IsEmpty() const override;
    virtual Ecdsa& ECDSA() const = 0;
    bool GetKey(Data& key) const;
    bool GetKey(proto::Ciphertext& key) const;
    bool GetPublicKey(String& strKey) const override;
    const std::string Path() const override;
    bool ReEncryptPrivateKey(
        const OTPassword& theExportPassword,
        bool bImporting) const override;
    void Release_AsymmetricKeyEC() {}
    void Release() override;
    bool SetKey(std::unique_ptr<Data>& key);
    bool SetKey(std::unique_ptr<proto::Ciphertext>& key);
    serializedAsymmetricKey Serialize() const override;
    bool TransportKey(Data& publicKey, OTPassword& privateKey) const override;

    virtual ~AsymmetricKeyEC() = default;
};

}  // namespace opentxs

#endif  // OPENTXS_CORE_CRYPTO_ASYMMETRICKEYEC_HPP
