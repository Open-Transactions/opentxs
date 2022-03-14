// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

// IWYU pragma: begin_exports
#include "opentxs/Version.hpp"  // IWYU pragma: associated
// IWYU pragma: end_exports

constexpr auto PAYMENT_CODE_VERSION = 1;
constexpr auto PEER_MESSAGE_VERSION = 2;
constexpr auto PEER_PAYMENT_VERSION = 5;
constexpr auto PEER_CASH_VERSION = 7;
constexpr auto PEER_OBJECT_PEER_REQUEST = 7;
constexpr auto PEER_OBJECT_PEER_REPLY = 7;
constexpr auto OT_CONTACT_VERSION = 3;
constexpr auto CONTACT_CONTACT_DATA_VERSION = 6;
constexpr auto MESSAGE_SEND_ERROR = -1;
constexpr auto MESSAGE_NOT_SENT_NO_ERROR = 0;
constexpr auto MESSAGE_SENT = 1;
constexpr auto REPLY_NOT_RECEIVED = -1;
constexpr auto MESSAGE_SUCCESS_FALSE = 0;
constexpr auto MESSAGE_SUCCESS_TRUE = 1;
constexpr auto FIRST_REQUEST_NUMBER = 1;

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace amount
{
}  // namespace amount

namespace api
{
namespace crypto
{
namespace blank
{
}  // namespace blank

namespace blockchain
{
}  // namespace blockchain

namespace imp
{
}  // namespace imp

namespace internal
{
}  // namespace internal
}  // namespace crypto

namespace imp
{
}  // namespace imp

namespace internal
{
}  // namespace internal

namespace network
{
namespace asio
{
}  // namespace asio

namespace imp
{
}  // namespace imp

namespace internal
{
}  // namespace internal
}  // namespace network

namespace session
{
namespace activity
{
}  // namespace activity

namespace base
{
}  // namespace base

namespace client
{
}  // namespace client

namespace imp
{
}  // namespace imp

namespace internal
{
}  // namespace internal

namespace server
{
}  // namespace server

namespace ui
{
}  // namespace ui
}  // namespace session
}  // namespace api

namespace blank
{
}  // namespace blank

namespace blockchain
{
namespace bitcoin
{
}  // namespace bitcoin

namespace block
{
namespace bitcoin
{
namespace implementation
{
}  // namespace implementation

namespace internal
{
}  // namespace internal
}  // namespace bitcoin

namespace implementation
{
}  // namespace implementation

namespace internal
{
}  // namespace internal

namespace pkt
{
}  // namespace pkt
}  // namespace block

namespace crypto
{
namespace implementation
{
}  // namespace implementation

namespace internal
{
}  // namespace internal
}  // namespace crypto

namespace database
{
namespace common
{
}  // namespace common

namespace wallet
{
namespace db
{
}  // namespace db
}  // namespace wallet
}  // namespace database

namespace download
{
}  // namespace download

namespace implementation
{
}  // namespace implementation

namespace internal
{
}  // namespace internal

namespace node
{
namespace base
{
}  // namespace base

namespace implementation
{
}  // namespace implementation

namespace internal
{
}  // namespace internal

namespace wallet
{
}  // namespace wallet

}  // namespace node

namespace p2p
{
namespace bitcoin
{
namespace implementation
{
}  // namespace implementation

namespace message
{
namespace implementation
{
}  // namespace implementation

namespace internal
{
}  // namespace internal
}  // namespace message
}  // namespace bitcoin

namespace implementation
{
}  // namespace implementation

namespace internal
{
}  // namespace internal

namespace peer
{
}  // namespace peer
}  // namespace p2p

namespace params
{
}  // namespace params

namespace script
{
}  // namespace script
}  // namespace blockchain

