// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <cstdint>
#include <iostream>
#include <memory>
#include <utility>

#include "internal/util/LogMacros.hpp"
#include "opentxs/Version.hpp"
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
};

// Currently supporting filesystem, with subclasses possible via API.
//
enum StorageType  // STORAGE TYPE
{
    STORE_FILESYSTEM = 0,  // Filesystem
    STORE_TYPE_SUBCLASS    // (Subclass provided by API client via SWIG.)
};

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
    virtual bool onPack(PackedBuffer& theBuffer, Storable& inObj) = 0;

    // buffer is input, outObj is output.
    virtual bool onUnpack(PackedBuffer& theBuffer, Storable& outObj) = 0;

    // This is called just before packing a storable. (Opportunity to copy
    // values...)
    virtual void hookBeforePack() {}

    // This is called just after unpacking a storable. (Opportunity to copy
    // values...)
    virtual void hookAfterUnpack() {}
};

#define DEFINE_OT_DYNAMIC_CAST(CLASS_NAME)                                     \
    CLASS_NAME* clone() const override                                         \
    {                                                                          \
        std::cout                                                              \
            << "********* THIS SHOULD NEVER HAPPEN!!!!! *****************"     \
            << std::endl;                                                      \
        OT_FAIL;                                                               \
    }                                                                          \
    static CLASS_NAME* ot_dynamic_cast(Storable* pObject)                      \
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
        : m_Type("Storable")
    {
    }

    UnallocatedCString m_Type;

public:
    virtual ~Storable() = default;

    // %ignore spam(uint16_t); API users don't need this function, it's for
    // internal purposes.
    static Storable* Create(StoredObjectType eType, PackType thePackType);

    virtual Storable* clone() const = 0;
    static Storable* ot_dynamic_cast(Storable* pObject)
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

    virtual bool PackString(const UnallocatedCString& theString) = 0;
    virtual bool UnpackString(UnallocatedCString& theString) = 0;

    virtual bool ReadFromIStream(
        std::istream& inStream,
        std::int64_t lFilesize) = 0;
    virtual bool WriteToOStream(std::ostream& outStream) = 0;

    virtual const std::uint8_t* GetData() = 0;
    virtual size_t GetSize() = 0;

    virtual void SetData(const std::uint8_t* pData, size_t theSize) = 0;
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

    static OTPacker* Create(PackType ePackType);

    PackType GetType() const;

    PackedBuffer* Pack(Storable& inObj);
    bool Unpack(PackedBuffer& inBuf, Storable& outObj);

    PackedBuffer* Pack(const UnallocatedCString& inObj);
    bool Unpack(PackedBuffer& inBuf, UnallocatedCString& outObj);

    virtual PackedBuffer* CreateBuffer() = 0;
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

    PackedBuffer* CreateBuffer() override { return new theBufferType; }

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
    OTPacker* m_pPacker{nullptr};

    Storage(Storage&&) = delete;
    Storage& operator=(const Storage&) = delete;
    Storage& operator=(Storage&&) = delete;

