// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>
#include <string_view>

#include "internal/core/String.hpp"
#include "opentxs/api/crypto/Encode.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
class Data;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::api::crypto::internal
{
class Encode : virtual public api::crypto::Encode
{
public:
    auto InternalEncode() const noexcept -> const Encode& final
    {
        return *this;
    }
    virtual auto IsBase64(std::string_view str) const noexcept -> bool = 0;
    virtual auto Nonce(const std::uint32_t size) const -> OTString = 0;
    virtual auto Nonce(const std::uint32_t size, Data& rawOutput) const
        -> OTString = 0;
    virtual auto RandomFilename() const -> UnallocatedCString = 0;
    virtual auto SanatizeBase58(std::string_view input) const
        -> UnallocatedCString = 0;
    virtual auto SanatizeBase64(std::string_view input) const
        -> UnallocatedCString = 0;

    auto InternalEncode() noexcept -> Encode& final { return *this; }

    ~Encode() override = default;
};
}  // namespace opentxs::api::crypto::internal
