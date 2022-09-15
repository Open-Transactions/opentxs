// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                         // IWYU pragma: associated
#include "1_Internal.hpp"                       // IWYU pragma: associated
#include "blockchain/bitcoin/block/Output.hpp"  // IWYU pragma: associated

#include <BlockchainTransactionOutput.pb.h>
#include <BlockchainWalletKey.pb.h>
#include <boost/container/vector.hpp>
#include <algorithm>
#include <cstddef>
#include <cstring>
#include <iosfwd>
#include <iterator>
#include <sstream>
#include <stdexcept>
#include <utility>

#include "Proto.hpp"
#include "internal/blockchain/Blockchain.hpp"
#include "internal/blockchain/bitcoin/Bitcoin.hpp"
#include "internal/blockchain/bitcoin/block/Factory.hpp"
#include "internal/core/Amount.hpp"
#include "internal/core/Factory.hpp"
#include "internal/identity/wot/claim/Types.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/crypto/Blockchain.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/bitcoin/block/Script.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/blockchain/crypto/Element.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/crypto/Subchain.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/blockchain/node/TxoState.hpp"
#include "opentxs/blockchain/node/TxoTag.hpp"
#include "opentxs/core/display/Definition.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/identity/wot/claim/Types.hpp"
#include "opentxs/network/blockchain/bitcoin/CompactSize.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::factory
{
auto BitcoinTransactionOutput(
    const api::Session& api,
    const blockchain::Type chain,
    const std::uint32_t index,
    const blockchain::Amount& value,
    std::unique_ptr<const blockchain::bitcoin::block::internal::Script> script,
    const UnallocatedSet<blockchain::crypto::Key>& keys) noexcept
    -> std::unique_ptr<blockchain::bitcoin::block::internal::Output>
{
    using ReturnType = blockchain::bitcoin::block::implementation::Output;

    try {
        auto keySet = boost::container::flat_set<blockchain::crypto::Key>{};
        std::for_each(std::begin(keys), std::end(keys), [&](const auto& key) {
            keySet.emplace(key);
        });

        return std::make_unique<ReturnType>(
            api, chain, index, value, std::move(script), std::move(keySet));
    } catch (const std::exception& e) {
        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        return {};
    }
}

auto BitcoinTransactionOutput(
    const api::Session& api,
    const blockchain::Type chain,
    const std::uint32_t index,
    const blockchain::Amount& value,
    const blockchain::bitcoin::CompactSize& cs,
    const ReadView script) noexcept
    -> std::unique_ptr<blockchain::bitcoin::block::internal::Output>
{
    using ReturnType = blockchain::bitcoin::block::implementation::Output;

    try {
        return std::make_unique<ReturnType>(
            api, chain, index, value, sizeof(value) + cs.Total(), script);
    } catch (const std::exception& e) {
        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        return {};
    }
}

auto BitcoinTransactionOutput(
    const api::Session& api,
    const blockchain::Type chain,
    const proto::BlockchainTransactionOutput& in) noexcept
    -> std::unique_ptr<blockchain::bitcoin::block::internal::Output>
{
    using ReturnType = blockchain::bitcoin::block::implementation::Output;
    using Position = opentxs::blockchain::bitcoin::block::Script::Position;

    try {
        auto value = factory::Amount(in.value());
        auto cs = blockchain::bitcoin::CompactSize(in.script().size());
        auto keys = boost::container::flat_set<blockchain::crypto::Key>{};
        auto pkh = boost::container::flat_set<blockchain::PatternID>{};
        using Payer = identifier::Generic;
        using Payee = identifier::Generic;
        using Correction = std::pair<Payer, Payee>;
        auto corrections = UnallocatedVector<Correction>{};
        const auto& blockchain = api.Crypto().Blockchain();

        for (const auto& key : in.key()) {
            const auto subchain = static_cast<blockchain::crypto::Subchain>(
                static_cast<std::uint8_t>(key.subchain()));
            auto keyid = blockchain::crypto::Key{
                api.Factory().IdentifierFromBase58(key.subaccount()),
                subchain,
                key.index()};

            if (blockchain::crypto::Subchain::Outgoing == subchain) {
                LogError()("opentxs::factory::")(__func__)(
                    ": invalid key detected in transaction")
                    .Flush();
                auto sender = blockchain.SenderContact(keyid);
                auto recipient = blockchain.RecipientContact(keyid);

                if (sender.empty() || recipient.empty()) { OT_FAIL; }

                corrections.emplace_back(
                    std::move(sender), std::move(recipient));
            } else {
                keys.emplace(std::move(keyid));
            }
        }

        for (const auto& pattern : in.pubkey_hash()) { pkh.emplace(pattern); }

        auto out = std::make_unique<ReturnType>(
            api,
            chain,
            in.version(),
            in.index(),
            value,
            factory::BitcoinScript(chain, in.script(), Position::Output),
            sizeof(value) + cs.Total(),
            std::move(keys),
            std::move(pkh),
            (in.has_script_hash()
                 ? std::make_optional<blockchain::PatternID>(in.script_hash())
                 : std::nullopt),
            in.indexed(),
            [&]() -> blockchain::block::Position {
                if (const auto& hash = in.mined_block(); 0 < hash.size()) {

                    return {in.mined_height(), hash};
                } else {

                    return {};
                }
            }(),
            static_cast<blockchain::node::TxoState>(in.state()),
            [&] {
                auto out = UnallocatedSet<blockchain::node::TxoTag>{};

                for (const auto& tag : in.tag()) {
                    out.emplace(static_cast<blockchain::node::TxoTag>(tag));
                }

                return out;
            }());

        for (const auto& payer : in.payer()) {
            if (false == payer.empty()) {
                out->SetPayer([&] {
                    auto id = identifier::Generic{};
                    id.Assign(payer.data(), payer.size());

                    return id;
                }());
            }
        }

        for (const auto& payee : in.payee()) {
            if (false == payee.empty()) {
                out->SetPayee([&] {
                    auto id = identifier::Generic{};
                    id.Assign(payee.data(), payee.size());

                    return id;
                }());
            }
        }

        for (const auto& [payer, payee] : corrections) {
            out->SetPayer(payer);
            out->SetPayee(payee);
        }

        return out;
    } catch (const std::exception& e) {
        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        return {};
    }
}
}  // namespace opentxs::factory