protected:
    Storage()
        : m_pPacker(nullptr)
    {
    }

    Storage(const Storage&)
        : m_pPacker(nullptr)
    {
    }  // We don't want to copy the pointer. Let it create its own.

    // This is called once, in the factory.
    void SetPacker(OTPacker& thePacker)
    {
        OT_ASSERT(nullptr == m_pPacker);
        m_pPacker = &thePacker;
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
    virtual bool onStorePackedBuffer(
        const api::Session& api,
        PackedBuffer& theBuffer,
        const UnallocatedCString& dataFolder,
        const UnallocatedCString& strFolder,
        const UnallocatedCString& oneStr,
        const UnallocatedCString& twoStr,
        const UnallocatedCString& threeStr) = 0;

    virtual bool onQueryPackedBuffer(
        const api::Session& api,
        PackedBuffer& theBuffer,
        const UnallocatedCString& dataFolder,
        const UnallocatedCString& strFolder,
        const UnallocatedCString& oneStr,
        const UnallocatedCString& twoStr,
        const UnallocatedCString& threeStr) = 0;

    virtual bool onStorePlainString(
        const api::Session& api,
        const UnallocatedCString& theBuffer,
        const UnallocatedCString& dataFolder,
        const UnallocatedCString& strFolder,
        const UnallocatedCString& oneStr,
        const UnallocatedCString& twoStr,
        const UnallocatedCString& threeStr) = 0;

    virtual bool onQueryPlainString(
        const api::Session& api,
        UnallocatedCString& theBuffer,
        const UnallocatedCString& dataFolder,
        const UnallocatedCString& strFolder,
        const UnallocatedCString& oneStr,
        const UnallocatedCString& twoStr,
        const UnallocatedCString& threeStr) = 0;

    virtual bool onEraseValueByKey(
        const api::Session& api,
        const UnallocatedCString& dataFolder,
        const UnallocatedCString& strFolder,
        const UnallocatedCString& oneStr,
        const UnallocatedCString& twoStr,
        const UnallocatedCString& threeStr) = 0;

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
    OTPacker* GetPacker(PackType ePackType = OTDB_DEFAULT_PACKER);

    // See if the file is there.
    virtual bool Exists(
        const api::Session& api,
        const UnallocatedCString& dataFolder,
        const UnallocatedCString& strFolder,
        const UnallocatedCString& oneStr,
        const UnallocatedCString& twoStr,
        const UnallocatedCString& threeStr) = 0;

    virtual std::int64_t FormPathString(
        const api::Session& api,
        UnallocatedCString& strOutput,
        const UnallocatedCString& dataFolder,
        const UnallocatedCString& strFolder,
        const UnallocatedCString& oneStr,
        const UnallocatedCString& twoStr,
        const UnallocatedCString& threeStr) = 0;

    virtual ~Storage()
    {
        if (nullptr != m_pPacker) delete m_pPacker;
        m_pPacker = nullptr;
    }

    // Store/Retrieve a string.

    bool StoreString(
        const api::Session& api,
        const UnallocatedCString& strContents,
        const UnallocatedCString& dataFolder,
        const UnallocatedCString& strFolder,
        const UnallocatedCString& oneStr,
        const UnallocatedCString& twoStr,
        const UnallocatedCString& threeStr);

    UnallocatedCString QueryString(
        const api::Session& api,
        const UnallocatedCString& dataFolder,
        const UnallocatedCString& strFolder,
        const UnallocatedCString& oneStr,
        const UnallocatedCString& twoStr,
        const UnallocatedCString& threeStr);

    bool StorePlainString(
        const api::Session& api,
        const UnallocatedCString& strContents,
        const UnallocatedCString& dataFolder,
        const UnallocatedCString& strFolder,
        const UnallocatedCString& oneStr,
        const UnallocatedCString& twoStr,
        const UnallocatedCString& threeStr);

    UnallocatedCString QueryPlainString(
        const api::Session& api,
        const UnallocatedCString& dataFolder,
        const UnallocatedCString& strFolder,
        const UnallocatedCString& oneStr,
        const UnallocatedCString& twoStr,
        const UnallocatedCString& threeStr);

    // Store/Retrieve an object. (Storable.)

    bool StoreObject(
        const api::Session& api,
        Storable& theContents,
        const UnallocatedCString& dataFolder,
        const UnallocatedCString& strFolder,
        const UnallocatedCString& oneStr,
        const UnallocatedCString& twoStr,
        const UnallocatedCString& threeStr);

    // Use %newobject OTDB::Storage::QueryObject();
    Storable* QueryObject(
        const api::Session& api,
        const StoredObjectType& theObjectType,
        const UnallocatedCString& dataFolder,
        const UnallocatedCString& strFolder,
        const UnallocatedCString& oneStr,
        const UnallocatedCString& twoStr,
        const UnallocatedCString& threeStr);
    // Store/Retrieve a Storable object inside an Armored object.

    UnallocatedCString EncodeObject(
        const api::Session& api,
        Storable& theContents);

    // Use %newobject OTDB::Storage::DecodeObject();
    Storable* DecodeObject(
        const StoredObjectType& theObjectType,
        const UnallocatedCString& strInput);

    // Erase any value based on its location.

    bool EraseValueByKey(
        const api::Session& api,
        const UnallocatedCString& dataFolder,
        const UnallocatedCString& strFolder,
        const UnallocatedCString& oneStr,
        const UnallocatedCString& twoStr,
        const UnallocatedCString& threeStr);

    // Note:
    // Make sure to use: %newobject Factory::createObj();  IN OTAPI.i file!
    //
    // That way, Java garbage cleanup will handle object after this.
    // (Instead of leaking because it thinks C++ will clean it up.)
    //
    // Factory for Storable objects.   %newobject Factory::createObj();
    Storable* CreateObject(const StoredObjectType& eType);

    // Factory for Storage itself.  %ignore this in OTAPI.i  (It's accessed
    // through
    // a namespace-level function, whereas this is for internal purposes.)
    //
    static Storage* Create(
        const StorageType& eStorageType,
        const PackType& ePackType);  // FACTORY

    StorageType GetType() const;
};

