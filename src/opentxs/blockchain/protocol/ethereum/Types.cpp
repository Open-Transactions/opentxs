// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/blockchain/protocol/ethereum/Types.hpp"  // IWYU pragma: associated

#include <boost/multiprecision/cpp_int.hpp>
#include <iterator>
#include <limits>
#include <memory>
#include <utility>

#include "core/Amount.hpp"
#include "internal/core/Amount.hpp"
#include "internal/util/Bytes.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Writer.hpp"

namespace opentxs::blockchain::protocol::ethereum
{
auto amount_to_native(const Amount& in) noexcept -> UnallocatedCString
{
    auto out = UnallocatedCString{};

    if (amount_to_native(in, writer(out))) {

        return out;
    } else {

        return {};
    }
}
auto amount_to_native(const Amount& in, alloc::Strategy alloc) noexcept
    -> CString
{
    auto out = CString{alloc.result_};

    if (amount_to_native(in, writer(out))) {

        return out;
    } else {

        return {};
    }
}

auto amount_to_native(const Amount& in, Writer&& out) noexcept -> bool
{
    return in.Internal().SerializeEthereum(std::move(out));
}

auto native_to_amount(std::string_view hex) noexcept -> std::optional<Amount>
{
    remove_hex_prefix(hex);
    const auto hexSize = [&] {
        auto out = hex.size();

        return out + (out % 2_uz);
    }();
    const auto decodedSize = hexSize / 2_uz;
    static constexpr auto maxSize = amount::nominal_integer_bits_ /
                                    std::numeric_limits<unsigned char>::digits;

    if (decodedSize > maxSize) { return std::nullopt; }

    auto bytes = ByteArray{};

    if (false == bytes.resize(decodedSize)) { return std::nullopt; }

    if (false == decode_hex(hex, bytes.get())) { return std::nullopt; }

    using boost::multiprecision::cpp_int;

    auto value = amount::Integer{};
    const auto* start = static_cast<const unsigned char*>(bytes.data());
    const auto* end = std::next(start, bytes.size());
    import_bits(value, start, end, 8, true);

    return Amount{std::make_unique<Amount::Imp>(Amount::Imp::shift_left(value))
                      .release()};
}
}  // namespace opentxs::blockchain::protocol::ethereum
