// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::OTDB::PackType
// IWYU pragma: no_forward_declare opentxs::OTDB::StorageType

#pragma once

#include <cstddef>
#include <cstdint>
#include <iostream>
#include <memory>
#include <utility>

#include "internal/util/LogMacros.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

namespace OTDB
{
class Acct;
class AddressBook;
class AskData;
class BidData;
class BitcoinAcct;
class BitcoinServer;
class Blob;
class Contact;
class ContactAcct;
class ContactNym;
class Displayable;
class LoomServer;
class MarketData;
class MarketList;
class OTDBString;
class OfferDataMarket;
class OfferDataNym;
class OfferListMarket;
class OfferListNym;
class RippleServer;
class Server;
class ServerInfo;
class StringMap;
class TradeDataMarket;
class TradeDataNym;
class TradeListMarket;
class TradeListNym;
class WalletData;

#define OTDB_PROTOCOL_BUFFERS 1
#define OTDB_DEFAULT_PACKER OTDB::PACK_PROTOCOL_BUFFERS
#define OTDB_DEFAULT_STORAGE OTDB::STORE_FILESYSTEM

// ENUMS:    PackType, StorageType, and StoredObjectType.

// Currently supporting MsgPack and Protocol Buffers.
//
enum PackType  // PACKING TYPE
{
    PACK_MESSAGE_PACK = 0,  // Using MessagePack as packer.
    PACK_PROTOCOL_BUFFERS,  // Using Google Protocol Buffers as packer.
    PACK_TYPE_ERROR         // (Should never be.)
};                          // IWYU pragma: export

// Currently supporting filesystem, with subclasses possible via API.
//
enum StorageType  // STORAGE TYPE
{
    STORE_FILESYSTEM = 0,  // Filesystem
    STORE_TYPE_SUBCLASS    // (Subclass provided by API client via SWIG.)
};                         // IWYU pragma: export

// NOLINTNEXTLINE(modernize-avoid-c-arrays)
extern const char* StoredObjectTypeStrings[];

enum StoredObjectType {
    STORED_OBJ_STRING = 0,   // Just a string.
    STORED_OBJ_BLOB,         // Used for storing binary data. Bytes of arbitrary
                             // length.
    STORED_OBJ_STRING_MAP,   // A StringMap is a list of Key/Value pairs, useful
                             // for storing nearly anything.
    STORED_OBJ_WALLET_DATA,  // The GUI wallet's stored data
    STORED_OBJ_BITCOIN_ACCT,    // The GUI wallet's stored data about a Bitcoin
                                // acct
    STORED_OBJ_BITCOIN_SERVER,  // The GUI wallet's stored data about a Bitcoin
                                // RPC port.
    STORED_OBJ_RIPPLE_SERVER,   // The GUI wallet's stored data about a Ripple
                                // server.
    STORED_OBJ_LOOM_SERVER,     // The GUI wallet's stored data about a Loom
                                // server.
    STORED_OBJ_SERVER_INFO,     // A Nym has a list of these.
    STORED_OBJ_CONTACT_NYM,     // This is a Nym record inside a contact of your
                                // address book.
    STORED_OBJ_CONTACT_ACCT,    // This is an account record inside a contact of
                                // your address book.
    STORED_OBJ_CONTACT,         // Your address book has a list of these.
    STORED_OBJ_ADDRESS_BOOK,    // Your address book.
    STORED_OBJ_MARKET_DATA,     // The description data for any given Market ID.
    STORED_OBJ_MARKET_LIST,     // A list of MarketDatas.
    STORED_OBJ_BID_DATA,  // Offer details (doesn't contain private details)
    STORED_OBJ_ASK_DATA,  // Offer details (doesn't contain private details)
    STORED_OBJ_OFFER_LIST_MARKET,  // A list of offer details, for a specific
                                   // market.
    STORED_OBJ_TRADE_DATA_MARKET,  // Trade details (doesn't contain private
                                   // data)
    STORED_OBJ_TRADE_LIST_MARKET,  // A list of trade details, for a specific
                                   // market.
    STORED_OBJ_OFFER_DATA_NYM,     // Private offer details for a particular Nym
                                   // and
                                   // Offer.
    STORED_OBJ_OFFER_LIST_NYM,     // A list of private offer details for a
                                   // particular Nym.
    STORED_OBJ_TRADE_DATA_NYM,     // Private trade details for a particular Nym
                                   // and
                                   // Trade.
    STORED_OBJ_TRADE_LIST_NYM,     // A list of private trade details for a
                                   // particular Nym and Offer.
    STORED_OBJ_ERROR               // (Should never be.)
};

class OTPacker;  // A packer (Could be MsgPack, or Google Protocol Buffers, or a
                 // Swappable.)
class PackedBuffer;  // A buffer for containing a PACKED STORABLE. (On its way
// ABSTRACT BASE CLASSES
//
class Storable;  // A storable object
                 // json lib...)
class Storage;   // A storage context (database, filesystem, cloud, etc.
                 // to/from storage.)

// OTDB NAMESPACE "CONSTRUCTOR"
//
class InitOTDBDetails
{
public:
    InitOTDBDetails();   // See implementation of this in CPP file for namespace
                         // construction.
    ~InitOTDBDetails();  // Ditto.
};

// As far as the USERS of the Storage API are concerned, the above classes are
// nearly everything.
// (In addition to the "Pure Data" classes such as ContactNym, BitcoinAcct,
// etc.)
// Behind the scenes, in OTStorage, there is the IStorable interface, with its
// progeny, the various
// subclasses based on specific packers, such as ContactNymMsgpack, or
// WalletDataProtobuf. But these
// are hidden, and are not seen outside of OTStorage in its actual USE.

//
// OTDB Namespace internal typedefs
//
// In short:
// - InstantiateFunc (function pointer type.)
// - InstantiateFuncKey (based on Pack Type and Stored Object Type.)
// - mapOfFunctions (type: map of InstantiateFuncs, indexed by
// InstantiateFuncKeys.)
//
// Resulting in: pFunctionMap (Instance of mapOfFunctions, created in the OTDB
// constructor.)
//
using InstantiateFunc = Storable*();  // Each storable has one of these as a
                                      // static method.
using InstantiateFuncKey = std::pair<PackType, StoredObjectType>;  // Those
// methods are
// stored as
// function
// pointers
// here, and
// they are
// indexed by Pack Type and Stored Object Type. So if you know "LoomAcct" and
// "protocol buffers", those form the KEY for looking up the LoomAcctPB
// instantiator.
using mapOfFunctions =
    UnallocatedMap<InstantiateFuncKey, InstantiateFunc*>;  //...basically
                                                           // implementing
                                                           // my own vtable, eh?

// OTDB Namespace PRIVATE MEMBERS
// this "details" naming is a common C++ idiom for "private" in a namespace.
//
namespace details
{
extern OTDB::Storage* s_pStorage;

extern OTDB::mapOfFunctions* pFunctionMap;  // This is a pointer so I can
                                            // control
                                            // what order it is created in, on
                                            // startup.
}  // namespace details

// All of the class hierarchy under Storable is based on OT data design. (Not
// packing and such implementation details.)
// So when we need to add custom behavior that's common to groups of the final
// subclasses,
// we use **Interfaces** to do it.

// ===> That way, the Storable hierarchy can focus on DATA, (and form the
// external interface for OTStorage.)
// ===> while the IStorable hierarchy focuses on PACKING.   (and is hidden
// INSIDE OTStorage.)
// ===> (Things are more elegant this way.)

//
//
// Interface:    IStorable
//
// Each specific Packer library (MsgPack, Protobuf, etc) must provide an
// interface
// derived from IStorable (They're all listed somewhere below.)
//

class IStorable
{
public:
    virtual ~IStorable() = default;