//
// OTDB Namespace PUBLIC INTERFACE
//

bool InitDefaultStorage(const StorageType eStoreType, const PackType ePackType);

// Default Storage instance:
Storage* GetDefaultStorage();

// %newobject Factory::createObj();
Storage* CreateStorageContext(
    const StorageType eStoreType,
    const PackType ePackType = OTDB_DEFAULT_PACKER);

Storable* CreateObject(const StoredObjectType eType);

// BELOW FUNCTIONS use the DEFAULT Storage context for the OTDB Namespace

// Check if the values are good.
//
bool CheckStringsExistInOrder(
    const UnallocatedCString& dataFolder,
    const UnallocatedCString& strFolder,
    const UnallocatedCString& oneStr,
    const UnallocatedCString& twoStr,
    const UnallocatedCString& threeStr,
    const char* szFuncName = nullptr);

// See if the file is there.
//
bool Exists(
    const api::Session& api,
    const UnallocatedCString& dataFolder,
    const UnallocatedCString strFolder,
    const UnallocatedCString oneStr,
    const UnallocatedCString twoStr,
    const UnallocatedCString threeStr);

std::int64_t FormPathString(
    const api::Session& api,
    UnallocatedCString& strOutput,
    const UnallocatedCString& dataFolder,
    const UnallocatedCString& strFolder,
    const UnallocatedCString& oneStr,
    const UnallocatedCString& twoStr,
    const UnallocatedCString& threeStr);
// Store/Retrieve a string.
//
bool StoreString(
    const api::Session& api,
    const UnallocatedCString& strContents,
    const UnallocatedCString& dataFolder,
    const UnallocatedCString& strFolder,
    const UnallocatedCString& oneStr,
    const UnallocatedCString& twoStr,
    const UnallocatedCString& threeStr);

UnallocatedCString QueryString(
    const api::Session& api,
    const UnallocatedCString& dataFolder,
    const UnallocatedCString& strFolder,
    const UnallocatedCString& oneStr,
    const UnallocatedCString& twoStr,
    const UnallocatedCString& threeStr);

bool StorePlainString(
    const api::Session& api,
    const UnallocatedCString& strContents,
    const UnallocatedCString& dataFolder,
    const UnallocatedCString& strFolder,
    const UnallocatedCString& oneStr,
    const UnallocatedCString& twoStr,
    const UnallocatedCString& threeStr);

UnallocatedCString QueryPlainString(
    const api::Session& api,
    const UnallocatedCString& dataFolder,
    const UnallocatedCString& strFolder,
    const UnallocatedCString& oneStr,
    const UnallocatedCString& twoStr,
    const UnallocatedCString& threeStr);

