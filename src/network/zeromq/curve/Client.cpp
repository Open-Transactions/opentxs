// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/core/contract/ServerContract.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"

#include <zmq.h>

#include <array>

#include "Client.hpp"

#define OT_METHOD "opentxs::network::zeromq::curve::implementation::Client::"

namespace opentxs::network::zeromq
{
std::pair<std::string, std::string> CurveClient::RandomKeypair()
{
    std::pair<std::string, std::string> output{};
    auto& [privKey, pubKey] = output;

    std::array<char, CURVE_KEY_Z85_BYTES + 1> secretKey{};
    std::array<char, CURVE_KEY_Z85_BYTES + 1> publicKey{};
    auto* privkey = &secretKey[0];
    auto* pubkey = &publicKey[0];
    auto set = zmq_curve_keypair(pubkey, privkey);

    if (0 == set) {
        privKey.assign(secretKey.data(), secretKey.size());
        pubKey.assign(publicKey.data(), publicKey.size());
    } else {
        otErr << OT_METHOD << __FUNCTION__ << ": Failed to generate keypair."
              << std::endl;
    }

    return output;
}
}  // namespace opentxs::network::zeromq

namespace opentxs::network::zeromq::curve::implementation
{
Client::Client(socket::implementation::Socket& socket)
    : parent_(socket)
{
}

bool Client::SetKeysZ85(
    const std::string& serverPublic,
    const std::string& clientPrivate,
    const std::string& clientPublic) const
{
    if (CURVE_KEY_Z85_BYTES > serverPublic.size()) {
        otErr << OT_METHOD << __FUNCTION__ << ": Invalid server key size ("
              << serverPublic.size() << ")" << std::endl;

        return false;
    }

    std::array<std::uint8_t, CURVE_KEY_BYTES> key{};
    ::zmq_z85_decode(key.data(), serverPublic.data());

    if (false == set_remote_key(key.data(), key.size())) {
        otErr << OT_METHOD << __FUNCTION__ << ": Failed to set server key."
              << std::endl;

        return false;
    }

    return set_local_keys(clientPrivate, clientPublic);
}

bool Client::SetServerPubkey(const ServerContract& contract) const
{
    return set_public_key(contract);
}

bool Client::SetServerPubkey(const Data& key) const
{
    return set_public_key(key);
}

bool Client::set_public_key(const ServerContract& contract) const
{
    const auto& key = contract.TransportKey();

    if (CURVE_KEY_BYTES != key.GetSize()) {
        otErr << OT_METHOD << __FUNCTION__ << ": Invalid server key."
              << std::endl;

        return false;
    }

    return set_public_key(key);
}

bool Client::set_public_key(const Data& key) const
{
    if (false == set_remote_key(key.data(), key.size())) { return false; }

    return set_local_keys();
}

bool Client::set_local_keys() const
{
    OT_ASSERT(nullptr != parent_);

    const auto [secretKey, publicKey] = RandomKeypair();

    if (secretKey.empty() || publicKey.empty()) {
        otErr << OT_METHOD << __FUNCTION__ << ": Failed to generate keypair."
              << std::endl;

        return false;
    }

    return set_local_keys(secretKey, publicKey);
}

bool Client::set_local_keys(
    const std::string& privateKey,
    const std::string& publicKey) const
{
    OT_ASSERT(nullptr != parent_);

    if (CURVE_KEY_Z85_BYTES > privateKey.size()) {
        otErr << OT_METHOD << __FUNCTION__ << ": Invalid private key size ("
              << privateKey.size() << ")" << std::endl;

        return false;
    }

    std::array<std::uint8_t, CURVE_KEY_BYTES> privateDecoded{};
    ::zmq_z85_decode(privateDecoded.data(), privateKey.data());

    if (CURVE_KEY_Z85_BYTES > publicKey.size()) {
        otErr << OT_METHOD << __FUNCTION__ << ": Invalid public key size ("
              << publicKey.size() << ")" << std::endl;

        return false;
    }

    std::array<std::uint8_t, CURVE_KEY_BYTES> publicDecoded{};
    ::zmq_z85_decode(publicDecoded.data(), publicKey.data());

    return set_local_keys(
        privateDecoded.data(),
        privateDecoded.size(),
        publicDecoded.data(),
        publicDecoded.size());
}

bool Client::set_local_keys(
    const void* privateKey,
    const std::size_t privateKeySize,
    const void* publicKey,
    const std::size_t publicKeySize) const
{
    OT_ASSERT(nullptr != parent_);

    socket::implementation::Socket::SocketCallback cb{[&](const Lock&) -> bool {
        auto set = zmq_setsockopt(
            parent_, ZMQ_CURVE_SECRETKEY, privateKey, privateKeySize);

        if (0 != set) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to set private key.")
                .Flush();

            return false;
        }

        set = zmq_setsockopt(
            parent_, ZMQ_CURVE_PUBLICKEY, publicKey, publicKeySize);

        if (0 != set) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to set public key.")
                .Flush();

            return false;
        }

        return true;
    }};

    return parent_.apply_socket(std::move(cb));
}

bool Client::set_remote_key(const void* key, const std::size_t size) const
{
    OT_ASSERT(nullptr != parent_);

    socket::implementation::Socket::SocketCallback cb{[&](const Lock&) -> bool {
        const auto set =
            zmq_setsockopt(parent_, ZMQ_CURVE_SERVERKEY, key, size);

        if (0 != set) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to set server key.")
                .Flush();

            return false;
        }

        return true;
    }};

    return parent_.apply_socket(std::move(cb));
}
}  // namespace opentxs::network::zeromq::curve::implementation