    // buffer is output, inObj is input.
    virtual auto onPack(PackedBuffer& theBuffer, Storable& inObj) -> bool = 0;

    // buffer is input, outObj is output.
    virtual auto onUnpack(PackedBuffer& theBuffer, Storable& outObj)
        -> bool = 0;

    // This is called just before packing a storable. (Opportunity to copy
    // values...)
    virtual void hookBeforePack() {}

    // This is called just after unpacking a storable. (Opportunity to copy
    // values...)
    virtual void hookAfterUnpack() {}
};

#define DEFINE_OT_DYNAMIC_CAST(CLASS_NAME)                                     \
    auto clone() const->CLASS_NAME* override                                   \
    {                                                                          \
        std::cout                                                              \
            << "********* THIS SHOULD NEVER HAPPEN!!!!! *****************"     \
            << std::endl;                                                      \
        OT_FAIL;                                                               \
    }                                                                          \
    static auto ot_dynamic_cast(Storable* pObject)->CLASS_NAME*                \
    {                                                                          \
        return dynamic_cast<CLASS_NAME*>(pObject);                             \
    }

// STORABLE
//
// Abstract base class for OT serializable object types.
//
class Storable
{
protected:
    Storable()
        : type_("Storable")
    {
    }

    UnallocatedCString type_;

public:
    virtual ~Storable() = default;

    // %ignore spam(uint16_t); API users don't need this function, it's for
    // internal purposes.
    static auto Create(StoredObjectType eType, PackType thePackType)
        -> Storable*;

    virtual auto clone() const -> Storable* = 0;
    static auto ot_dynamic_cast(Storable* pObject) -> Storable*
    {
        return dynamic_cast<Storable*>(pObject);
    }
};

// PACKED BUFFER (for storing PACKED DATA)
//
// %ignore these classes (I think)
//

class PackedBuffer
{
protected:
    PackedBuffer() = default;  // Only subclasses of this should be
                               // instantiated.
public:
    virtual ~PackedBuffer() = default;

    virtual auto PackString(const UnallocatedCString& theString) -> bool = 0;
    virtual auto UnpackString(UnallocatedCString& theString) -> bool = 0;

    virtual auto ReadFromIStream(std::istream& inStream, std::int64_t lFilesize)
        -> bool = 0;
    virtual auto WriteToOStream(std::ostream& outStream) -> bool = 0;

    virtual auto GetData() -> const std::uint8_t* = 0;
    virtual auto GetSize() -> std::size_t = 0;

    virtual void SetData(const std::uint8_t* pData, std::size_t theSize) = 0;
};

// PACKER (now OTPacker since MsgPack also has a "Packer" in a #define).
//
// abstract base class for a packer
//

// %ignore spam(uint16_t);  (probably for all packers.)
class OTPacker
{
protected:
    OTPacker() = default;

public:
    virtual ~OTPacker() = default;

    static auto Create(PackType ePackType) -> OTPacker*;

    auto GetType() const -> PackType;

    auto Pack(Storable& inObj) -> PackedBuffer*;
    auto Unpack(PackedBuffer& inBuf, Storable& outObj) -> bool;

    auto Pack(const UnallocatedCString& inObj) -> PackedBuffer*;
    auto Unpack(PackedBuffer& inBuf, UnallocatedCString& outObj) -> bool;

    virtual auto CreateBuffer() -> PackedBuffer* = 0;
};

// For declaring subclasses of OTPacker.

template <class theBufferType>
class PackerSubclass : public OTPacker
{
public:
    PackerSubclass()
        : OTPacker()
    {
    }
    ~PackerSubclass() override = default;

    auto CreateBuffer() -> PackedBuffer* override { return new theBufferType; }

    // You don't see onPack and onUnpack here because they are on IStorable.
};

// To use:
// typedef PackerSubclass<theBufferType> theType;
//

// SUBCLASSES:
//
// (Actual declarations are at the bottom of the file.)
//
//    typedef PackerSubclass<BufferPB>        PackerPB;
//
//
// STORAGE  -- abstract base class
//
class Storage
{
private:
    OTPacker* packer_{nullptr};

protected:
    Storage()
        : packer_(nullptr)
    {
    }

    Storage(const Storage&)
        : packer_(nullptr)
    {
    }  // We don't want to copy the pointer. Let it create its own.

    // This is called once, in the factory.
    void SetPacker(OTPacker& thePacker)
    {
        OT_ASSERT(nullptr == packer_);
        packer_ = &thePacker;
    }

    // OVERRIDABLES
    //
    // If you wish to MAKE YOUR OWN subclass of Storage (to provide your own
    // storage system)
    // then just subclass OTDB::Storage, and override the below methods. For an
    // example of how
    // it's done, see StorageFS (filesystem), which is included below and in
    // OTStorage.cpp.
    //
    // NOTE: This should be possible even in other languages! I'm using SWIG
    // directors, meaning
    // that you can make a Java subclass of OTDB::Storage, or a Python subclass,
    // etc. This isn't
    // possible with the other classes in OTStorage (yet), which must be
    // subclassed in C++. But
    // for this class, it is.
    //
    virtual auto onStorePackedBuffer(
        const api::Session& api,
        PackedBuffer& theBuffer,
        const UnallocatedCString& dataFolder,
        const UnallocatedCString& strFolder,
        const UnallocatedCString& oneStr,
        const UnallocatedCString& twoStr,
        const UnallocatedCString& threeStr) -> bool = 0;

    virtual auto onQueryPackedBuffer(
        const api::Session& api,
        PackedBuffer& theBuffer,
        const UnallocatedCString& dataFolder,
        const UnallocatedCString& strFolder,
        const UnallocatedCString& oneStr,
        const UnallocatedCString& twoStr,
        const UnallocatedCString& threeStr) -> bool = 0;