// Store/Retrieve an object. (Storable.)
//
bool StoreObject(
    const api::Session& api,
    Storable& theContents,
    const UnallocatedCString& dataFolder,
    const UnallocatedCString& strFolder,
    const UnallocatedCString& oneStr,
    const UnallocatedCString& twoStr,
    const UnallocatedCString& threeStr);

// Use %newobject OTDB::Storage::Query();
Storable* QueryObject(
    const api::Session& api,
    const StoredObjectType theObjectType,
    const UnallocatedCString& dataFolder,
    const UnallocatedCString& strFolder,
    const UnallocatedCString& oneStr,
    const UnallocatedCString& twoStr,
    const UnallocatedCString& threeStr);

// Store/Retrieve a Storable object inside an Armored object.
UnallocatedCString EncodeObject(const api::Session& api, Storable& theContents);

// Use %newobject OTDB::Storage::DecodeObject();
Storable* DecodeObject(
    const StoredObjectType theObjectType,
    const UnallocatedCString& strInput);

// Erase any value based on its location.

bool EraseValueByKey(
    const api::Session& api,
    const UnallocatedCString& dataFolder,
    const UnallocatedCString& strFolder,
    const UnallocatedCString& oneStr,
    const UnallocatedCString& twoStr,
    const UnallocatedCString& threeStr);

#define DECLARE_GET_ADD_REMOVE(name)                                           \
                                                                               \
protected:                                                                     \
    UnallocatedDeque<std::shared_ptr<name>> list_##name##s;                    \
                                                                               \
public:                                                                        \
    size_t Get##name##Count();                                                 \
    name* Get##name(size_t nIndex);                                            \
    bool Remove##name(size_t nIndex##name);                                    \
    bool Add##name(name& disownObject)

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
        , m_string()
    {
        m_Type = "OTDBString";
    }
    OTDBString(const UnallocatedCString& rhs)
        : Storable()
        , m_string(rhs)
    {
        m_Type = "OTDBString";
    }

public:
    ~OTDBString() override = default;

    UnallocatedCString m_string;

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
        , m_memBuffer()
    {
        m_Type = "Blob";
    }

public:
    ~Blob() override = default;

    UnallocatedVector<std::uint8_t> m_memBuffer;  // Where the actual binary
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
        , the_map()
    {
        m_Type = "StringMap";
    }

