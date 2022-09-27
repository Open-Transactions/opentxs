// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#if defined(OTDB_PROTOCOL_BUFFERS)

#include <Bitcoin.pb.h>
#include <Generics.pb.h>
#include <Markets.pb.h>
#include <Moneychanger.pb.h>
#include <iostream>

#include "opentxs/Version.hpp"
#include "opentxs/util/Container.hpp"

namespace opentxs::OTDB
{
// Interface:    IStorablePB
//
class IStorablePB : public IStorable
{
public:
    ~IStorablePB() override = default;

    virtual auto getPBMessage() -> ::google::protobuf::MessageLite*;
    auto onPack(PackedBuffer& theBuffer, Storable& inObj) -> bool override;
    auto onUnpack(PackedBuffer& theBuffer, Storable& outObj) -> bool override;
    OT_USING_ISTORABLE_HOOKS;
};

// BUFFER for Protocol Buffers.
// Google's protocol buffers serializes to UnallocatedCStrings and streams. How
// conveeeeeenient.

class BufferPB : public PackedBuffer
{
    friend PackerSubclass<BufferPB>;
    friend IStorablePB;
    UnallocatedCString buffer_;

public:
    BufferPB()
        : PackedBuffer()
        , buffer_()
    {
    }
    ~BufferPB() override = default;
    auto PackString(const UnallocatedCString& theString) -> bool override;
    auto UnpackString(UnallocatedCString& theString) -> bool override;
    auto ReadFromIStream(std::istream& inStream, std::int64_t lFilesize)
        -> bool override;
    auto WriteToOStream(std::ostream& outStream) -> bool override;
    auto GetData() -> const std::uint8_t* override;
    auto GetSize() -> std::size_t override;
    void SetData(const std::uint8_t* pData, std::size_t theSize) override;
    auto GetBuffer() -> UnallocatedCString& { return buffer_; }
};

// Protocol Buffers packer.
//
using PackerPB = PackerSubclass<BufferPB>;

// Used for subclassing IStorablePB:
//
template <
    class theBaseType,
    class theInternalType,
    StoredObjectType theObjectType>
class ProtobufSubclass : public theBaseType, public IStorablePB
{
private:
    theInternalType pb_obj_;
    UnallocatedCString type_;

public:
    static auto Instantiate() -> Storable*
    {
        return dynamic_cast<Storable*>(
            new ProtobufSubclass<theBaseType, theInternalType, theObjectType>);
    }

    ProtobufSubclass()
        : theBaseType()
        , IStorablePB()
        , pb_obj_()
        , type_(
              StoredObjectTypeStrings[static_cast<std::int32_t>(theObjectType)])
    {
        type_ += "PB";
    }

    ProtobufSubclass(
        const ProtobufSubclass<theBaseType, theInternalType, theObjectType>&
            rhs)
        : theBaseType()
        , IStorablePB()
        , type_(
              StoredObjectTypeStrings[static_cast<std::int32_t>(theObjectType)])
    {
        type_ += "PB";
        rhs.CopyToObject(*this);
    }

    auto operator=(
        const ProtobufSubclass<theBaseType, theInternalType, theObjectType>&
            rhs)
        -> ProtobufSubclass<theBaseType, theInternalType, theObjectType>&
    {
        rhs.CopyToObject(*this);
        return *this;
    }

    void CopyToObject(
        ProtobufSubclass<theBaseType, theInternalType, theObjectType>&
            theNewStorable) const
    {
        std::unique_ptr<OTPacker> pPacker(
            OTPacker::Create(PACK_PROTOCOL_BUFFERS));
        const auto* pIntermediate = dynamic_cast<const OTDB::Storable*>(this);

        if (!pPacker) { OT_FAIL; }

        std::unique_ptr<PackedBuffer> pBuffer(
            pPacker->Pack(*(const_cast<OTDB::Storable*>(pIntermediate))));

        if (!pBuffer) { OT_FAIL; }

        if (!pPacker->Unpack(*pBuffer, theNewStorable)) { OT_FAIL; }
    }

    auto getPBMessage() -> ::google::protobuf::MessageLite* override;

    auto clone() const -> theBaseType* override
    {
        return dynamic_cast<theBaseType*>(do_clone());
    }

    auto do_clone() const -> IStorable*
    {
        Storable* pNewStorable =
            Storable::Create(theObjectType, PACK_PROTOCOL_BUFFERS);
        if (nullptr == pNewStorable) { OT_FAIL; }
        CopyToObject(
            *(dynamic_cast<
                ProtobufSubclass<theBaseType, theInternalType, theObjectType>*>(
                pNewStorable)));
        return dynamic_cast<IStorable*>(pNewStorable);
    }

    ~ProtobufSubclass() override = default;
    OT_USING_ISTORABLE_HOOKS;
    void hookBeforePack() override;   // <=== Implement this if you subclass.
    void hookAfterUnpack() override;  // <=== Implement this if you subclass.
};

#define DECLARE_PROTOBUF_SUBCLASS(                                             \
    theBaseType, theInternalType, theNewType, theObjectType)                   \
    template <>                                                                \
    void ProtobufSubclass<theBaseType, theInternalType, theObjectType>::       \
        hookBeforePack();                                                      \
    template <>                                                                \
    void ProtobufSubclass<theBaseType, theInternalType, theObjectType>::       \
        hookAfterUnpack();                                                     \
    using theNewType =                                                         \
        ProtobufSubclass<theBaseType, theInternalType, theObjectType>