    virtual auto onStorePlainString(
        const api::Session& api,
        const UnallocatedCString& theBuffer,
        const UnallocatedCString& dataFolder,
        const UnallocatedCString& strFolder,
        const UnallocatedCString& oneStr,
        const UnallocatedCString& twoStr,
        const UnallocatedCString& threeStr) -> bool = 0;

    virtual auto onQueryPlainString(
        const api::Session& api,
        UnallocatedCString& theBuffer,
        const UnallocatedCString& dataFolder,
        const UnallocatedCString& strFolder,
        const UnallocatedCString& oneStr,
        const UnallocatedCString& twoStr,
        const UnallocatedCString& threeStr) -> bool = 0;

    virtual auto onEraseValueByKey(
        const api::Session& api,
        const UnallocatedCString& dataFolder,
        const UnallocatedCString& strFolder,
        const UnallocatedCString& oneStr,
        const UnallocatedCString& twoStr,
        const UnallocatedCString& threeStr) -> bool = 0;

public:
    // Use GetPacker() to access the Packer, throughout duration of this Storage
    // object. If it doesn't exist yet, this function will create it on the
    // first call. (The parameter allows you the choose what type will be
    // created, other than default.)
    //
    // This way, whenever using an OT Storage, you KNOW the packer is always the
    // right one, and that you don't have to fiddle with it at all. You can also
    // therefore use it for creating instances of various Storables and
    // PackedBuffers, and knowing that the right types will be instantiated
    // automatically, with the buffer being the appropriate subclass for the
    // packer.
    auto GetPacker(PackType ePackType = OTDB_DEFAULT_PACKER) -> OTPacker*;

    // See if the file is there.
    virtual auto Exists(
        const api::Session& api,
        const UnallocatedCString& dataFolder,
        const UnallocatedCString& strFolder,
        const UnallocatedCString& oneStr,
        const UnallocatedCString& twoStr,
        const UnallocatedCString& threeStr) -> bool = 0;

    virtual auto FormPathString(
        const api::Session& api,
        UnallocatedCString& strOutput,
        const UnallocatedCString& dataFolder,
        const UnallocatedCString& strFolder,
        const UnallocatedCString& oneStr,
        const UnallocatedCString& twoStr,
        const UnallocatedCString& threeStr) -> std::int64_t = 0;

    Storage(Storage&&) = delete;
    auto operator=(const Storage&) -> Storage& = delete;
    auto operator=(Storage&&) -> Storage& = delete;

    virtual ~Storage()
    {
        if (nullptr != packer_) { delete packer_; }
        packer_ = nullptr;
    }

    // Store/Retrieve a string.

    auto StoreString(
        const api::Session& api,
        const UnallocatedCString& strContents,
        const UnallocatedCString& dataFolder,
        const UnallocatedCString& strFolder,
        const UnallocatedCString& oneStr,
        const UnallocatedCString& twoStr,
        const UnallocatedCString& threeStr) -> bool;

    auto QueryString(
        const api::Session& api,
        const UnallocatedCString& dataFolder,
        const UnallocatedCString& strFolder,
        const UnallocatedCString& oneStr,
        const UnallocatedCString& twoStr,
        const UnallocatedCString& threeStr) -> UnallocatedCString;

    auto StorePlainString(
        const api::Session& api,
        const UnallocatedCString& strContents,
        const UnallocatedCString& dataFolder,
        const UnallocatedCString& strFolder,
        const UnallocatedCString& oneStr,
        const UnallocatedCString& twoStr,
        const UnallocatedCString& threeStr) -> bool;

    auto QueryPlainString(
        const api::Session& api,
        const UnallocatedCString& dataFolder,
        const UnallocatedCString& strFolder,
        const UnallocatedCString& oneStr,
        const UnallocatedCString& twoStr,
        const UnallocatedCString& threeStr) -> UnallocatedCString;

    // Store/Retrieve an object. (Storable.)

    auto StoreObject(
        const api::Session& api,
        Storable& theContents,
        const UnallocatedCString& dataFolder,
        const UnallocatedCString& strFolder,
        const UnallocatedCString& oneStr,
        const UnallocatedCString& twoStr,
        const UnallocatedCString& threeStr) -> bool;

    // Use %newobject OTDB::Storage::QueryObject();
    auto QueryObject(
        const api::Session& api,
        const StoredObjectType& theObjectType,
        const UnallocatedCString& dataFolder,
        const UnallocatedCString& strFolder,
        const UnallocatedCString& oneStr,
        const UnallocatedCString& twoStr,
        const UnallocatedCString& threeStr) -> Storable*;
    // Store/Retrieve a Storable object inside an Armored object.

    auto EncodeObject(const api::Session& api, Storable& theContents)
        -> UnallocatedCString;

    // Use %newobject OTDB::Storage::DecodeObject();
    auto DecodeObject(
        const StoredObjectType& theObjectType,
        const UnallocatedCString& strInput) -> Storable*;

    // Erase any value based on its location.

    auto EraseValueByKey(
        const api::Session& api,
        const UnallocatedCString& dataFolder,
        const UnallocatedCString& strFolder,
        const UnallocatedCString& oneStr,
        const UnallocatedCString& twoStr,
        const UnallocatedCString& threeStr) -> bool;

    // Note:
    // Make sure to use: %newobject Factory::createObj();  IN OTAPI.i file!
    //
    // That way, Java garbage cleanup will handle object after this.
    // (Instead of leaking because it thinks C++ will clean it up.)
    //
    // Factory for Storable objects.   %newobject Factory::createObj();
    auto CreateObject(const StoredObjectType& eType) -> Storable*;

    // Factory for Storage itself.  %ignore this in OTAPI.i  (It's accessed
    // through
    // a namespace-level function, whereas this is for internal purposes.)
    //
    static auto Create(
        const StorageType& eStorageType,
        const PackType& ePackType) -> Storage*;  // FACTORY