public:
    ~StringMap() override = default;

    UnallocatedMap<UnallocatedCString, UnallocatedCString> the_map;

    void SetValue(
        const UnallocatedCString& strKey,
        const UnallocatedCString& strValue)
    {
        auto ii = the_map.find(strKey);
        if (ii != the_map.end()) the_map.erase(ii);
        the_map[strKey] = strValue;
    }

    UnallocatedCString GetValue(const UnallocatedCString& strKey)
    {
        UnallocatedCString ret_val("");
        auto ii = the_map.find(strKey);
        if (ii != the_map.end()) ret_val = (*ii).second;
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
        , gui_label()
    {
        m_Type = "Displayable";
    }

public:
    ~Displayable() override = default;

    UnallocatedCString gui_label;  // The label that appears in the GUI

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
        , notary_id()
        , market_id()
        , instrument_definition_id()
        , currency_type_id()
        , scale("0")
        , total_assets("0")
        , number_bids("0")
        , number_asks()
        , last_sale_price("0")
        , current_bid("0")
        , current_ask("0")
        , volume_trades("0")
        , volume_assets("0")
        , volume_currency("0")
        , recent_highest_bid("0")
        , recent_lowest_ask("0")
        , last_sale_date("0")
    {
        m_Type = "MarketData";
    }

public:
    ~MarketData() override = default;

    using Displayable::gui_label;  // The label that appears in the GUI

    UnallocatedCString notary_id;
    UnallocatedCString market_id;

    UnallocatedCString instrument_definition_id;
    UnallocatedCString currency_type_id;

    UnallocatedCString scale;  // the Market scale. (A trade in any particular
                               // asset is measured in X units of SCALE.)
    // IOW, if the scale is 5000 on the gold market, that means "3 units" is
    // 15000 gold

    UnallocatedCString total_assets;  // total amount of assets available on
                                      // market for purchase.

    UnallocatedCString number_bids;  // number of bids that are currently on the
                                     // market.
    UnallocatedCString number_asks;  // number of asks that are currently on the
                                     // market.

    UnallocatedCString last_sale_price;  // The price at which the most recent
                                         // trade occurred on this market.
    UnallocatedCString current_bid;  // The highest bid currently on the market.
    UnallocatedCString current_ask;  // The lowest ask price currently available
                                     // on the market.

    UnallocatedCString volume_trades;  // 24-hour period, number of trades.

    UnallocatedCString volume_assets;    // 24-hour volume, amount of assets
                                         // traded.
    UnallocatedCString volume_currency;  // 24-hour volume, amount of currency
                                         // paid for assets traded.

    UnallocatedCString recent_highest_bid;  // in a 24hour period, the highest
                                            // bid to hit the market.
    UnallocatedCString recent_lowest_ask;  // in a 24hour period, the lowest ask
                                           // to hit the market.

    UnallocatedCString last_sale_date;  // (NEW FIELD) The date on which the
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
        m_Type = "MarketList";
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
        , transaction_id("0")
        , price_per_scale("1")
        , available_assets("0")
        , minimum_increment("1")
        , date("0")
    {
        m_Type = "OfferDataMarket";
    }

public:
    ~OfferDataMarket() override = default;

    using Displayable::gui_label;  // The label that appears in the GUI

    UnallocatedCString transaction_id;
    UnallocatedCString price_per_scale;
    UnallocatedCString available_assets;

    // Each sale or purchase against (total_assets - finished_so_far) must be in
    // minimum increments.
    // Minimum Increment must be evenly divisible by scale.
    // (This effectively becomes a "FILL OR KILL" order if set to the same value
    // as total_assets. Also, MUST be 1
    // or greater. CANNOT be zero. Enforce this at class level. You cannot sell
    // something in minimum increments of 0.)

    UnallocatedCString minimum_increment;

    UnallocatedCString date;  // (NEW FIELD) The date this offer was added to
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
        m_Type = "BidData";
    }

public:
    ~BidData() override = default;

    using Displayable::gui_label;  // The label that appears in the GUI

    using OfferDataMarket::available_assets;
    using OfferDataMarket::date;
    using OfferDataMarket::minimum_increment;
    using OfferDataMarket::price_per_scale;
    using OfferDataMarket::transaction_id;

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
        m_Type = "AskData";
    }

public:
    ~AskData() override = default;

    using Displayable::gui_label;  // The label that appears in the GUI

    using OfferDataMarket::available_assets;
    using OfferDataMarket::date;
    using OfferDataMarket::minimum_increment;
    using OfferDataMarket::price_per_scale;
    using OfferDataMarket::transaction_id;

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
        m_Type = "OfferListMarket";
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
        , transaction_id("0")
        , date("0")
        , price("0")
        , amount_sold("0")
    {
        m_Type = "TradeDataMarket";
    }

public:
    ~TradeDataMarket() override = default;

    using Displayable::gui_label;  // The label that appears in the GUI

    UnallocatedCString transaction_id;  // (transaction number for this trade.)
    UnallocatedCString date;            // (The date of this trade's execution)
    UnallocatedCString price;           // (The price this trade executed at.)
    UnallocatedCString amount_sold;  // (Amount of asset sold for that price.)

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
        m_Type = "TradeListMarket";
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
        , valid_from("0")
        , valid_to("0")
        , notary_id()
        , instrument_definition_id()
        , asset_acct_id()
        , currency_type_id()
        , currency_acct_id()
        , selling(false)
        , scale("1")
        , price_per_scale("1")
        , transaction_id("0")
        , total_assets("1")
        , finished_so_far("0")
        , minimum_increment("1")
        , stop_sign()
        , stop_price("0")
        , date("0")
    {
        m_Type = "OfferDataNym";
    }

