// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "opentxs/identity/Source.hpp"
#include "opentxs/identity/Types.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/util/PasswordPrompt.hpp"

namespace opentxs::identity
{
class SourceMock : public Source {
public:
    MOCK_METHOD(OTString, asString, (), (const, noexcept, override));
    MOCK_METHOD(OTString, Description, (), (const, noexcept, override));
    MOCK_METHOD(identity::SourceType, Type, (), (const, noexcept, override));
    MOCK_METHOD(OTNymID, NymID, (), (const, noexcept, override));
    MOCK_METHOD(bool, Serialize, (proto::NymIDSource& serialized), (const, noexcept, override));
    MOCK_METHOD(bool, Verify, (const proto::Credential& master,
                               const proto::Signature& sourceSignature), (const, noexcept, override));
    MOCK_METHOD(bool, Sign, (const identity::credential::Primary& credential,
                                     proto::Signature& sig,
                                     const PasswordPrompt& reason), (const, noexcept, override));
};
}