    auto GetType() const -> StorageType;
};

//
// OTDB Namespace PUBLIC INTERFACE
//

auto InitDefaultStorage(const StorageType eStoreType, const PackType ePackType)
    -> bool;

// Default Storage instance:
auto GetDefaultStorage() -> Storage*;

// %newobject Factory::createObj();
auto CreateStorageContext(
    const StorageType eStoreType,
    const PackType ePackType = OTDB_DEFAULT_PACKER) -> Storage*;

auto CreateObject(const StoredObjectType eType) -> Storable*;

// BELOW FUNCTIONS use the DEFAULT Storage context for the OTDB Namespace

// Check if the values are good.
//
auto CheckStringsExistInOrder(
    const UnallocatedCString& dataFolder,
    const UnallocatedCString& strFolder,
    const UnallocatedCString& oneStr,
    const UnallocatedCString& twoStr,
    const UnallocatedCString& threeStr,
    const char* szFuncName = nullptr) -> bool;

// See if the file is there.
//
auto Exists(
    const api::Session& api,
    const UnallocatedCString& dataFolder,
    const UnallocatedCString strFolder,
    const UnallocatedCString oneStr,
    const UnallocatedCString twoStr,
    const UnallocatedCString threeStr) -> bool;

auto FormPathString(
    const api::Session& api,
    UnallocatedCString& strOutput,
    const UnallocatedCString& dataFolder,
    const UnallocatedCString& strFolder,
    const UnallocatedCString& oneStr,
    const UnallocatedCString& twoStr,
    const UnallocatedCString& threeStr) -> std::int64_t;
// Store/Retrieve a string.
//
auto StoreString(
    const api::Session& api,
    const UnallocatedCString& strContents,
    const UnallocatedCString& dataFolder,
    const UnallocatedCString& strFolder,
    const UnallocatedCString& oneStr,
    const UnallocatedCString& twoStr,
    const UnallocatedCString& threeStr) -> bool;

auto QueryString(
    const api::Session& api,
    const UnallocatedCString& dataFolder,
    const UnallocatedCString& strFolder,
    const UnallocatedCString& oneStr,
    const UnallocatedCString& twoStr,
    const UnallocatedCString& threeStr) -> UnallocatedCString;

auto StorePlainString(
    const api::Session& api,
    const UnallocatedCString& strContents,
    const UnallocatedCString& dataFolder,
    const UnallocatedCString& strFolder,
    const UnallocatedCString& oneStr,
    const UnallocatedCString& twoStr,
    const UnallocatedCString& threeStr) -> bool;

auto QueryPlainString(
    const api::Session& api,
    const UnallocatedCString& dataFolder,
    const UnallocatedCString& strFolder,
    const UnallocatedCString& oneStr,
    const UnallocatedCString& twoStr,
    const UnallocatedCString& threeStr) -> UnallocatedCString;

// Store/Retrieve an object. (Storable.)
//
auto StoreObject(
    const api::Session& api,
    Storable& theContents,
    const UnallocatedCString& dataFolder,
    const UnallocatedCString& strFolder,
    const UnallocatedCString& oneStr,
    const UnallocatedCString& twoStr,
    const UnallocatedCString& threeStr) -> bool;

// Use %newobject OTDB::Storage::Query();
auto QueryObject(
    const api::Session& api,
    const StoredObjectType theObjectType,
    const UnallocatedCString& dataFolder,
    const UnallocatedCString& strFolder,
    const UnallocatedCString& oneStr,
    const UnallocatedCString& twoStr,
    const UnallocatedCString& threeStr) -> Storable*;

// Store/Retrieve a Storable object inside an Armored object.
auto EncodeObject(const api::Session& api, Storable& theContents)
    -> UnallocatedCString;

// Use %newobject OTDB::Storage::DecodeObject();
auto DecodeObject(
    const StoredObjectType theObjectType,
    const UnallocatedCString& strInput) -> Storable*;

// Erase any value based on its location.

auto EraseValueByKey(
    const api::Session& api,
    const UnallocatedCString& dataFolder,
    const UnallocatedCString& strFolder,
    const UnallocatedCString& oneStr,
    const UnallocatedCString& twoStr,
    const UnallocatedCString& threeStr) -> bool;

#define DECLARE_GET_ADD_REMOVE(name)                                           \
                                                                               \
protected:                                                                     \
    UnallocatedDeque<std::shared_ptr<name>> list_##name##s;                    \
                                                                               \
public:                                                                        \
    auto Get##name##Count()->std::size_t;                                      \
    auto Get##name(std::size_t nIndex)->name*;                                 \
    auto Remove##name(std::size_t nIndex##name)->bool;                         \
    auto Add##name(name& disownObject)->bool

// Serialized types...
//
// Here the entire hierarchy focuses on the OT data itself.
// Later, subclasses are made providing the final implementation,
// based on the packer type. (Same for buffers.)

class OTDBString : public Storable
{
    // You never actually get an instance of this, only its subclasses.
    // Therefore, I don't allow you to access the constructor except through the
    // factory.
protected:
    OTDBString()
        : Storable()
        , string_()
    {
        type_ = "OTDBString";
    }
    OTDBString(const UnallocatedCString& rhs)
        : Storable()
        , string_(rhs)
    {
        type_ = "OTDBString";
    }

public:
    ~OTDBString() override = default;

    UnallocatedCString string_;

    DEFINE_OT_DYNAMIC_CAST(OTDBString)
};

class Blob : public Storable
{
    // You never actually get an instance of this, only its subclasses.
    // Therefore, I don't allow you to access the constructor except through the
    // factory.
protected:
    Blob()
        : Storable()
        , mem_buffer_()
    {
        type_ = "Blob";
    }

public:
    ~Blob() override = default;

    UnallocatedVector<std::uint8_t> mem_buffer_;  // Where the actual binary
                                                  // data is stored, before
                                                  // packing.

    DEFINE_OT_DYNAMIC_CAST(Blob)
};

// The most useful generic data object... a map of strings, key/value pairs.
//
class StringMap : public Storable
{
    // You never actually get an instance of this, only its subclasses.
    // Therefore, I don't allow you to access the constructor except through the
    // factory.
protected:
    StringMap()
        : Storable()
        , the_map_()
    {
        type_ = "StringMap";
    }

public:
    ~StringMap() override = default;

    UnallocatedMap<UnallocatedCString, UnallocatedCString> the_map_;

    void SetValue(
        const UnallocatedCString& strKey,
        const UnallocatedCString& strValue)
    {
        auto ii = the_map_.find(strKey);
        if (ii != the_map_.end()) { the_map_.erase(ii); }
        the_map_[strKey] = strValue;
    }

    auto GetValue(const UnallocatedCString& strKey) -> UnallocatedCString
    {
        UnallocatedCString ret_val("");
        auto ii = the_map_.find(strKey);
        if (ii != the_map_.end()) { ret_val = (*ii).second; }
        return ret_val;
    }

    DEFINE_OT_DYNAMIC_CAST(StringMap)
};

class Displayable : public Storable
{
    // You never actually get an instance of this, only its subclasses.
    // Therefore, I don't allow you to access the constructor except through the
    // factory.
protected:
    Displayable()
        : Storable()
        , gui_label_()
    {
        type_ = "Displayable";
    }

public:
    ~Displayable() override = default;

    UnallocatedCString gui_label_;  // The label that appears in the GUI

