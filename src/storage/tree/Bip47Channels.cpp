/************************************************************
 *
 *                 OPEN TRANSACTIONS
 *
 *       Financial Cryptography and Digital Cash
 *       Library, Protocol, API, Unit, CLI, GUI
 *
 *       -- Anonymous Numbered Accounts.
 *       -- Untraceable Digital Cash.
 *       -- Triple-Signed Receipts.
 *       -- Cheques, Vouchers, Transfers, Inboxes.
 *       -- Basket Currencies, Markets, Payment Plans.
 *       -- Signed, XML, Ricardian-style Contracts.
 *       -- Scripted smart contracts.
 *
 *  EMAIL:
 *  fellowtraveler@opentransactions.org
 *
 *  WEBSITE:
 *  http://www.opentransactions.org/
 *
 *  -----------------------------------------------------
 *
 *   LICENSE:
 *   This Source Code Form is subject to the terms of the
 *   Mozilla Public License, v. 2.0. If a copy of the MPL
 *   was not distributed with this file, You can obtain one
 *   at http://mozilla.org/MPL/2.0/.
 *
 *   DISCLAIMER:
 *   This program is distributed in the hope that it will
 *   be useful, but WITHOUT ANY WARRANTY; without even the
 *   implied warranty of MERCHANTABILITY or FITNESS FOR A
 *   PARTICULAR PURPOSE.  See the Mozilla Public License
 *   for more details.
 *
 ************************************************************/

#include "stdafx.hpp"

#include "Bip47Channels.hpp"

#include "opentxs/core/Data.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/Types.hpp"

#include "storage/Plugin.hpp"

#define CHANNEL_VERSION 1
#define ADDRESS_INDEX_VERSION 1
#define CHANNEL_INDEX_VERSION 1

//#define OT_METHOD "opentxs::storage::Bip47Channels::"

namespace opentxs::storage
{
Bip47Channels::Bip47Channels(
    const opentxs::api::storage::Driver& storage,
    const std::string& hash)
    : Node(storage, hash)
    , index_lock_{}
    , address_index_{}
    , address_reverse_index_{}
    , chain_map_{}
    , contact_map_{}
    , local_map_{}
    , remote_map_{}
    , channel_data_{}
{
    if (check_hash(hash)) {
        init(hash);
    } else {
        version_ = CHANNEL_VERSION;
        root_ = Node::BLANK_HASH;
    }
}

OTIdentifier Bip47Channels::AddressToChannel(const std::string& address) const
{
    sLock lock(index_lock_);

    try {
        return address_reverse_index_.at(address);
    } catch (...) {

        return Identifier::Factory();
    }
}

void Bip47Channels::calculate_id(
    const proto::Bip47Channel& data,
    Identifier& channelID)
{
    auto preimage = Data::Factory(
        data.localpaymentcode().data(), data.localpaymentcode().size());
    const int type = data.chain();
    preimage += Data::Factory(&type, sizeof(type));
    preimage += Data::Factory(
        data.remotepaymentcode().data(), data.remotepaymentcode().size());
    channelID.Release();
    channelID.CalculateDigest(preimage);
}

proto::ContactItemType Bip47Channels::Chain(const Identifier& channelID) const
{
    sLock lock(index_lock_);

    return std::get<1>(get_channel_data(lock, channelID));
}

Bip47Channels::ChannelList Bip47Channels::ChannelsByContact(
    const Identifier& contactID) const
{
    return extract_set(contactID, contact_map_);
}

Bip47Channels::ChannelList Bip47Channels::ChannelsByChain(
    const proto::ContactItemType chain) const
{
    return extract_set(chain, chain_map_);
}

Bip47Channels::ChannelList Bip47Channels::ChannelsByLocalPaymentCode(
    const std::string& code) const
{
    return extract_set(code, local_map_);
}

Bip47Channels::ChannelList Bip47Channels::ChannelsByRemotePaymentCode(
    const std::string& code) const
{
    return extract_set(code, remote_map_);
}

OTIdentifier Bip47Channels::Contact(const Identifier& channelID) const
{
    sLock lock(index_lock_);

    return std::get<2>(get_channel_data(lock, channelID));
}

bool Bip47Channels::Delete(const std::string& id) { return delete_item(id); }

void Bip47Channels::extract_addresses(
    const proto::Bip47Direction& input,
    std::set<std::string>& output)
{
    for (const auto& address : input.address()) {
        output.emplace(address.address());
    }
}

template <typename I, typename V>
typename V::mapped_type Bip47Channels::extract_set(const I& id, const V& index)
    const
{
    sLock lock(index_lock_);

    try {
        return index.at(id);

    } catch (...) {

        return {};
    }
}

std::set<std::string>& Bip47Channels::get_address_set(
    const eLock& lock,
    const Identifier& channelID)
{
    return _get_address_set(lock, Identifier::Factory(channelID));
}

std::set<std::string>& Bip47Channels::_get_address_set(
    const eLock&,  // TODO switch Node to Lockable
    OTIdentifier&& id)
{
    try {
        return address_index_.at(id);
    } catch (const std::out_of_range&) {
        return address_index_.insert({std::move(id), {}}).first->second;
    }
}

template <typename L>
Bip47Channels::ChannelData& Bip47Channels::get_channel_data(
    const L& lock,
    const Identifier& channelID) const
{
    return _get_channel_data(lock, Identifier::Factory(channelID));
}

template <typename L>
Bip47Channels::ChannelData& Bip47Channels::_get_channel_data(
    const L&,  // TODO switch Node to Lockable
    OTIdentifier&& id) const
{
    try {
        return channel_data_.at(id);
    } catch (const std::out_of_range&) {
        ChannelData blank{
            {}, proto::CITEMTYPE_ERROR, Identifier::Factory(), {}};

        return channel_data_.insert({std::move(id), std::move(blank)})
            .first->second;
    }
}

void Bip47Channels::index_addresses(
    const eLock& lock,
    const Identifier& channelID,
    const proto::Bip47Channel& data)
{
    auto& addresses = get_address_set(lock, channelID);
    std::set<std::string> newSet{};
    extract_addresses(data.incoming(), newSet);
    extract_addresses(data.outgoing(), newSet);
    addresses.swap(newSet);
}

void Bip47Channels::init(const std::string& hash)
{
    std::shared_ptr<proto::StorageBip47Contexts> serialized{nullptr};
    driver_.LoadProto(hash, serialized);

    if (!serialized) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failed to load bip47 channel index file.")
            .Flush();
        OT_FAIL
    }

