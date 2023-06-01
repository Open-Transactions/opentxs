// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "otx/common/crypto/Signature.hpp"  // IWYU pragma: associated

#include "core/Armored.hpp"
#include "internal/otx/common/crypto/Signature.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Session.hpp"

namespace opentxs
{
auto Signature::Factory(const api::Session& api) -> OTSignature
{
    return OTSignature(new implementation::Signature(api));
}
}  // namespace opentxs

namespace opentxs::implementation
{
Signature::Signature(const api::Session& api)
    : opentxs::Signature()
    , Armored(api.Crypto())
    , metadata_(api)
{
}
}  // namespace opentxs::implementation
