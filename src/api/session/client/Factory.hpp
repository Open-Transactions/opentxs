// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <ContactEnums.pb.h>
#include <Enums.pb.h>
#include <PeerEnums.pb.h>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <string_view>

#include "api/session/Factory.hpp"
#include "internal/otx/common/Item.hpp"
#include "internal/otx/common/Ledger.hpp"
#include "internal/otx/common/OTTransaction.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/crypto/Symmetric.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/blockchain/p2p/Address.hpp"
#include "opentxs/core/Armored.hpp"
#include "opentxs/core/PaymentCode.hpp"
#include "opentxs/core/Secret.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/core/contract/BasketContract.hpp"
#include "opentxs/core/contract/CurrencyContract.hpp"
#include "opentxs/core/contract/SecurityContract.hpp"
#include "opentxs/core/contract/ServerContract.hpp"
#include "opentxs/core/contract/Unit.hpp"
#include "opentxs/core/contract/peer/BailmentNotice.hpp"
#include "opentxs/core/contract/peer/BailmentReply.hpp"
#include "opentxs/core/contract/peer/BailmentRequest.hpp"
#include "opentxs/core/contract/peer/ConnectionReply.hpp"
#include "opentxs/core/contract/peer/ConnectionRequest.hpp"
#include "opentxs/core/contract/peer/NoticeAcknowledgement.hpp"
#include "opentxs/core/contract/peer/OutBailmentReply.hpp"
#include "opentxs/core/contract/peer/OutBailmentRequest.hpp"
#include "opentxs/core/contract/peer/PeerReply.hpp"
#include "opentxs/core/contract/peer/PeerRequest.hpp"
#include "opentxs/core/contract/peer/StoreSecret.hpp"
#include "opentxs/crypto/Envelope.hpp"
#include "opentxs/crypto/key/Asymmetric.hpp"
#include "opentxs/crypto/key/Keypair.hpp"
#include "opentxs/crypto/key/Symmetric.hpp"
#include "opentxs/identity/Types.hpp"
#include "opentxs/network/zeromq/Pipeline.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"
#include "opentxs/util/PasswordPrompt.hpp"
#include "opentxs/util/Time.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace session
{
class Client;
}  // namespace session
}  // namespace api

namespace blockchain
{
namespace bitcoin
{
namespace block
{
namespace internal
{
class Input;
class Output;
}  // namespace internal

class Block;
class Transaction;
}  // namespace block
}  // namespace bitcoin

namespace block
{
class Block;
class Hash;
class Header;
}  // namespace block
}  // namespace blockchain

namespace otx
{
namespace blind
{
class Purse;
}  // namespace blind
}  // namespace otx

namespace proto
{
class BlockchainBlockHeader;
class PeerObject;
}  // namespace proto

class Armored;
class Data;
class PasswordPrompt;
class PeerObject;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::api::session::client
{
class Factory final : public session::imp::Factory
{
public:
    auto BitcoinBlock(
        const opentxs::blockchain::Type chain,
        const ReadView bytes) const noexcept
        -> std::shared_ptr<
            const opentxs::blockchain::bitcoin::block::Block> final;
    auto BitcoinBlock(
        const opentxs::blockchain::block::Header& previous,
        const Transaction_p generationTransaction,
        const std::uint32_t nBits,
        const UnallocatedVector<Transaction_p>& extraTransactions,
        const std::int32_t version,
        const AbortFunction abort) const noexcept
        -> std::shared_ptr<
            const opentxs::blockchain::bitcoin::block::Block> final;
    auto BitcoinGenerationTransaction(
        const opentxs::blockchain::Type chain,
        const opentxs::blockchain::block::Height height,
        UnallocatedVector<blockchain::OutputBuilder>&& outputs,
        const UnallocatedCString& coinbase,
        const std::int32_t version) const noexcept -> Transaction_p final;
    auto BitcoinTransaction(
        const opentxs::blockchain::Type chain,
        const ReadView bytes,
        const bool isGeneration,
        const Time& time) const noexcept
        -> std::unique_ptr<
            const opentxs::blockchain::bitcoin::block::Transaction> final;
    auto BlockHeader(const proto::BlockchainBlockHeader& serialized) const
        -> BlockHeaderP final;
    auto BlockHeader(const ReadView protobuf) const -> BlockHeaderP final;
    auto BlockHeader(
        const opentxs::blockchain::Type type,
        const ReadView native) const -> BlockHeaderP final;
    auto BlockHeader(const opentxs::blockchain::block::Block& block) const
        -> BlockHeaderP final;
    auto BlockHeaderForUnitTests(
        const opentxs::blockchain::block::Hash& hash,
        const opentxs::blockchain::block::Hash& parent,
        const opentxs::blockchain::block::Height height) const
        -> BlockHeaderP final;
    auto PeerObject(const Nym_p& senderNym, const UnallocatedCString& message)
        const -> std::unique_ptr<opentxs::PeerObject> final;
    auto PeerObject(
        const Nym_p& senderNym,
        const UnallocatedCString& payment,
        const bool isPayment) const
        -> std::unique_ptr<opentxs::PeerObject> final;
    auto PeerObject(const Nym_p& senderNym, otx::blind::Purse&&) const
        -> std::unique_ptr<opentxs::PeerObject> final;
    auto PeerObject(
        const OTPeerRequest request,
        const OTPeerReply reply,
        const VersionNumber version) const
        -> std::unique_ptr<opentxs::PeerObject> final;
    auto PeerObject(const OTPeerRequest request, const VersionNumber version)
        const -> std::unique_ptr<opentxs::PeerObject> final;
    auto PeerObject(const Nym_p& signerNym, const proto::PeerObject& serialized)
        const -> std::unique_ptr<opentxs::PeerObject> final;
    auto PeerObject(
        const Nym_p& recipientNym,
        const opentxs::Armored& encrypted,
        const opentxs::PasswordPrompt& reason) const
        -> std::unique_ptr<opentxs::PeerObject> final;

    Factory(const api::session::Client& parent);
    Factory() = delete;
    Factory(const Factory&) = delete;
    Factory(Factory&&) = delete;
    auto operator=(const Factory&) -> Factory& = delete;
    auto operator=(Factory&&) -> Factory& = delete;

    ~Factory() final = default;

private:
    const api::session::Client& client_;
};
}  // namespace opentxs::api::session::client
