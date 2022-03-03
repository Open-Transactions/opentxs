// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"
#include "opentxs/core/Armored.hpp"
#include "opentxs/util/Pimpl.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace api
{
class Session;
}  // namespace api

class OTSignatureMetadata;
class Signature;

using OTSignature = Pimpl<Signature>;
// }  // namespace v1
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

    ~Signature() override = default;

protected:
    Signature() = default;

private:
    Signature(const Signature&) = delete;
    Signature(Signature&&) = delete;
    auto operator=(const Signature&) -> Signature& = delete;
    auto operator=(Signature&&) -> Signature& = delete;
};
}  // namespace opentxs