namespace contract
{
namespace blank
{
}  // namespace blank

namespace implementation
{
}  // namespace implementation

namespace peer
{
namespace blank
{
}  // namespace blank

namespace implementation
{
}  // namespace implementation

namespace internal
{
}  // namespace internal

namespace reply
{
namespace blank
{
}  // namespace blank

namespace implementation
{
}  // namespace implementation
}  // namespace reply

namespace request
{
namespace blank
{
}  // namespace blank

namespace implementation
{
}  // namespace implementation
}  // namespace request
}  // namespace peer

namespace unit
{
namespace implementation
{
}  // namespace implementation
}  // namespace unit
}  // namespace contract

namespace crypto
{
namespace blank
{
}  // namespace blank

namespace implementation
{
}  // namespace implementation

namespace internal
{
}  // namespace internal

namespace key
{
namespace blank
{
}  // namespace blank

namespace implementation
{
}  // namespace implementation
}  // namespace key

namespace openssl
{
}  // namespace openssl

namespace sodium
{
}  // namespace sodium
}  // namespace crypto

namespace factory
{
}  // namespace factory

namespace gcs
{
}  // namespace gcs

namespace identity
{
namespace credential
{
namespace implementation
{
}  // namespace implementation

namespace internal
{
}  // namespace internal
}  // namespace credential

namespace implementation
{
}  // namespace implementation

namespace internal
{
}  // namespace internal

namespace wot
{
namespace verification
{
namespace implementation
{
}  // namespace implementation

namespace internal
{
}  // namespace internal
}  // namespace verification
}  // namespace wot
}  // namespace identity

namespace implementation
{
}  // namespace implementation

namespace internal
{
}  // namespace internal

namespace netwok
{
namespace implementation
{
}  // namespace implementation

namespace p2p
{
namespace client
{
}  // namespace client
}  // namespace p2p

namespace zeromq
{
namespace blank
{
}  // namespace blank

namespace context
{
}  // namespace context

namespace curve
{
namespace implementation
{
}  // namespace implementation
}  // namespace curve

namespace implementation
{
}  // namespace implementation

namespace internal
{
}  // namespace internal

namespace socket
{
namespace implementation
{
}  // namespace implementation

namespace internal
{
}  // namespace internal
}  // namespace socket

namespace zap
{
namespace implementation
{
}  // namespace implementation
}  // namespace zap
}  // namespace zeromq
}  // namespace netwok

namespace OTDB
{
}  // namespace OTDB

namespace otx
{
namespace blind
{
namespace internal
{
}  // namespace internal

namespace mint
{
}  // namespace mint

namespace purse
{
}  // namespace purse

namespace token
{
}  // namespace token

}  // namespace blind

namespace client
{
namespace implementation
{
}  // namespace implementation

namespace internal
{
}  // namespace internal
}  // namespace client

namespace context
{
namespace implementation
{
}  // namespace implementation

namespace internal
{
}  // namespace internal
}  // namespace context

namespace implementation
{
}  // namespace implementation

namespace internal
{
}  // namespace internal
}  // namespace otx

namespace paymentcode
{
}  // namespace paymentcode

namespace peer
{
namespace implementation
{
}  // namespace implementation
}  // namespace peer

namespace proto
{
}  // namespace proto

namespace rpc
{
namespace implementation
{
}  // namespace implementation

namespace internal
{
}  // namespace internal

namespace request
{
namespace implementation
{
}  // namespace implementation
}  // namespace request

namespace response
{
namespace implementation
{
}  // namespace implementation
}  // namespace response
}  // namespace rpc

namespace server
{
}  // namespace server

namespace storage
{
namespace driver
{
namespace filesystem
{
}  // namespace filesystem

namespace internal
{
}  // namespace internal
}  // namespace driver

namespace implementation
{
}  // namespace implementation

namespace lmdb
{
}  // namespace lmdb
}  // namespace storage

namespace ui
{
namespace identitymanager
{
}  // namespace identitymanager

namespace implementation
{
}  // namespace implementation

namespace internal
{
namespace blank
{
}  // namespace blank
}  // namespace internal

namespace qt
{
namespace internal
{
}  // namespace internal
}  // namespace qt
}  // namespace ui

namespace util
{
}  // namespace util
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)