public:
    ~OfferDataNym() override = default;

    using Displayable::gui_label;  // The label that appears in the GUI

    UnallocatedCString valid_from;
    UnallocatedCString valid_to;

    UnallocatedCString notary_id;
    UnallocatedCString instrument_definition_id;  // the instrument definition
                                                  // on offer.
    UnallocatedCString asset_acct_id;     // the account where the asset is.
    UnallocatedCString currency_type_id;  // the currency being used to purchase
                                          // the asset.
    UnallocatedCString currency_acct_id;  // the account where currency is.

    bool selling;  // true for ask, false for bid.

    UnallocatedCString scale;  // 1oz market? 100oz market? 10,000oz market?
                               // This determines size and granularity.
    UnallocatedCString price_per_scale;

    UnallocatedCString transaction_id;

    UnallocatedCString total_assets;
    UnallocatedCString finished_so_far;

    // Each sale or purchase against (total_assets - finished_so_far) must be in
    // minimum increments.
    // Minimum Increment must be evenly divisible by scale.
    // (This effectively becomes a "FILL OR KILL" order if set to the same value
    // as total_assets. Also, MUST be 1
    // or greater. CANNOT be zero. Enforce this at class level. You cannot sell
    // something in minimum increments of 0.)

    UnallocatedCString minimum_increment;

    UnallocatedCString stop_sign;  // If this is a stop order, this will contain
                                   // '<' or
                                   // '>'.
    UnallocatedCString stop_price;  // The price at which the stop order
                                    // activates (less than X or greater than X,
                                    // based on sign.)

    UnallocatedCString date;  // (NEW FIELD) The date on which this offer was
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
        m_Type = "OfferListNym";
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
        , transaction_id("0")
        , completed_count("0")
        , date("0")
        , price("0")
        , amount_sold("0")
        , updated_id("0")
        , offer_price("0")
        , finished_so_far("0")
        , instrument_definition_id()
        , currency_id()
        , currency_paid("0")
        , asset_acct_id()
        , currency_acct_id()
        , scale("1")
        , is_bid(true)
        , asset_receipt()
        , currency_receipt()
        , final_receipt()
    {
        m_Type = "TradeDataNym";
    }

public:
    ~TradeDataNym() override = default;

    using Displayable::gui_label;  // The label that appears in the GUI

    UnallocatedCString transaction_id;  // (transaction number for original
                                        // offer.)

    UnallocatedCString completed_count;  // (How many trades have processed for
                                         // the associated offer? We keep count
                                         // for each trade.)
    UnallocatedCString date;             // (The date of this trade's execution)
    UnallocatedCString price;            // (The price this trade executed at.)
    UnallocatedCString amount_sold;  // (Amount of asset sold for that price.)
    UnallocatedCString updated_id;   // NEW FIELD (Transaction ID for trade
                                     // receipt.)
    UnallocatedCString offer_price;  // NEW FIELD (price limit on the original
                                     // offer.)
    UnallocatedCString finished_so_far;  // NEW FIELD (total amount sold across
                                         // all trades.)
    UnallocatedCString instrument_definition_id;  // NEW FIELD instrument
                                                  // definition id for trade
    UnallocatedCString currency_id;    // NEW FIELD currency ID for trade
    UnallocatedCString currency_paid;  // NEW FIELD currency paid for trade

    UnallocatedCString asset_acct_id;
    UnallocatedCString currency_acct_id;

    UnallocatedCString scale;
    bool is_bid;

    UnallocatedCString asset_receipt;  // FYI TradeDataNym is used on the client
                                       // side.
    UnallocatedCString currency_receipt;  // These variables are set on the
                                          // client side.
    UnallocatedCString final_receipt;

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
        m_Type = "TradeListNym";
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
        , acct_id()
        , notary_id()
    {
        m_Type = "Acct";
    }