    DEFINE_OT_DYNAMIC_CAST(Displayable)
};

class MarketData : public Displayable
{
    // You never actually get an instance of this, only its subclasses.
    // Therefore, I don't allow you to access the constructor except through
    // factory.
protected:
    MarketData()
        : Displayable()
        , notary_id_()
        , market_id_()
        , instrument_definition_id_()
        , currency_type_id_()
        , scale_("0")
        , total_assets_("0")
        , number_bids_("0")
        , number_asks_()
        , last_sale_price_("0")
        , current_bid_("0")
        , current_ask_("0")
        , volume_trades_("0")
        , volume_assets_("0")
        , volume_currency_("0")
        , recent_highest_bid_("0")
        , recent_lowest_ask_("0")
        , last_sale_date_("0")
    {
        type_ = "MarketData";
    }

public:
    ~MarketData() override = default;

    using Displayable::gui_label_;  // The label that appears in the GUI

    UnallocatedCString notary_id_;
    UnallocatedCString market_id_;

    UnallocatedCString instrument_definition_id_;
    UnallocatedCString currency_type_id_;

    UnallocatedCString scale_;  // the Market scale. (A trade in any particular
                                // asset is measured in X units of SCALE.)
    // IOW, if the scale is 5000 on the gold market, that means "3 units" is
    // 15000 gold

    UnallocatedCString total_assets_;  // total amount of assets available on
                                       // market for purchase.

    UnallocatedCString number_bids_;  // number of bids that are currently on
                                      // the market.
    UnallocatedCString number_asks_;  // number of asks that are currently on
                                      // the market.

    UnallocatedCString last_sale_price_;  // The price at which the most recent
                                          // trade occurred on this market.
    UnallocatedCString current_bid_;      // The highest bid currently on the
                                          // market.
    UnallocatedCString current_ask_;      // The lowest ask price currently
                                          // available on the market.

    UnallocatedCString volume_trades_;  // 24-hour period, number of trades.

    UnallocatedCString volume_assets_;    // 24-hour volume, amount of assets
                                          // traded.
    UnallocatedCString volume_currency_;  // 24-hour volume, amount of currency
                                          // paid for assets traded.

    UnallocatedCString recent_highest_bid_;  // in a 24hour period, the highest
                                             // bid to hit the market.
    UnallocatedCString recent_lowest_ask_;   // in a 24hour period, the lowest
                                             // ask to hit the market.

    UnallocatedCString last_sale_date_;  // (NEW FIELD) The date on which the
                                         // most recent trade occurred on this
                                         // market.

    DEFINE_OT_DYNAMIC_CAST(MarketData)
};

class MarketList : public Storable
{
    // You never actually get an instance of this, only its subclasses.
    // Therefore, I don't allow you to access the constructor except through
    // factory.
protected:
    MarketList()
        : Storable()
        , list_MarketDatas()
    {
        type_ = "MarketList";
    }

public:
    ~MarketList() override = default;

    DECLARE_GET_ADD_REMOVE(MarketData);

    DEFINE_OT_DYNAMIC_CAST(MarketList)
};

class OfferDataMarket : public Displayable
{
    // You never actually get an instance of this, only its subclasses.
    // Therefore, I don't allow you to access the constructor except through
    // factory.
protected:
    OfferDataMarket()
        : Displayable()
        , transaction_id_("0")
        , price_per_scale_("1")
        , available_assets_("0")
        , minimum_increment_("1")
        , date_("0")
    {
        type_ = "OfferDataMarket";
    }

public:
    ~OfferDataMarket() override = default;

    using Displayable::gui_label_;  // The label that appears in the GUI

    UnallocatedCString transaction_id_;
    UnallocatedCString price_per_scale_;
    UnallocatedCString available_assets_;

    // Each sale or purchase against (total_assets - finished_so_far) must be in
    // minimum increments.
    // Minimum Increment must be evenly divisible by scale.
    // (This effectively becomes a "FILL OR KILL" order if set to the same value
    // as total_assets. Also, MUST be 1
    // or greater. CANNOT be zero. Enforce this at class level. You cannot sell
    // something in minimum increments of 0.)

    UnallocatedCString minimum_increment_;

    UnallocatedCString date_;  // (NEW FIELD) The date this offer was added to
                               // the market.

    DEFINE_OT_DYNAMIC_CAST(OfferDataMarket)
};

class BidData : public OfferDataMarket
{
    // You never actually get an instance of this, only its subclasses.
    // Therefore, I don't allow you to access the constructor except through
    // factory.
protected:
    BidData()
        : OfferDataMarket()
    {
        type_ = "BidData";
    }

public:
    ~BidData() override = default;

    using Displayable::gui_label_;  // The label that appears in the GUI

    using OfferDataMarket::available_assets_;
    using OfferDataMarket::date_;
    using OfferDataMarket::minimum_increment_;
    using OfferDataMarket::price_per_scale_;
    using OfferDataMarket::transaction_id_;

    DEFINE_OT_DYNAMIC_CAST(BidData)
};

class AskData : public OfferDataMarket
{
    // You never actually get an instance of this, only its subclasses.
    // Therefore, I don't allow you to access the constructor except through
    // factory.
protected:
    AskData()
        : OfferDataMarket()
    {
        type_ = "AskData";
    }

public:
    ~AskData() override = default;

    using Displayable::gui_label_;  // The label that appears in the GUI

    using OfferDataMarket::available_assets_;
    using OfferDataMarket::date_;
    using OfferDataMarket::minimum_increment_;
    using OfferDataMarket::price_per_scale_;
    using OfferDataMarket::transaction_id_;

    DEFINE_OT_DYNAMIC_CAST(AskData)
};

class OfferListMarket : public Storable
{
    // You never actually get an instance of this, only its subclasses.
    // Therefore, I don't allow you to access the constructor except through
    // factory.
protected:
    OfferListMarket()
        : Storable()
        , list_BidDatas()
        , list_AskDatas()
    {
        type_ = "OfferListMarket";
    }

public:
    ~OfferListMarket() override = default;

    DECLARE_GET_ADD_REMOVE(BidData);
    DECLARE_GET_ADD_REMOVE(AskData);

    DEFINE_OT_DYNAMIC_CAST(OfferListMarket)
};

class TradeDataMarket : public Displayable
{
    // You never actually get an instance of this, only its subclasses.
    // Therefore, I don't allow you to access the constructor except through
    // factory.
protected:
    TradeDataMarket()
        : Displayable()
        , transaction_id_("0")
        , date_("0")
        , price_("0")
        , amount_sold_("0")
    {
        type_ = "TradeDataMarket";
    }

public:
    ~TradeDataMarket() override = default;

    using Displayable::gui_label_;  // The label that appears in the GUI