    version_ = serialized->version();

    // Upgrade version
    if (CHANNEL_VERSION > version_) { version_ = CHANNEL_VERSION; }

    for (const auto& it : serialized->context()) {
        item_map_.emplace(
            it.itemid(), Metadata{it.hash(), it.alias(), 0, false});
    }

    eLock lock(index_lock_);

    for (const auto& address : serialized->address()) {
        auto channelID = Identifier::Factory(address.channelid());
        address_reverse_index_.insert({address.address(), channelID});
        _get_address_set(lock, std::move(channelID)).emplace(address.address());
    }

    for (const auto& index : serialized->index()) {
        auto id = Identifier::Factory(index.channelid());
        auto& [local, chain, contact, remote] =
            get_channel_data(lock, id.get());
        local = index.localpaymentcode();
        chain = index.chain();
        contact = Identifier::Factory(index.contact());
        remote = index.remotepaymentcode();
        chain_map_[chain].emplace(id);
        contact_map_[contact].emplace(id);
        local_map_[local].emplace(id);
        remote_map_[remote].emplace(id);
    }
}

bool Bip47Channels::Load(
    const Identifier& id,
    std::shared_ptr<proto::Bip47Channel>& output,
    const bool checking) const
{
    std::string alias{""};

    return load_proto<proto::Bip47Channel>(id.str(), output, alias, checking);
}

std::string Bip47Channels::LocalPaymentCode(const Identifier& channelID) const
{
    sLock lock(index_lock_);

    return std::get<0>(get_channel_data(lock, channelID));
}

std::string Bip47Channels::RemotePaymentCode(const Identifier& channelID) const
{
    sLock lock(index_lock_);

    return std::get<3>(get_channel_data(lock, channelID));
}

void Bip47Channels::reverse_index_addresses(const eLock& /* TODO Lockable*/)
{
    AddressReverseMap newMap{};

    for (const auto& [id, addresses] : address_index_) {
        for (const auto& address : addresses) { newMap.insert({address, id}); }
    }

    address_reverse_index_.swap(newMap);
}

bool Bip47Channels::save(const std::unique_lock<std::mutex>& lock) const
{
    if (!verify_write_lock(lock)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Lock failure.").Flush();
        OT_FAIL
    }

    auto serialized = serialize();

    if (!proto::Validate(serialized, VERBOSE)) { return false; }

    return driver_.StoreProto(serialized, root_);
}

proto::StorageBip47Contexts Bip47Channels::serialize() const
{
    proto::StorageBip47Contexts serialized{};
    serialized.set_version(version_);

    for (const auto item : item_map_) {
        const bool goodID = !item.first.empty();
        const bool goodHash = check_hash(std::get<0>(item.second));
        const bool good = goodID && goodHash;

        if (good) {
            serialize_index(item.first, item.second, *serialized.add_context());
        }
    }

    sLock lock(index_lock_);

    for (const auto& [channelID, addressSet] : address_index_) {
        for (const auto& address : addressSet) {
            OT_ASSERT(false == address.empty())

            auto& index = *serialized.add_address();
            index.set_version(ADDRESS_INDEX_VERSION);
            index.set_address(address);
            index.set_channelid(channelID->str());
        }
    }

    for (const auto& [id, data] : channel_data_) {
        const auto& [local, chain, contact, remote] = data;
        auto& index = *serialized.add_index();
        index.set_version(CHANNEL_INDEX_VERSION);
        index.set_localpaymentcode(local);
        index.set_chain(chain);
        index.set_contact(contact->str());
        index.set_remotepaymentcode(remote);
        const auto valid = proto::Validate(index, SILENT);

        // Invalid entries may appear due to queries for properties of
        // non-existent channels.
        if (false == valid) { serialized.mutable_index()->RemoveLast(); }
    }

    return serialized;
}

bool Bip47Channels::Store(
    const proto::Bip47Channel& data,
    Identifier& channelID)
{
    calculate_id(data, channelID);
    eLock lock(index_lock_);
    index_addresses(lock, channelID, data);
    reverse_index_addresses(lock);
    auto& [local, chain, contact, remote] = get_channel_data(lock, channelID);
    local = data.localpaymentcode();
    chain = data.chain();
    contact = Identifier::Factory(data.contact());
    remote = data.remotepaymentcode();

    return store_proto(data, channelID.str(), "");
}
}  // namespace opentxs::storage
