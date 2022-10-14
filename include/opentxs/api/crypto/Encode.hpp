// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <string_view>

#include "opentxs/Export.hpp"
#include "opentxs/util/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
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

class Writer;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::api::crypto
{
/**
 The api::crypto::encode API is used for byte sequences to and from encoded
 strings.
 */
class Encode
{
public:
    [[nodiscard]] virtual auto Base58CheckEncode(
        ReadView input,
        Writer&& output) const noexcept -> bool = 0;
    [[nodiscard]] virtual auto Base58CheckDecode(
        std::string_view input,
        Writer&& output) const -> bool = 0;
    [[nodiscard]] virtual auto Base64Encode(ReadView input, Writer&& output)
        const noexcept -> bool = 0;
    [[nodiscard]] virtual auto Base64Decode(
        std::string_view input,
        Writer&& output) const noexcept -> bool = 0;
    OPENTXS_NO_EXPORT virtual auto InternalEncode() const noexcept
        -> const internal::Encode& = 0;
    [[nodiscard]] virtual auto Z85Encode(ReadView input, Writer&& output)
        const noexcept -> bool = 0;
    [[nodiscard]] virtual auto Z85Decode(
        std::string_view input,
        Writer&& output) const noexcept -> bool = 0;

    OPENTXS_NO_EXPORT virtual auto InternalEncode() noexcept
        -> internal::Encode& = 0;

    Encode(const Encode&) = delete;
    Encode(Encode&&) = delete;
    auto operator=(const Encode&) -> Encode& = delete;
    auto operator=(Encode&&) -> Encode& = delete;

    OPENTXS_NO_EXPORT virtual ~Encode() = default;

protected:
    Encode() = default;
};
}  // namespace opentxs::api::crypto