    UnallocatedCString transaction_id_;  // (transaction number for this trade.)
    UnallocatedCString date_;            // (The date of this trade's execution)
    UnallocatedCString price_;           // (The price this trade executed at.)
    UnallocatedCString amount_sold_;  // (Amount of asset sold for that price.)

    DEFINE_OT_DYNAMIC_CAST(TradeDataMarket)
};

class TradeListMarket : public Storable
{
    // You never actually get an instance of this, only its subclasses.
    // Therefore, I don't allow you to access the constructor except through
    // factory.
protected:
    TradeListMarket()
        : Storable()
        , list_TradeDataMarkets()
    {
        type_ = "TradeListMarket";
    }

public:
    ~TradeListMarket() override = default;

    DECLARE_GET_ADD_REMOVE(TradeDataMarket);

    DEFINE_OT_DYNAMIC_CAST(TradeListMarket)
};

class OfferDataNym : public Displayable
{
    // You never actually get an instance of this, only its subclasses.
    // Therefore, I don't allow you to access the constructor except through
    // factory.
protected:
    OfferDataNym()
        : Displayable()
        , valid_from_("0")
        , valid_to_("0")
        , notary_id_()
        , instrument_definition_id_()
        , asset_acct_id_()
        , currency_type_id_()
        , currency_acct_id_()
        , selling_(false)
        , scale_("1")
        , price_per_scale_("1")
        , transaction_id_("0")
        , total_assets_("1")
        , finished_so_far_("0")
        , minimum_increment_("1")
        , stop_sign_()
        , stop_price_("0")
        , date_("0")
    {
        type_ = "OfferDataNym";
    }

public:
    ~OfferDataNym() override = default;

    using Displayable::gui_label_;  // The label that appears in the GUI

    UnallocatedCString valid_from_;
    UnallocatedCString valid_to_;

    UnallocatedCString notary_id_;
    UnallocatedCString instrument_definition_id_;  // the instrument definition
                                                   // on offer.
    UnallocatedCString asset_acct_id_;     // the account where the asset is.
    UnallocatedCString currency_type_id_;  // the currency being used to
                                           // purchase the asset.
    UnallocatedCString currency_acct_id_;  // the account where currency is.

    bool selling_;  // true for ask, false for bid.

    UnallocatedCString scale_;  // 1oz market? 100oz market? 10,000oz market?
                                // This determines size and granularity.
    UnallocatedCString price_per_scale_;

    UnallocatedCString transaction_id_;

    UnallocatedCString total_assets_;
    UnallocatedCString finished_so_far_;

    // Each sale or purchase against (total_assets - finished_so_far) must be in
    // minimum increments.
    // Minimum Increment must be evenly divisible by scale.
    // (This effectively becomes a "FILL OR KILL" order if set to the same value
    // as total_assets. Also, MUST be 1
    // or greater. CANNOT be zero. Enforce this at class level. You cannot sell
    // something in minimum increments of 0.)

    UnallocatedCString minimum_increment_;

    UnallocatedCString stop_sign_;   // If this is a stop order, this will
                                     // contain
                                     // '<' or
                                     // '>'.
    UnallocatedCString stop_price_;  // The price at which the stop order
                                     // activates (less than X or greater than
                                     // X, based on sign.)

    UnallocatedCString date_;  // (NEW FIELD) The date on which this offer was
                               // added to the market.

    DEFINE_OT_DYNAMIC_CAST(OfferDataNym)
};

class OfferListNym : public Storable
{
    // You never actually get an instance of this, only its subclasses.
    // Therefore, I don't allow you to access the constructor except through
    // factory.
protected:
    OfferListNym()
        : Storable()
        , list_OfferDataNyms()
    {
        type_ = "OfferListNym";
    }

public:
    ~OfferListNym() override = default;

    DECLARE_GET_ADD_REMOVE(OfferDataNym);

    DEFINE_OT_DYNAMIC_CAST(OfferListNym)
};

class TradeDataNym : public Displayable
{
    // You never actually get an instance of this, only its subclasses.
    // Therefore, I don't allow you to access the constructor except through
    // factory.
protected:
    TradeDataNym()
        : Displayable()
        , transaction_id_("0")
        , completed_count_("0")
        , date_("0")
        , price_("0")
        , amount_sold_("0")
        , updated_id_("0")
        , offer_price_("0")
        , finished_so_far_("0")
        , instrument_definition_id_()
        , currency_id_()
        , currency_paid_("0")
        , asset_acct_id_()
        , currency_acct_id_()
        , scale_("1")
        , is_bid_(true)
        , asset_receipt_()
        , currency_receipt_()
        , final_receipt_()
    {
        type_ = "TradeDataNym";
    }

public:
    ~TradeDataNym() override = default;

    using Displayable::gui_label_;  // The label that appears in the GUI

    UnallocatedCString transaction_id_;  // (transaction number for original
                                         // offer.)

    UnallocatedCString completed_count_;  // (How many trades have processed for
                                          // the associated offer? We keep count
                                          // for each trade.)
    UnallocatedCString date_;         // (The date of this trade's execution)
    UnallocatedCString price_;        // (The price this trade executed at.)
    UnallocatedCString amount_sold_;  // (Amount of asset sold for that price.)
    UnallocatedCString updated_id_;   // NEW FIELD (Transaction ID for trade
                                      // receipt.)
    UnallocatedCString offer_price_;  // NEW FIELD (price limit on the original
                                      // offer.)
    UnallocatedCString finished_so_far_;  // NEW FIELD (total amount sold across
                                          // all trades.)
    UnallocatedCString instrument_definition_id_;  // NEW FIELD instrument
                                                   // definition id for trade
    UnallocatedCString currency_id_;    // NEW FIELD currency ID for trade
    UnallocatedCString currency_paid_;  // NEW FIELD currency paid for trade

    UnallocatedCString asset_acct_id_;
    UnallocatedCString currency_acct_id_;

    UnallocatedCString scale_;
    bool is_bid_;

    UnallocatedCString asset_receipt_;     // FYI TradeDataNym is used on the
                                           // client side.
    UnallocatedCString currency_receipt_;  // These variables are set on the
                                           // client side.
    UnallocatedCString final_receipt_;

    DEFINE_OT_DYNAMIC_CAST(TradeDataNym)
};

class TradeListNym : public Storable
{
    // You never actually get an instance of this, only its subclasses.
    // Therefore, I don't allow you to access the constructor except through
    // factory.
protected:
    TradeListNym()
        : Storable()
        , list_TradeDataNyms()
    {
        type_ = "TradeListNym";
    }

public:
    ~TradeListNym() override = default;

    DECLARE_GET_ADD_REMOVE(TradeDataNym);

