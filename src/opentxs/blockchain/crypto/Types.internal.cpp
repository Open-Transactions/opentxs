// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/blockchain/crypto/Types.internal.hpp"  // IWYU pragma: associated

#include <boost/endian/buffers.hpp>
#include <frozen/bits/algorithms.h>
#include <frozen/unordered_map.h>
#include <opentxs/protobuf/HDPath.pb.h>
#include <cstdint>
#include <sstream>
#include <string_view>
#include <utility>
#include <variant>

#include "internal/util/P0330.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/crypto/Crypto.hpp"
#include "opentxs/api/crypto/Hash.hpp"
#include "opentxs/blockchain/block/TransactionHash.hpp"
#include "opentxs/blockchain/crypto/HDProtocol.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/blockchain/token/Descriptor.hpp"
#include "opentxs/blockchain/token/Types.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/FixedByteArray.hpp"
#include "opentxs/crypto/Bip32.hpp"
#include "opentxs/crypto/Bip32Child.hpp"    // IWYU pragma: keep
#include "opentxs/crypto/Bip43Purpose.hpp"  // IWYU pragma: keep
#include "opentxs/crypto/HashType.hpp"      // IWYU pragma: keep
#include "opentxs/crypto/Types.hpp"
#include "opentxs/identifier/Generic.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Writer.hpp"
#include "util/HDIndex.hpp"

namespace opentxs
{
auto blockchain_thread_item_id(
    const api::Crypto& crypto,
    const api::Factory& factory,
    const blockchain::Type chain,
    const blockchain::block::TransactionHash& txid) noexcept
    -> identifier::Generic
{
    auto preimage = UnallocatedCString{};
    const auto hashed = crypto.Hash().HMAC(
        crypto::HashType::Sha256,
        ReadView{reinterpret_cast<const char*>(&chain), sizeof(chain)},
        txid.Bytes(),
        writer(preimage));

    assert_true(hashed);

    return factory.IdentifierFromPreimage(preimage);
}
}  // namespace opentxs

namespace opentxs::blockchain::crypto
{
auto deserialize(std::span<const network::zeromq::Frame> in) noexcept -> Target
{
    assert_true(3_uz == in.size());

    const auto& chain = in[0];
    const auto& type = in[1];
    const auto& id = in[2];
    const auto isToken = (32_uz == id.size());

    if (isToken) {

        return token::Descriptor{
            chain.as<blockchain::Type>(), type.as<token::Type>(), id.Bytes()};
    } else {

        return chain.as<blockchain::Type>();
    }
}

auto get_name(const protobuf::HDPath& path, HDProtocol type) noexcept
    -> UnallocatedCString
{
    auto out = std::stringstream{};
    out << print(type);
    out << ": ";
    out << opentxs::crypto::Print(path, false);

    return out.str();
}

auto get_standard(const protobuf::HDPath& path) noexcept -> HDProtocol
{
    using Index = opentxs::HDIndex<opentxs::crypto::Bip43Purpose>;
    using enum opentxs::crypto::Bip43Purpose;
    using enum opentxs::crypto::Bip32Child;
    using enum HDProtocol;

    // NOTE can't be constexpr because of static_cast in HDIndex
    static const auto map = frozen::make_unordered_map<Bip32Index, HDProtocol>({
        {Index{HDWALLET, HARDENED}, BIP_44},
        {Index{P2SH_P2WPKH, HARDENED}, BIP_49},
        {Index{P2WPKH, HARDENED}, BIP_84},
    });

    if (0 < path.child().size()) {
        if (const auto* i = map.find(path.child(0)); map.end() != i) {

            return i->second;
        }
    }

    return BIP_32;
}

auto serialize(const Target& target, Data& out) noexcept -> void
{
    struct Visitor {
        Data& preimage_;

        auto operator()(blockchain::Type c) noexcept -> void
        {
            const auto chain = boost::endian::little_uint32_buf_t{
                static_cast<std::uint32_t>(c)};
            preimage_.Concatenate(std::addressof(chain), sizeof(chain));
        }
        auto operator()(const token::Descriptor& token) noexcept -> void
        {
            const auto chain = boost::endian::little_uint32_buf_t{
                static_cast<std::uint32_t>(token.host_)};
            const auto type = boost::endian::little_uint32_buf_t{
                static_cast<std::uint32_t>(token.type_)};
            preimage_.Concatenate(std::addressof(chain), sizeof(chain));
            preimage_.Concatenate(std::addressof(type), sizeof(chain));
            preimage_.Concatenate(token.id_.Bytes());
        }
    };
    std::visit(Visitor{out}, target);
}

auto serialize(const Target& target, network::zeromq::Message& out) noexcept
    -> void
{
    struct Visitor {
        network::zeromq::Message& zmq_;

        auto operator()(blockchain::Type chain) noexcept -> void
        {
            zmq_.AddFrame(chain);
            zmq_.AddFrame();
            zmq_.AddFrame();
        }
        auto operator()(const token::Descriptor& token) noexcept -> void
        {
            zmq_.AddFrame(token.host_);
            zmq_.AddFrame(token.type_);
            zmq_.AddFrame(token.id_);
        }
    };
    std::visit(Visitor{out}, target);
}
}  // namespace opentxs::blockchain::crypto
