// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <functional>
#include <stdexcept>
#include <utility>

#include "internal/network/blockchain/bitcoin/message/Factory.hpp"
#include "internal/network/blockchain/bitcoin/message/Message.hpp"
#include "internal/network/blockchain/bitcoin/message/Types.hpp"
#include "network/blockchain/bitcoin/Peer.hpp"
#include "opentxs/api/Session.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace network
{
namespace blockchain
{
namespace bitcoin
{
namespace message
{
namespace internal
{
class Addr2;
class Addr;
class Block;
class Cfheaders;
class Cfilter;
class Getaddr;
class Getcfheaders;
class Getcfilters;
class Getdata;
class Getheaders;
class Headers;
class Inv;
class Mempool;
class Notfound;
class Ping;
class Pong;
class Sendaddr2;
class Tx;
class Verack;
class Version;
}  // namespace internal
}  // namespace message
}  // namespace bitcoin
}  // namespace blockchain
}  // namespace network
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::network::blockchain::bitcoin
{
using enum message::Command;

template <>
struct Peer::ToWire<message::internal::Addr> {
    static auto Name() noexcept { return print(addr); }

    template <typename... Args>
    auto operator()(
        const api::Session& api,
        opentxs::blockchain::Type chain,
        Args&&... args) const
    {
        return factory::BitcoinP2PAddr(api, chain, std::forward<Args>(args)...);
    }
};
template <>
struct Peer::ToWire<message::internal::Addr2> {
    static auto Name() noexcept { return print(addr2); }

    template <typename... Args>
    auto operator()(
        const api::Session& api,
        opentxs::blockchain::Type chain,
        Args&&... args) const
    {
        return factory::BitcoinP2PAddr2(
            api, chain, std::forward<Args>(args)...);
    }
};
template <>
struct Peer::ToWire<message::internal::Block> {
    static auto Name() noexcept { return print(block); }

    template <typename... Args>
    auto operator()(
        const api::Session& api,
        opentxs::blockchain::Type chain,
        Args&&... args) const
    {
        return factory::BitcoinP2PBlock(
            api, chain, std::forward<Args>(args)...);
    }
};
template <>
struct Peer::ToWire<message::internal::Cfheaders> {
    static auto Name() noexcept { return print(cfheaders); }

    template <typename... Args>
    auto operator()(
        const api::Session& api,
        opentxs::blockchain::Type chain,
        Args&&... args) const
    {
        return factory::BitcoinP2PCfheaders(
            api, chain, std::forward<Args>(args)...);
    }
};
template <>
struct Peer::ToWire<message::internal::Cfilter> {
    static auto Name() noexcept { return print(cfilter); }

    template <typename... Args>
    auto operator()(
        const api::Session& api,
        opentxs::blockchain::Type chain,
        Args&&... args) const
    {
        return factory::BitcoinP2PCfilter(
            api, chain, std::forward<Args>(args)...);
    }
};
template <>
struct Peer::ToWire<message::internal::Getaddr> {
    static auto Name() noexcept { return print(getaddr); }

    template <typename... Args>
    auto operator()(
        const api::Session& api,
        opentxs::blockchain::Type chain,
        Args&&... args) const
    {
        return factory::BitcoinP2PGetaddr(
            api, chain, std::forward<Args>(args)...);
    }
};
template <>
struct Peer::ToWire<message::internal::Getcfheaders> {
    static auto Name() noexcept { return print(getcfheaders); }

    template <typename... Args>
    auto operator()(
        const api::Session& api,
        opentxs::blockchain::Type chain,
        Args&&... args) const
    {
        return factory::BitcoinP2PGetcfheaders(
            api, chain, std::forward<Args>(args)...);
    }
};
template <>
struct Peer::ToWire<message::internal::Getcfilters> {
    static auto Name() noexcept { return print(getcfilters); }

    template <typename... Args>
    auto operator()(
        const api::Session& api,
        opentxs::blockchain::Type chain,
        Args&&... args) const
    {
        return factory::BitcoinP2PGetcfilters(
            api, chain, std::forward<Args>(args)...);
    }
};
template <>
struct Peer::ToWire<message::internal::Getdata> {
    static auto Name() noexcept { return print(getdata); }

    template <typename... Args>
    auto operator()(
        const api::Session& api,
        opentxs::blockchain::Type chain,
        Args&&... args) const
    {
        return factory::BitcoinP2PGetdata(
            api, chain, std::forward<Args>(args)...);
    }
};
template <>
struct Peer::ToWire<message::internal::Getheaders> {
    static auto Name() noexcept { return print(getheaders); }

    template <typename... Args>
    auto operator()(
        const api::Session& api,
        opentxs::blockchain::Type chain,
        Args&&... args) const
    {
        return factory::BitcoinP2PGetheaders(
            api, chain, std::forward<Args>(args)...);
    }
};
template <>
struct Peer::ToWire<message::internal::Headers> {
    static auto Name() noexcept { return print(headers); }

    template <typename... Args>
    auto operator()(
        const api::Session& api,
        opentxs::blockchain::Type chain,
        Args&&... args) const
    {
        return factory::BitcoinP2PHeaders(
            api, chain, std::forward<Args>(args)...);
    }
};
template <>
struct Peer::ToWire<message::internal::Inv> {
    static auto Name() noexcept { return print(inv); }

    template <typename... Args>
    auto operator()(
        const api::Session& api,
        opentxs::blockchain::Type chain,
        Args&&... args) const
    {
        return factory::BitcoinP2PInv(api, chain, std::forward<Args>(args)...);
    }
};
template <>
struct Peer::ToWire<message::internal::Mempool> {
    static auto Name() noexcept { return print(mempool); }

    template <typename... Args>
    auto operator()(
        const api::Session& api,
        opentxs::blockchain::Type chain,
        Args&&... args) const
    {
        return factory::BitcoinP2PMempool(
            api, chain, std::forward<Args>(args)...);
    }
};
template <>
struct Peer::ToWire<message::internal::Notfound> {
    static auto Name() noexcept { return print(notfound); }

    template <typename... Args>
    auto operator()(
        const api::Session& api,
        opentxs::blockchain::Type chain,
        Args&&... args) const
    {
        return factory::BitcoinP2PNotfound(
            api, chain, std::forward<Args>(args)...);
    }
};
template <>
struct Peer::ToWire<message::internal::Ping> {
    static auto Name() noexcept { return print(ping); }

    template <typename... Args>
    auto operator()(
        const api::Session& api,
        opentxs::blockchain::Type chain,
        Args&&... args) const
    {
        return factory::BitcoinP2PPing(api, chain, std::forward<Args>(args)...);
    }
};
template <>
struct Peer::ToWire<message::internal::Pong> {
    static auto Name() noexcept { return print(pong); }

    template <typename... Args>
    auto operator()(
        const api::Session& api,
        opentxs::blockchain::Type chain,
        Args&&... args) const
    {
        return factory::BitcoinP2PPong(api, chain, std::forward<Args>(args)...);
    }
};
template <>
struct Peer::ToWire<message::internal::Sendaddr2> {
    static auto Name() noexcept { return print(sendaddr2); }

    template <typename... Args>
    auto operator()(
        const api::Session& api,
        opentxs::blockchain::Type chain,
        Args&&... args) const
    {
        return factory::BitcoinP2PSendaddr2(
            api, chain, std::forward<Args>(args)...);
    }
};
template <>
struct Peer::ToWire<message::internal::Tx> {
    static auto Name() noexcept { return print(tx); }

    template <typename... Args>
    auto operator()(
        const api::Session& api,
        opentxs::blockchain::Type chain,
        Args&&... args) const
    {
        return factory::BitcoinP2PTx(api, chain, std::forward<Args>(args)...);
    }
};
template <>
struct Peer::ToWire<message::internal::Verack> {
    static auto Name() noexcept { return print(verack); }

    template <typename... Args>
    auto operator()(
        const api::Session& api,
        opentxs::blockchain::Type chain,
        Args&&... args) const
    {
        return factory::BitcoinP2PVerack(
            api, chain, std::forward<Args>(args)...);
    }
};
template <>
struct Peer::ToWire<message::internal::Version> {
    static auto Name() noexcept { return print(version); }

    template <typename... Args>
    auto operator()(
        const api::Session& api,
        opentxs::blockchain::Type chain,
        Args&&... args) const
    {
        return factory::BitcoinP2PVersion(
            api, chain, std::forward<Args>(args)...);
    }
};
}  // namespace opentxs::network::blockchain::bitcoin

namespace opentxs::network::blockchain::bitcoin
{
template <typename Outgoing, typename... Args>
auto Peer::transmit_protocol(allocator_type monotonic, Args&&... args) noexcept
    -> void
{
    static const auto factory = ToWire<Outgoing>{};

    try {
        const auto message = std::invoke(
            factory, api_, chain_, std::forward<Args>(args)..., monotonic);

        if (false == message.IsValid()) {
            auto error = CString{get_allocator()};
            error.append("failed to construct ");
            error.append(factory.Name());

            throw std::runtime_error{error.c_str()};
        }
        log_()(name_)(": sending ")(factory.Name()).Flush();
        transmit(message);
    } catch (const std::exception& e) {
        disconnect(e.what(), monotonic);

        return;
    }
}
}  // namespace opentxs::network::blockchain::bitcoin