    DEFINE_OT_DYNAMIC_CAST(TradeListNym)
};

// ACCOUNT (GUI local storage about my own accounts, in my wallet.)

class Acct : public Displayable
{
    // You never actually get an instance of this, only its subclasses.
    // Therefore, I don't allow you to access the constructor except through
    // factory.
protected:
    Acct()
        : Displayable()
        , acct_id_()
        , notary_id_()
    {
        type_ = "Acct";
    }

public:
    ~Acct() override = default;

    using Displayable::gui_label_;  // The label that appears in the GUI

    UnallocatedCString acct_id_;
    UnallocatedCString notary_id_;

    DEFINE_OT_DYNAMIC_CAST(Acct)
};

class BitcoinAcct : public Acct
{
    // You never actually get an instance of this, only its subclasses.
    // Therefore, I don't allow you to access the constructor except through
    // factory.
protected:
    BitcoinAcct()
        : Acct()
        , bitcoin_acct_name_()
    {
        type_ = "BitcoinAcct";
    }

public:
    ~BitcoinAcct() override = default;

    using Displayable::gui_label_;  // The label that appears in the GUI

    using Acct::acct_id_;
    using Acct::notary_id_;

    UnallocatedCString bitcoin_acct_name_;

    DEFINE_OT_DYNAMIC_CAST(BitcoinAcct)
};

// SERVER (GUI local storage about servers.)

class ServerInfo : public Displayable
{
    // You never actually get an instance of this, only its subclasses.
    // Therefore, I don't allow you to access the constructor except through
    // factory.
protected:
    ServerInfo()
        : Displayable()
        , notary_id_()
        , server_type_()
    {
        type_ = "ServerInfo";
    }

public:
    ~ServerInfo() override = default;

    using Displayable::gui_label_;  // The label that appears in the GUI

    UnallocatedCString notary_id_;
    UnallocatedCString server_type_;

    DEFINE_OT_DYNAMIC_CAST(ServerInfo)
};

class Server : public ServerInfo
{
    // You never actually get an instance of this, only its subclasses.
    // Therefore, I don't allow you to access the constructor except through
    // factory.
protected:
    Server()
        : ServerInfo()
        , server_host_()
        , server_port_()
    {
        type_ = "Server";
    }

public:
    ~Server() override = default;

    using Displayable::gui_label_;  // The label that appears in the GUI

    using ServerInfo::notary_id_;    // in base class
    using ServerInfo::server_type_;  // in base class

    UnallocatedCString server_host_;
    UnallocatedCString server_port_;

    DEFINE_OT_DYNAMIC_CAST(Server)
};

class BitcoinServer : public Server
{
    // You never actually get an instance of this, only its subclasses.
    // Therefore, I don't allow you to access the constructor except through
    // factory.
protected:
    BitcoinServer()
        : Server()
        , bitcoin_username_()
        , bitcoin_password_()
    {
        type_ = "BitcoinServer";
    }

public:
    ~BitcoinServer() override = default;

    using Displayable::gui_label_;  // The label that appears in the GUI

    using ServerInfo::notary_id_;    // in base class
    using ServerInfo::server_type_;  // in base class

    using Server::server_host_;  // in base class
    using Server::server_port_;  // in base class

    UnallocatedCString bitcoin_username_;
    UnallocatedCString bitcoin_password_;

    DEFINE_OT_DYNAMIC_CAST(BitcoinServer)
};

class RippleServer : public Server
{
    // You never actually get an instance of this, only its subclasses.
    // Therefore, I don't allow you to access the constructor except through
    // factory.
protected:
    RippleServer()
        : Server()
        , ripple_username_()
        , ripple_password_()
        , namefield_id_()
        , passfield_id_()
    {
        type_ = "RippleServer";
    }

public:
    ~RippleServer() override = default;

    using Displayable::gui_label_;  // The label that appears in the GUI

    using ServerInfo::notary_id_;    // in base class
    using ServerInfo::server_type_;  // in base class

    using Server::server_host_;  // in base class
    using Server::server_port_;  // in base class

    UnallocatedCString ripple_username_;
    UnallocatedCString ripple_password_;

    UnallocatedCString namefield_id_;
    UnallocatedCString passfield_id_;

    DEFINE_OT_DYNAMIC_CAST(RippleServer)
};

class LoomServer : public Server
{
    // You never actually get an instance of this, only its subclasses.
    // Therefore, I don't allow you to access the constructor except through
    // factory.
protected:
    LoomServer()
        : Server()
        , loom_username_()
        , namefield_id_()
    {
        type_ = "LoomServer";
    }

public:
    ~LoomServer() override = default;

    using Displayable::gui_label_;  // The label that appears in the GUI

    using ServerInfo::notary_id_;    // in base class
    using ServerInfo::server_type_;  // in base class

    using Server::server_host_;  // in base class
    using Server::server_port_;  // in base class

    UnallocatedCString loom_username_;

    UnallocatedCString namefield_id_;

    DEFINE_OT_DYNAMIC_CAST(LoomServer)
};

class ContactNym : public Displayable
{
    // You never actually get an instance of this, only its subclasses.
    // Therefore, I don't allow you to access the constructor except through
    // factory.
protected:
    ContactNym()
        : Displayable()
        , nym_type_()
        , nym_id_()
        , public_key_()
        , memo_()
        , list_ServerInfos()
    {
        type_ = "ContactNym";
    }

public:
    ~ContactNym() override;

    using Displayable::gui_label_;  // The label that appears in the GUI

    UnallocatedCString nym_type_;
    UnallocatedCString nym_id_;
    UnallocatedCString public_key_;
    UnallocatedCString memo_;

    DECLARE_GET_ADD_REMOVE(ServerInfo);

    DEFINE_OT_DYNAMIC_CAST(ContactNym)
};

class WalletData : public Storable
{
    // You never actually get an instance of this, only its subclasses.
    // Therefore, I don't allow you to access the constructor except through
    // factory.
protected:
    WalletData()
        : Storable()
        , list_BitcoinServers()
        , list_BitcoinAccts()
        , list_RippleServers()
        , list_LoomServers()
    {
        type_ = "WalletData";
    }

public:
    ~WalletData() override
    {
        std::cout << "WalletData destructor" << std::endl;
    }

    // List of Bitcoin servers
    // List of Bitcoin accounts
    // Loom, etc.

    DECLARE_GET_ADD_REMOVE(BitcoinServer);
    DECLARE_GET_ADD_REMOVE(BitcoinAcct);

    DECLARE_GET_ADD_REMOVE(RippleServer);
    DECLARE_GET_ADD_REMOVE(LoomServer);