namespace opentxs::blockchain::bitcoin::block::implementation
{
const VersionNumber Output::default_version_{1};
const VersionNumber Output::key_version_{1};

Output::Output(
    const api::Session& api,
    const blockchain::Type chain,
    const VersionNumber version,
    const std::uint32_t index,
    const blockchain::Amount& value,
    std::unique_ptr<const internal::Script> script,
    std::optional<std::size_t> size,
    boost::container::flat_set<crypto::Key>&& keys,
    boost::container::flat_set<PatternID>&& pubkeyHashes,
    std::optional<PatternID>&& scriptHash,
    bool indexed,
    blockchain::block::Position minedPosition,
    node::TxoState state,
    UnallocatedSet<node::TxoTag> tags) noexcept(false)
    : api_(api)
    , chain_(chain)
    , serialize_version_(version)
    , index_(index)
    , value_(value)
    , script_(std::move(script))
    , pubkey_hashes_(std::move(pubkeyHashes))
    , script_hash_(std::move(scriptHash))
    , cache_(
          api,
          std::move(size),
          std::move(keys),
          std::move(minedPosition),
          state,
          std::move(tags))
{
    if (false == bool(script_)) {
        throw std::runtime_error("Invalid output script");
    }

    if (false == indexed) { index_elements(); }
}

Output::Output(
    const api::Session& api,
    const blockchain::Type chain,
    const std::uint32_t index,
    const blockchain::Amount& value,
    const std::size_t size,
    const ReadView in,
    const VersionNumber version) noexcept(false)
    : Output(
          api,
          chain,
          version,
          index,
          value,
          factory::BitcoinScript(chain, in, Script::Position::Output),
          size,
          {},
          {},
          {},
          false,
          blockchain::block::Position{},
          node::TxoState::Error,
          {})
{
}

Output::Output(
    const api::Session& api,
    const blockchain::Type chain,
    const std::uint32_t index,
    const blockchain::Amount& value,
    std::unique_ptr<const internal::Script> script,
    boost::container::flat_set<crypto::Key>&& keys,
    const VersionNumber version) noexcept(false)
    : Output(
          api,
          chain,
          version,
          index,
          value,
          std::move(script),
          {},
          std::move(keys),
          {},
          {},
          false,
          blockchain::block::Position{},
          node::TxoState::Error,
          {})
{
}

Output::Output(const Output& rhs) noexcept
    : api_(rhs.api_)
    , chain_(rhs.chain_)
    , serialize_version_(rhs.serialize_version_)
    , index_(rhs.index_)
    , value_(rhs.value_)
    , script_(rhs.script_->clone())
    , pubkey_hashes_(rhs.pubkey_hashes_)
    , script_hash_(rhs.script_hash_)
    , cache_(rhs.cache_)
{
}

auto Output::AssociatedLocalNyms(
    UnallocatedVector<identifier::Nym>& output) const noexcept -> void
{
    cache_.for_each_key([&](const auto& key) {
        const auto& owner = api_.Crypto().Blockchain().Owner(key);

        if (false == owner.empty()) { output.emplace_back(owner); }
    });
}

auto Output::AssociatedRemoteContacts(
    UnallocatedVector<identifier::Generic>& output) const noexcept -> void
{
    const auto hashes = script_->LikelyPubkeyHashes(api_);
    const auto& api = api_.Crypto().Blockchain();
    std::for_each(std::begin(hashes), std::end(hashes), [&](const auto& hash) {
        auto contacts = api.LookupContacts(hash);
        std::move(
            std::begin(contacts),
            std::end(contacts),
            std::back_inserter(output));
    });

    auto payer = cache_.payer();
    auto payee = cache_.payee();

    if (false == payee.empty()) { output.emplace_back(std::move(payee)); }
    if (false == payer.empty()) { output.emplace_back(std::move(payer)); }
}

auto Output::CalculateSize() const noexcept -> std::size_t
{
    return cache_.size([&] {
        const auto scriptCS =
            blockchain::bitcoin::CompactSize(script_->CalculateSize());

        return opentxs::internal::Amount::SerializeBitcoinSize() +
               scriptCS.Total();
    });
}

auto Output::ExtractElements(const cfilter::Type style) const noexcept
    -> Vector<Vector<std::byte>>
{
    return script_->ExtractElements(style);
}

auto Output::FindMatches(
    const blockchain::block::Txid& tx,
    const cfilter::Type type,
    const blockchain::block::ParsedPatterns& patterns,
    const Log& log) const noexcept -> blockchain::block::Matches
{
    const auto output = blockchain::block::internal::SetIntersection(
        api_, tx.Bytes(), patterns, ExtractElements(type));
    const auto& api = api_.Crypto().Blockchain();
    std::for_each(
        std::begin(output.second),
        std::end(output.second),
        [&](const auto& match) {
            const auto& [txid, element] = match;
            const auto& [index, subchainID] = element;
            const auto& [subchain, account] = subchainID;
            auto keyid = crypto::Key{account, subchain, index};
            log(OT_PRETTY_CLASS())("output ")(index_)(" of transaction ")
                .asHex(tx)(" matches ")(print(keyid))
                .Flush();

            if (crypto::Subchain::Outgoing == subchain) {
                auto sender = api.SenderContact(keyid);
                auto recipient = api.RecipientContact(keyid);

                if (sender.empty()) { OT_FAIL; }
                if (recipient.empty()) { OT_FAIL; }

                cache_.set_payer(sender);
                cache_.set_payee(recipient);
            } else {
                cache_.add(std::move(keyid));
            }
        });

    return output;
}

auto Output::GetPatterns() const noexcept -> UnallocatedVector<PatternID>
{
    return {std::begin(pubkey_hashes_), std::end(pubkey_hashes_)};
}

auto Output::index_elements() noexcept -> void
{
    auto& hashes =
        const_cast<boost::container::flat_set<PatternID>&>(pubkey_hashes_);
    const auto patterns = script_->ExtractPatterns(api_);
    LogTrace()(OT_PRETTY_CLASS())(patterns.size())(" pubkey hashes found:")
        .Flush();
    std::for_each(
        std::begin(patterns), std::end(patterns), [&](const auto& id) -> auto{
            hashes.emplace(id);
            LogTrace()("    * ")(id).Flush();
        });
    const auto scriptHash = script_->ScriptHash();

    if (scriptHash.has_value()) {
        const_cast<std::optional<PatternID>&>(script_hash_) =
            api_.Crypto().Blockchain().IndexItem(scriptHash.value());
    }
}

auto Output::MergeMetadata(const internal::Output& rhs, const Log& log) noexcept
    -> bool
{
    return cache_.merge(rhs, index_, log);
}

auto Output::NetBalanceChange(const identifier::Nym& nym, const Log& log)
    const noexcept -> opentxs::Amount
{
    auto done{false};
    auto output = opentxs::Amount{0};
    cache_.for_each_key([&](const auto& key) {
        if (done) { return; }

        if (nym == api_.Crypto().Blockchain().Owner(key)) {
            done = true;
            output = value_;
        }
    });

    if (done) {
        log(OT_PRETTY_CLASS())("output ")(index_)(" contributes ")(value_)
            .Flush();
    } else {
        log(OT_PRETTY_CLASS())("output ")(index_)(" contributes 0").Flush();
    }

    return output;
}

auto Output::Note() const noexcept -> UnallocatedCString
{
    auto done{false};
    auto output = UnallocatedCString{};
    const auto& api = api_.Crypto().Blockchain();
    cache_.for_each_key([&](const auto& key) {
        if (done) { return; }

        try {
            const auto& element = api.GetKey(key);
            const auto note = element.Label();

            if (false == note.empty()) {
                done = true;
                output = note;
            }
        } catch (...) {
        }
    });

    return output;
}

auto Output::Print() const noexcept -> UnallocatedCString
{
    const auto& definition = blockchain::GetDefinition(chain_);

    auto out = std::stringstream{};
    out << "    value: " << definition.Format(value_) << '\n';
    out << "    script: " << '\n';
    out << script_->Print();

    return out.str();
}

auto Output::Script() const noexcept -> const internal::Script&
{
    return *script_;
}

auto Output::Serialize(const AllocateOutput destination) const noexcept
    -> std::optional<std::size_t>
{
    if (!destination) {
        LogError()(OT_PRETTY_CLASS())("Invalid output allocator").Flush();

        return std::nullopt;
    }

    const auto size = CalculateSize();
    auto output = destination(size);

    if (false == output.valid(size)) {
        LogError()(OT_PRETTY_CLASS())("Failed to allocate output bytes")
            .Flush();

        return std::nullopt;
    }

    const auto scriptCS =
        blockchain::bitcoin::CompactSize(script_->CalculateSize());
    const auto csData = scriptCS.Encode();
    auto* it = static_cast<std::byte*>(output.data());
    value_.Internal().SerializeBitcoin(destination);
    std::advance(it, opentxs::internal::Amount::SerializeBitcoinSize());
    std::memcpy(static_cast<void*>(it), csData.data(), csData.size());
    std::advance(it, csData.size());

    if (script_->Serialize(preallocated(scriptCS.Value(), it))) {

        return size;
    } else {
        LogError()(OT_PRETTY_CLASS())("Failed to serialize script").Flush();

        return std::nullopt;
    }
}

auto Output::Serialize(SerializeType& out) const noexcept -> bool
{
    out.set_version(std::max(default_version_, serialize_version_));
    out.set_index(index_);
    value_.Serialize(writer(out.mutable_value()));

    if (false == script_->Serialize(writer(*out.mutable_script()))) {
        return false;
    }

    const auto& api = api_.Crypto().Blockchain();
    cache_.for_each_key([&](const auto& key) {
        const auto& [accountID, subchain, index] = key;
        auto& serializedKey = *out.add_key();
        serializedKey.set_version(key_version_);
        serializedKey.set_chain(
            translate(UnitToClaim(BlockchainToUnit(chain_))));
        serializedKey.set_nym(api.Owner(key).asBase58(api_.Crypto()));
        serializedKey.set_subaccount(accountID.asBase58(api_.Crypto()));
        serializedKey.set_subchain(static_cast<std::uint32_t>(subchain));
        serializedKey.set_index(index);
    });

    for (const auto& id : pubkey_hashes_) { out.add_pubkey_hash(id); }

    if (script_hash_.has_value()) { out.set_script_hash(script_hash_.value()); }

    out.set_indexed(true);

    if (const auto payer = cache_.payer(); false == payer.empty()) {
        out.add_payer(UnallocatedCString{payer.Bytes()});
    }

    if (const auto payee = cache_.payee(); false == payee.empty()) {
        out.add_payee(UnallocatedCString{payee.Bytes()});
    }

    if (const auto& [height, hash] = cache_.position(); 0 <= height) {
        out.set_mined_height(height);
        out.set_mined_block(hash.data(), hash.size());
    }

    if (auto state = cache_.state(); node::TxoState::Error != state) {
        out.set_state(static_cast<std::uint32_t>(state));
    }

    for (const auto tag : cache_.tags()) {
        out.add_tag(static_cast<std::uint32_t>(tag));
    }

    return true;
}
}  // namespace opentxs::blockchain::bitcoin::block::implementation