public:
    ~Acct() override = default;

    using Displayable::gui_label;  // The label that appears in the GUI

    UnallocatedCString acct_id;
    UnallocatedCString notary_id;

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
        , bitcoin_acct_name()
    {
        m_Type = "BitcoinAcct";
    }

public:
    ~BitcoinAcct() override = default;

    using Displayable::gui_label;  // The label that appears in the GUI

    using Acct::acct_id;
    using Acct::notary_id;

    UnallocatedCString bitcoin_acct_name;

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
        , notary_id()
        , server_type()
    {
        m_Type = "ServerInfo";
    }

public:
    ~ServerInfo() override = default;

    using Displayable::gui_label;  // The label that appears in the GUI

    UnallocatedCString notary_id;
    UnallocatedCString server_type;

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
        , server_host()
        , server_port()
    {
        m_Type = "Server";
    }

public:
    ~Server() override = default;

    using Displayable::gui_label;  // The label that appears in the GUI

    using ServerInfo::notary_id;    // in base class
    using ServerInfo::server_type;  // in base class

    UnallocatedCString server_host;
    UnallocatedCString server_port;

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
        , bitcoin_username()
        , bitcoin_password()
    {
        m_Type = "BitcoinServer";
    }

public:
    ~BitcoinServer() override = default;

    using Displayable::gui_label;  // The label that appears in the GUI

    using ServerInfo::notary_id;    // in base class
    using ServerInfo::server_type;  // in base class

    using Server::server_host;  // in base class
    using Server::server_port;  // in base class

    UnallocatedCString bitcoin_username;
    UnallocatedCString bitcoin_password;

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
        , ripple_username()
        , ripple_password()
        , namefield_id()
        , passfield_id()
    {
        m_Type = "RippleServer";
    }

public:
    ~RippleServer() override = default;

    using Displayable::gui_label;  // The label that appears in the GUI

    using ServerInfo::notary_id;    // in base class
    using ServerInfo::server_type;  // in base class

    using Server::server_host;  // in base class
    using Server::server_port;  // in base class

    UnallocatedCString ripple_username;
    UnallocatedCString ripple_password;

    UnallocatedCString namefield_id;
    UnallocatedCString passfield_id;

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
        , loom_username()
        , namefield_id()
    {
        m_Type = "LoomServer";
    }

public:
    ~LoomServer() override = default;

    using Displayable::gui_label;  // The label that appears in the GUI

    using ServerInfo::notary_id;    // in base class
    using ServerInfo::server_type;  // in base class

    using Server::server_host;  // in base class
    using Server::server_port;  // in base class

    UnallocatedCString loom_username;

    UnallocatedCString namefield_id;

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
        , nym_type()
        , nym_id()
        , public_key()
        , memo()
        , list_ServerInfos()
    {
        m_Type = "ContactNym";
    }

public:
    ~ContactNym() override;

    using Displayable::gui_label;  // The label that appears in the GUI

    UnallocatedCString nym_type;
    UnallocatedCString nym_id;
    UnallocatedCString public_key;
    UnallocatedCString memo;

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
        m_Type = "WalletData";
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
        , server_type()
        , notary_id()
        , instrument_definition_id()
        , acct_id()
        , nym_id()
        , memo()
        , public_key()
    {
        m_Type = "ContactAcct";
    }

public:
    ~ContactAcct() override = default;

    using Displayable::gui_label;  // The label that appears in the GUI

    UnallocatedCString server_type;
    UnallocatedCString notary_id;
    UnallocatedCString instrument_definition_id;
    UnallocatedCString acct_id;
    UnallocatedCString nym_id;
    UnallocatedCString memo;
    UnallocatedCString public_key;

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
        , contact_id()
        , email()
        , memo()
        , public_key()
        , list_ContactNyms()
        , list_ContactAccts()
    {
        m_Type = "Contact";
    }