    DEFINE_OT_DYNAMIC_CAST(WalletData)
};

class ContactAcct : public Displayable
{
    // You never actually get an instance of this, only its subclasses.
    // Therefore, I don't allow you to access the constructor except through
    // factory.
protected:
    ContactAcct()
        : Displayable()
        , server_type_()
        , notary_id_()
        , instrument_definition_id_()
        , acct_id_()
        , nym_id_()
        , memo_()
        , public_key_()
    {
        type_ = "ContactAcct";
    }

public:
    ~ContactAcct() override = default;

    using Displayable::gui_label_;  // The label that appears in the GUI

    UnallocatedCString server_type_;
    UnallocatedCString notary_id_;
    UnallocatedCString instrument_definition_id_;
    UnallocatedCString acct_id_;
    UnallocatedCString nym_id_;
    UnallocatedCString memo_;
    UnallocatedCString public_key_;

    DEFINE_OT_DYNAMIC_CAST(ContactAcct)
};

class Contact : public Displayable
{
    // You never actually get an instance of this, only its subclasses.
    // Therefore, I don't allow you to access the constructor except through
    // factory.
protected:
    Contact()
        : Displayable()
        , contact_id_()
        , email_()
        , memo_()
        , public_key_()
        , list_ContactNyms()
        , list_ContactAccts()
    {
        type_ = "Contact";
    }

public:
    ~Contact() override;

    using Displayable::gui_label_;  // The label that appears in the GUI

    UnallocatedCString contact_id_;
    UnallocatedCString email_;
    UnallocatedCString memo_;
    UnallocatedCString public_key_;

    DECLARE_GET_ADD_REMOVE(ContactNym);
    DECLARE_GET_ADD_REMOVE(ContactAcct);

    DEFINE_OT_DYNAMIC_CAST(Contact)
};

class AddressBook : public Storable
{
    // You never actually get an instance of this, only its subclasses.
    // Therefore, I don't allow you to access the constructor except through
    // factory.
protected:
    AddressBook()
        : Storable()
        , list_Contacts()
    {
        type_ = "AddressBook";
    }

public:
    ~AddressBook() override;

    DECLARE_GET_ADD_REMOVE(Contact);

    DEFINE_OT_DYNAMIC_CAST(AddressBook)
};
}  // Namespace OTDB

// StorageFS -- FILE-SYSTEM Storage Context
//
//
namespace OTDB
{
// StorageFS means "Storage on Filesystem."
//
// This is the first subclass of OTDB::Storage -- but it won't be the last!
//
class StorageFS : public Storage
{
protected:
    StorageFS();  // You have to use the factory to instantiate (so it can
                  // create
                  // the Packer also.)
    // But from there, however you Init, Store, Query, etc is entirely up to
    // you.

    // Confirms if a file exists.  If it exists at path; return length.
    auto ConstructAndConfirmPath(
        const api::Session& api,
        UnallocatedCString& strOutput,
        const UnallocatedCString& dataFolder,
        const UnallocatedCString& strFolder,
        const UnallocatedCString& oneStr,
        const UnallocatedCString& twoStr,
        const UnallocatedCString& threeStr) -> std::int64_t;

public:
    // Verifies whether path exists AND creates folders where necessary.
    auto ConstructAndCreatePath(
        const api::Session& api,
        UnallocatedCString& strOutput,
        const UnallocatedCString& dataFolder,
        const UnallocatedCString& strFolder,
        const UnallocatedCString& oneStr,
        const UnallocatedCString& twoStr,
        const UnallocatedCString& threeStr) -> std::int64_t;

private:
    auto ConstructAndConfirmPathImp(
        const api::Session& api,
        const bool bMakePath,
        UnallocatedCString& strOutput,
        const UnallocatedCString& dataFolder,
        const UnallocatedCString& zeroStr,
        const UnallocatedCString& oneStr,
        const UnallocatedCString& twoStr,
        const UnallocatedCString& threeStr) -> std::int64_t;

protected:
    // If you wish to make your own subclass of OTDB::Storage, then use
    // StorageFS as an example.
    // The below 6 methods are the only overrides you need to copy.
    //
    auto onStorePackedBuffer(
        const api::Session& api,
        PackedBuffer& theBuffer,
        const UnallocatedCString& dataFolder,
        const UnallocatedCString& strFolder,
        const UnallocatedCString& oneStr,
        const UnallocatedCString& twoStr,
        const UnallocatedCString& threeStr) -> bool override;

    auto onQueryPackedBuffer(
        const api::Session& api,
        PackedBuffer& theBuffer,
        const UnallocatedCString& dataFolder,
        const UnallocatedCString& strFolder,
        const UnallocatedCString& oneStr,
        const UnallocatedCString& twoStr,
        const UnallocatedCString& threeStr) -> bool override;

    auto onStorePlainString(
        const api::Session& api,
        const UnallocatedCString& theBuffer,
        const UnallocatedCString& dataFolder,
        const UnallocatedCString& strFolder,
        const UnallocatedCString& oneStr,
        const UnallocatedCString& twoStr,
        const UnallocatedCString& threeStr) -> bool override;

    auto onQueryPlainString(
        const api::Session& api,
        UnallocatedCString& theBuffer,
        const UnallocatedCString& dataFolder,
        const UnallocatedCString& strFolder,
        const UnallocatedCString& oneStr,
        const UnallocatedCString& twoStr,
        const UnallocatedCString& threeStr) -> bool override;

    auto onEraseValueByKey(
        const api::Session& api,
        const UnallocatedCString& dataFolder,
        const UnallocatedCString& strFolder,
        const UnallocatedCString& oneStr,
        const UnallocatedCString& twoStr,
        const UnallocatedCString& threeStr) -> bool override;

public:
    auto Exists(
        const api::Session& api,
        const UnallocatedCString& dataFolder,
        const UnallocatedCString& strFolder,
        const UnallocatedCString& oneStr,
        const UnallocatedCString& twoStr,
        const UnallocatedCString& threeStr) -> bool override;

    auto FormPathString(
        const api::Session& api,
        UnallocatedCString& strOutput,
        const UnallocatedCString& dataFolder,
        const UnallocatedCString& strFolder,
        const UnallocatedCString& oneStr,
        const UnallocatedCString& twoStr,
        const UnallocatedCString& threeStr) -> std::int64_t override;

    static auto Instantiate() -> StorageFS* { return new StorageFS; }

    ~StorageFS() override;
};
}  // namespace OTDB

// IStorable-derived types...
//
//
// BELOW are the SUBCLASSES of the storable objects that actually get
// INSTANTIATED,
// based on WHICH PACKER is being used.

// If you are adding a new DATA OBJECT, then you probably want to add lines
// below
// for EACH of the different packer types (MsgPack, Protocol Buffers, JSON,
// etc.)
//

#define OT_USING_ISTORABLE_HOOKS                                               \
    using IStorable::hookBeforePack;                                           \
    using IStorable::hookAfterUnpack

}  // namespace opentxs
