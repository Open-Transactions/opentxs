// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                           // IWYU pragma: associated
#include "core/identifier/IdentifierPrivate.hpp"  // IWYU pragma: associated

#include <Identifier.pb.h>
#include <boost/endian/buffers.hpp>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iterator>
#include <sstream>
#include <string_view>

#include "internal/core/identifier/Identifier.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/crypto/Crypto.hpp"
#include "opentxs/api/crypto/Encode.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/core/identifier/Algorithm.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Type.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::identifier
{
IdentifierPrivate::IdentifierPrivate(
    const identifier::Algorithm algorithm,
    const identifier::Type type,
    const ReadView hash,
    allocator_type alloc) noexcept
    : ByteArrayPrivate(alloc)
    , algorithm_(algorithm)
    , type_(type)
{
    Concatenate(hash);
}

auto IdentifierPrivate::asBase58(const api::Crypto& api) const
    -> UnallocatedCString
{
    return asBase58(api, {}).c_str();
}

auto IdentifierPrivate::asBase58(const api::Crypto& api, alloc::Default alloc)
    const -> CString
{
    const auto required = identifier_expected_hash_bytes(algorithm_);

    if (const auto len = size(); len != required) {
        if (0u != len) {
            LogError()(OT_PRETTY_CLASS())("Incorrect hash size (")(
                len)(") vs required (")(required)(")")
                .Flush();
        }

        return CString{alloc};
    }

    const auto preimage = [&] {
        auto out = ByteArray{alloc};
        const auto payload = size();

        if (0 == payload) { return out; }

        const auto type = boost::endian::little_uint16_buf_t{
            static_cast<std::uint16_t>(type_)};
        out.resize(sizeof(algorithm_) + sizeof(type) + payload);

        OT_ASSERT(out.size() == required + identifier_header_);

        auto* i = static_cast<std::byte*>(out.data());
        std::memcpy(i, &algorithm_, sizeof(algorithm_));
        std::advance(i, sizeof(algorithm_));
        std::memcpy(i, static_cast<const void*>(&type), sizeof(type));
        std::advance(i, sizeof(type));
        std::memcpy(i, data(), payload);
        std::advance(i, payload);

        return out;
    }();
    // TODO c++20 use allocator
    auto ss = std::stringstream{};

    if (0 < preimage.size()) {
        ss << identifier_prefix_;
        ss << api.Encode().IdentifierEncode(preimage.Bytes());
    }

    return CString{ss.str().c_str(), alloc};
}

auto IdentifierPrivate::Serialize(proto::Identifier& out) const noexcept -> bool
{
    out.set_version(proto_version_);
    static constexpr auto badAlgo = identifier::Algorithm::invalid;
    static constexpr auto badType = identifier::Type::invalid;

    if ((badAlgo == algorithm_) || (badType == type_)) {
        out.clear_hash();
        out.set_algorithm(static_cast<std::uint32_t>(badAlgo));
        out.set_type(static_cast<std::uint32_t>(badType));

        return true;
    }

    out.set_hash(UnallocatedCString{Bytes()});
    out.set_algorithm(static_cast<std::uint32_t>(algorithm_));
    out.set_type(static_cast<std::uint32_t>(type_));

    return true;
}
}  // namespace opentxs::identifier