public:
    ~Contact() override;

    using Displayable::gui_label;  // The label that appears in the GUI

    UnallocatedCString contact_id;
    UnallocatedCString email;
    UnallocatedCString memo;
    UnallocatedCString public_key;

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
        m_Type = "AddressBook";
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
    std::int64_t ConstructAndConfirmPath(
        const api::Session& api,
        UnallocatedCString& strOutput,
        const UnallocatedCString& dataFolder,
        const UnallocatedCString& strFolder,
        const UnallocatedCString& oneStr,
        const UnallocatedCString& twoStr,
        const UnallocatedCString& threeStr);

public:
    // Verifies whether path exists AND creates folders where necessary.
    std::int64_t ConstructAndCreatePath(
        const api::Session& api,
        UnallocatedCString& strOutput,
        const UnallocatedCString& dataFolder,
        const UnallocatedCString& strFolder,
        const UnallocatedCString& oneStr,
        const UnallocatedCString& twoStr,
        const UnallocatedCString& threeStr);

private:
    std::int64_t ConstructAndConfirmPathImp(
        const api::Session& api,
        const bool bMakePath,
        UnallocatedCString& strOutput,
        const UnallocatedCString& dataFolder,
        const UnallocatedCString& zeroStr,
        const UnallocatedCString& oneStr,
        const UnallocatedCString& twoStr,
        const UnallocatedCString& threeStr);

protected:
    // If you wish to make your own subclass of OTDB::Storage, then use
    // StorageFS as an example.
    // The below 6 methods are the only overrides you need to copy.
    //
    bool onStorePackedBuffer(
        const api::Session& api,
        PackedBuffer& theBuffer,
        const UnallocatedCString& dataFolder,
        const UnallocatedCString& strFolder,
        const UnallocatedCString& oneStr,
        const UnallocatedCString& twoStr,
        const UnallocatedCString& threeStr) override;

    bool onQueryPackedBuffer(
        const api::Session& api,
        PackedBuffer& theBuffer,
        const UnallocatedCString& dataFolder,
        const UnallocatedCString& strFolder,
        const UnallocatedCString& oneStr,
        const UnallocatedCString& twoStr,
        const UnallocatedCString& threeStr) override;

    bool onStorePlainString(
        const api::Session& api,
        const UnallocatedCString& theBuffer,
        const UnallocatedCString& dataFolder,
        const UnallocatedCString& strFolder,
        const UnallocatedCString& oneStr,
        const UnallocatedCString& twoStr,
        const UnallocatedCString& threeStr) override;

    bool onQueryPlainString(
        const api::Session& api,
        UnallocatedCString& theBuffer,
        const UnallocatedCString& dataFolder,
        const UnallocatedCString& strFolder,
        const UnallocatedCString& oneStr,
        const UnallocatedCString& twoStr,
        const UnallocatedCString& threeStr) override;

    bool onEraseValueByKey(
        const api::Session& api,
        const UnallocatedCString& dataFolder,
        const UnallocatedCString& strFolder,
        const UnallocatedCString& oneStr,
        const UnallocatedCString& twoStr,
        const UnallocatedCString& threeStr) override;

public:
    bool Exists(
        const api::Session& api,
        const UnallocatedCString& dataFolder,
        const UnallocatedCString& strFolder,
        const UnallocatedCString& oneStr,
        const UnallocatedCString& twoStr,
        const UnallocatedCString& threeStr) override;

    std::int64_t FormPathString(
        const api::Session& api,
        UnallocatedCString& strOutput,
        const UnallocatedCString& dataFolder,
        const UnallocatedCString& strFolder,
        const UnallocatedCString& oneStr,
        const UnallocatedCString& twoStr,
        const UnallocatedCString& threeStr) override;

    static StorageFS* Instantiate() { return new StorageFS; }

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
