// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"

#include <BlockchainBlockHeader.pb.h>
#include <ContactData.pb.h>
#include <Credential.pb.h>
#include <VerificationSet.pb.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>

#include "internal/identity/Types.hpp"

namespace opentxs::identity::credential
{
class PrimaryMock : public Primary
{
public:
    // NOLINTBEGIN(modernize-use-trailing-return-type)
    MOCK_METHOD(UnallocatedCString, Path, (), (const, override));
    MOCK_METHOD(bool, Path, (proto::HDPath & output), (const, override));
    MOCK_METHOD(UnallocatedCString, Alias, (), (const, noexcept, override));
    MOCK_METHOD(CString, Alias, (alloc::Strategy), (const, noexcept, override));
    MOCK_METHOD(
        const identifier::Generic&,
        ID,
        (),
        (const, noexcept, override));
    MOCK_METHOD(std::string_view, Name, (), (const, noexcept, override));
    MOCK_METHOD(std::string_view, Terms, (), (const, noexcept, override));
    MOCK_METHOD(bool, Serialize, (Writer&&), (const, noexcept, override));
    MOCK_METHOD(Nym_p, Signer, (), (const, noexcept, override));
    MOCK_METHOD(bool, Validate, (), (const, noexcept, override));
    MOCK_METHOD(VersionNumber, Version, (), (const, noexcept, override));
    MOCK_METHOD(Base*, clone, (), (const, noexcept, override));
    MOCK_METHOD(
        UnallocatedCString,
        asString,
        (const bool asPrivate),
        (const, noexcept, override));
    MOCK_METHOD(
        const identifier::Generic&,
        CredentialID,
        (),
        (const, noexcept, override));
    MOCK_METHOD(
        bool,
        hasCapability,
        (const NymCapability& capability),
        (const, noexcept, override));
    MOCK_METHOD(
        crypto::asymmetric::Mode,
        Mode,
        (),
        (const, noexcept, override));
    MOCK_METHOD(
        identity::CredentialRole,
        Role,
        (),
        (const, noexcept, override));
    MOCK_METHOD(bool, Private, (), (const, noexcept, override));
    MOCK_METHOD(bool, Save, (), (const, noexcept, override));
    MOCK_METHOD(
        bool,
        TransportKey,
        (Data & publicKey, Secret& privateKey, const PasswordPrompt& reason),
        (const, noexcept, override));

    MOCK_METHOD(
        identity::CredentialType,
        Type,
        (),
        (const, noexcept, override));
    MOCK_METHOD(bool, SetAlias, (std::string_view alias), (noexcept, override));
    MOCK_METHOD(
        const internal::Base&,
        Internal,
        (),
        (noexcept, const, override));
    MOCK_METHOD(internal::Base&, Internal, (), (noexcept, override));
    // NOLINTEND(modernize-use-trailing-return-type)
};
}  // namespace opentxs::identity::credential
