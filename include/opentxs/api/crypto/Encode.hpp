// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <cstdint>

#include "opentxs/core/Data.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace api
{
namespace crypto
{
namespace internal
{
class Encode;
}  // namespace internal
}  // namespace crypto
}  // namespace api
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::api::crypto
{
class Encode
{
public:
    virtual auto DataEncode(const UnallocatedCString& input) const
        -> UnallocatedCString = 0;
    virtual auto DataEncode(const Data& input) const -> UnallocatedCString = 0;
    virtual auto DataDecode(const UnallocatedCString& input) const
        -> UnallocatedCString = 0;
    virtual auto IdentifierEncode(const Data& input) const
        -> UnallocatedCString = 0;
    virtual auto IdentifierDecode(const UnallocatedCString& input) const
        -> UnallocatedCString = 0;
    OPENTXS_NO_EXPORT virtual auto InternalEncode() const noexcept
        -> const internal::Encode& = 0;
    virtual auto IsBase62(const UnallocatedCString& str) const -> bool = 0;
    virtual auto Nonce(const std::uint32_t size) const -> OTString = 0;
    virtual auto Nonce(const std::uint32_t size, Data& rawOutput) const
        -> OTString = 0;
    virtual auto RandomFilename() const -> UnallocatedCString = 0;
    virtual auto SanatizeBase58(const UnallocatedCString& input) const
        -> UnallocatedCString = 0;
    virtual auto SanatizeBase64(const UnallocatedCString& input) const
        -> UnallocatedCString = 0;
    virtual auto Z85Encode(const Data& input) const -> UnallocatedCString = 0;
    virtual auto Z85Encode(const UnallocatedCString& input) const
        -> UnallocatedCString = 0;
    virtual auto Z85Decode(const Data& input) const -> OTData = 0;
    virtual auto Z85Decode(const UnallocatedCString& input) const
        -> UnallocatedCString = 0;

    OPENTXS_NO_EXPORT virtual auto InternalEncode() noexcept
        -> internal::Encode& = 0;

    OPENTXS_NO_EXPORT virtual ~Encode() = default;

protected:
    Encode() = default;

private:
    Encode(const Encode&) = delete;
    Encode(Encode&&) = delete;
    auto operator=(const Encode&) -> Encode& = delete;
    auto operator=(Encode&&) -> Encode& = delete;
};
}  // namespace opentxs::api::crypto