// THE ACTUAL SUBCLASSES:

DECLARE_PROTOBUF_SUBCLASS(
    OTDBString,
    String_InternalPB,
    StringPB,
    STORED_OBJ_STRING);
DECLARE_PROTOBUF_SUBCLASS(Blob, Blob_InternalPB, BlobPB, STORED_OBJ_BLOB);
DECLARE_PROTOBUF_SUBCLASS(
    StringMap,
    StringMap_InternalPB,
    StringMapPB,
    STORED_OBJ_STRING_MAP);
DECLARE_PROTOBUF_SUBCLASS(
    BitcoinAcct,
    BitcoinAcct_InternalPB,
    BitcoinAcctPB,
    STORED_OBJ_BITCOIN_ACCT);
DECLARE_PROTOBUF_SUBCLASS(
    BitcoinServer,
    BitcoinServer_InternalPB,
    BitcoinServerPB,
    STORED_OBJ_BITCOIN_SERVER);
DECLARE_PROTOBUF_SUBCLASS(
    RippleServer,
    RippleServer_InternalPB,
    RippleServerPB,
    STORED_OBJ_RIPPLE_SERVER);
DECLARE_PROTOBUF_SUBCLASS(
    LoomServer,
    LoomServer_InternalPB,
    LoomServerPB,
    STORED_OBJ_LOOM_SERVER);
DECLARE_PROTOBUF_SUBCLASS(
    ServerInfo,
    ServerInfo_InternalPB,
    ServerInfoPB,
    STORED_OBJ_SERVER_INFO);
DECLARE_PROTOBUF_SUBCLASS(
    ContactAcct,
    ContactAcct_InternalPB,
    ContactAcctPB,
    STORED_OBJ_CONTACT_ACCT);
DECLARE_PROTOBUF_SUBCLASS(
    ContactNym,
    ContactNym_InternalPB,
    ContactNymPB,
    STORED_OBJ_CONTACT_NYM);
DECLARE_PROTOBUF_SUBCLASS(
    Contact,
    Contact_InternalPB,
    ContactPB,
    STORED_OBJ_CONTACT);
DECLARE_PROTOBUF_SUBCLASS(
    AddressBook,
    AddressBook_InternalPB,
    AddressBookPB,
    STORED_OBJ_ADDRESS_BOOK);
DECLARE_PROTOBUF_SUBCLASS(
    WalletData,
    WalletData_InternalPB,
    WalletDataPB,
    STORED_OBJ_WALLET_DATA);
DECLARE_PROTOBUF_SUBCLASS(
    MarketData,
    MarketData_InternalPB,
    MarketDataPB,
    STORED_OBJ_MARKET_DATA);
DECLARE_PROTOBUF_SUBCLASS(
    MarketList,
    MarketList_InternalPB,
    MarketListPB,
    STORED_OBJ_MARKET_LIST);

DECLARE_PROTOBUF_SUBCLASS(
    BidData,
    OfferDataMarket_InternalPB,
    BidDataPB,
    STORED_OBJ_BID_DATA);
DECLARE_PROTOBUF_SUBCLASS(
    AskData,
    OfferDataMarket_InternalPB,
    AskDataPB,
    STORED_OBJ_ASK_DATA);
DECLARE_PROTOBUF_SUBCLASS(
    OfferListMarket,
    OfferListMarket_InternalPB,
    OfferListMarketPB,
    STORED_OBJ_OFFER_LIST_MARKET);
DECLARE_PROTOBUF_SUBCLASS(
    TradeDataMarket,
    TradeDataMarket_InternalPB,
    TradeDataMarketPB,
    STORED_OBJ_TRADE_DATA_MARKET);
DECLARE_PROTOBUF_SUBCLASS(
    TradeListMarket,
    TradeListMarket_InternalPB,
    TradeListMarketPB,
    STORED_OBJ_TRADE_LIST_MARKET);
DECLARE_PROTOBUF_SUBCLASS(
    OfferDataNym,
    OfferDataNym_InternalPB,
    OfferDataNymPB,
    STORED_OBJ_OFFER_DATA_NYM);
DECLARE_PROTOBUF_SUBCLASS(
    OfferListNym,
    OfferListNym_InternalPB,
    OfferListNymPB,
    STORED_OBJ_OFFER_LIST_NYM);
DECLARE_PROTOBUF_SUBCLASS(
    TradeDataNym,
    TradeDataNym_InternalPB,
    TradeDataNymPB,
    STORED_OBJ_TRADE_DATA_NYM);
DECLARE_PROTOBUF_SUBCLASS(
    TradeListNym,
    TradeListNym_InternalPB,
    TradeListNymPB,
    STORED_OBJ_TRADE_LIST_NYM);

using BidData_InternalPB = OfferDataMarket_InternalPB;
using AskData_InternalPB = OfferDataMarket_InternalPB;

// !! ALL OF THESE have to provide implementations for hookBeforePack() and
// hookAfterUnpack().
// In .cpp file:
/*
void SUBCLASS_HERE::hookBeforePack()
{
pb_obj_.set_PROPERTY_NAME_GOES_HERE(PROPERTY_NAME_GOES_HERE);
}
void SUBCLASS_HERE::hookAfterUnpack()
{
PROPERTY_NAME_GOES_HERE    = pb_obj_.PROPERTY_NAME_GOES_HERE();
}
*/

}  // namespace opentxs::OTDB

#endif  // defined(OTDB_PROTOCOL_BUFFERS)
