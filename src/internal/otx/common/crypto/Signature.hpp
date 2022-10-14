// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "internal/core/Armored.hpp"
#include "internal/util/Pimpl.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

class OTSignatureMetadata;
class Signature;

using OTSignature = Pimpl<Signature>;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs
{
class Signature : virtual public Armored
{
public:
    static auto Factory(const api::Session& api) -> Pimpl<opentxs::Signature>;

    virtual auto getMetaData() const -> const OTSignatureMetadata& = 0;

    virtual auto getMetaData() -> OTSignatureMetadata& = 0;

    Signature(const Signature&) = delete;
    Signature(Signature&&) = delete;
    auto operator=(const Signature&) -> Signature& = delete;
    auto operator=(Signature&&) -> Signature& = delete;

    ~Signature() override = default;

protected:
    Signature() = default;
};
}  // namespace opentxs
