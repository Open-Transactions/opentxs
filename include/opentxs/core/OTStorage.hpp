/************************************************************
 *
 *  OTStorage.hpp
 *
 */

/************************************************************
 -----BEGIN PGP SIGNED MESSAGE-----
 Hash: SHA1

 *                 OPEN TRANSACTIONS
 *
 *       Financial Cryptography and Digital Cash
 *       Library, Protocol, API, Server, CLI, GUI
 *
 *       -- Anonymous Numbered Accounts.
 *       -- Untraceable Digital Cash.
 *       -- Triple-Signed Receipts.
 *       -- Cheques, Vouchers, Transfers, Inboxes.
 *       -- Basket Currencies, Markets, Payment Plans.
 *       -- Signed, XML, Ricardian-style Contracts.
 *       -- Scripted smart contracts.
 *
 *  Copyright (C) 2010-2013 by "Fellow Traveler" (A pseudonym)
 *
 *  EMAIL:
 *  FellowTraveler@rayservers.net
 *
 *  BITCOIN:  1NtTPVVjDsUfDWybS4BwvHpG2pdS9RnYyQ
 *
 *  KEY FINGERPRINT (PGP Key in license file):
 *  9DD5 90EB 9292 4B48 0484  7910 0308 00ED F951 BB8E
 *
 *  OFFICIAL PROJECT WIKI(s):
 *  https://github.com/FellowTraveler/Moneychanger
 *  https://github.com/FellowTraveler/Open-Transactions/wiki
 *
 *  WEBSITE:
 *  http://www.OpenTransactions.org/
 *
 *  Components and licensing:
 *   -- Moneychanger..A Java client GUI.....LICENSE:.....GPLv3
 *   -- otlib.........A class library.......LICENSE:...LAGPLv3
 *   -- otapi.........A client API..........LICENSE:...LAGPLv3
 *   -- opentxs/ot....Command-line client...LICENSE:...LAGPLv3
 *   -- otserver......Server Application....LICENSE:....AGPLv3
 *  Github.com/FellowTraveler/Open-Transactions/wiki/Components
 *
 *  All of the above OT components were designed and written by
 *  Fellow Traveler, with the exception of Moneychanger, which
 *  was contracted out to Vicky C (bitcointrader4@gmail.com).
 *  The open-source community has since actively contributed.
 *
 *  -----------------------------------------------------
 *
 *   LICENSE:
 *   This program is free software: you can redistribute it
 *   and/or modify it under the terms of the GNU Affero
 *   General Public License as published by the Free Software
 *   Foundation, either version 3 of the License, or (at your
 *   option) any later version.
 *
 *   ADDITIONAL PERMISSION under the GNU Affero GPL version 3
 *   section 7: (This paragraph applies only to the LAGPLv3
 *   components listed above.) If you modify this Program, or
 *   any covered work, by linking or combining it with other
 *   code, such other code is not for that reason alone subject
 *   to any of the requirements of the GNU Affero GPL version 3.
 *   (==> This means if you are only using the OT API, then you
 *   don't have to open-source your code--only your changes to
 *   Open-Transactions itself must be open source. Similar to
 *   LGPLv3, except it applies to software-as-a-service, not
 *   just to distributing binaries.)
 *
 *   Extra WAIVER for OpenSSL, Lucre, and all other libraries
 *   used by Open Transactions: This program is released under
 *   the AGPL with the additional exemption that compiling,
 *   linking, and/or using OpenSSL is allowed. The same is true
 *   for any other open source libraries included in this
 *   project: complete waiver from the AGPL is hereby granted to
 *   compile, link, and/or use them with Open-Transactions,
 *   according to their own terms, as long as the rest of the
 *   Open-Transactions terms remain respected, with regard to
 *   the Open-Transactions code itself.
 *
 *   Lucre License:
 *   This code is also "dual-license", meaning that Ben Lau-
 *   rie's license must also be included and respected, since
 *   the code for Lucre is also included with Open Transactions.
 *   See Open-Transactions/src/otlib/lucre/LUCRE_LICENSE.txt
 *   The Laurie requirements are light, but if there is any
 *   problem with his license, simply remove the Lucre code.
 *   Although there are no other blind token algorithms in Open
 *   Transactions (yet. credlib is coming), the other functions
 *   will continue to operate.
 *   See Lucre on Github:  https://github.com/benlaurie/lucre
 *   -----------------------------------------------------
 *   You should have received a copy of the GNU Affero General
 *   Public License along with this program.  If not, see:
 *   http://www.gnu.org/licenses/
 *
 *   If you would like to use this software outside of the free
 *   software license, please contact FellowTraveler.
 *   (Unfortunately many will run anonymously and untraceably,
 *   so who could really stop them?)
 *
 *   DISCLAIMER:
 *   This program is distributed in the hope that it will be
 *   useful, but WITHOUT ANY WARRANTY; without even the implied
 *   warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *   PURPOSE.  See the GNU Affero General Public License for
 *   more details.

 -----BEGIN PGP SIGNATURE-----
 Version: GnuPG v1.4.9 (Darwin)

 iQIcBAEBAgAGBQJRSsfJAAoJEAMIAO35UbuOQT8P/RJbka8etf7wbxdHQNAY+2cC
 vDf8J3X8VI+pwMqv6wgTVy17venMZJa4I4ikXD/MRyWV1XbTG0mBXk/7AZk7Rexk
 KTvL/U1kWiez6+8XXLye+k2JNM6v7eej8xMrqEcO0ZArh/DsLoIn1y8p8qjBI7+m
 aE7lhstDiD0z8mwRRLKFLN2IH5rAFaZZUvj5ERJaoYUKdn4c+RcQVei2YOl4T0FU
 LWND3YLoH8naqJXkaOKEN4UfJINCwxhe5Ke9wyfLWLUO7NamRkWD2T7CJ0xocnD1
 sjAzlVGNgaFDRflfIF4QhBx1Ddl6wwhJfw+d08bjqblSq8aXDkmFA7HeunSFKkdn
 oIEOEgyj+veuOMRJC5pnBJ9vV+7qRdDKQWaCKotynt4sWJDGQ9kWGWm74SsNaduN
 TPMyr9kNmGsfR69Q2Zq/FLcLX/j8ESxU+HYUB4vaARw2xEOu2xwDDv6jt0j3Vqsg
 x7rWv4S/Eh18FDNDkVRChiNoOIilLYLL6c38uMf1pnItBuxP3uhgY6COm59kVaRh
 nyGTYCDYD2TK+fI9o89F1297uDCwEJ62U0Q7iTDp5QuXCoxkPfv8/kX6lS6T3y9G
 M9mqIoLbIQ1EDntFv7/t6fUTS2+46uCrdZWbQ5RjYXdrzjij02nDmJAm2BngnZvd
 kamH0Y/n11lCvo1oQxM+
 =uSzz
 -----END PGP SIGNATURE-----
 **************************************************************/

#ifndef OPENTXS_CORE_OTSTORAGE_HPP
#define OPENTXS_CORE_OTSTORAGE_HPP

#ifndef SWIG

#include "util/Assert.hpp"
#include "opentxs/core/OTData.hpp"
#include "containers/simple_ptr.hpp"

#include <deque>
#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <cstdint>

#define OTDB_PROTOCOL_BUFFERS 1

#define OTDB_DEFAULT_PACKER OTDB::PACK_PROTOCOL_BUFFERS
#define OTDB_DEFAULT_STORAGE OTDB::STORE_FILESYSTEM

// JAVA-STYLE INTERFACES.
//
// I'm doing some crazy stuff in this file.
// What you see below is a set of preprocessor definitions that
// allow me to use "Java-Style Interfaces" here in C++.
//
// It turns out that Storable needed a normal class hierarchy, AND Java-
// style interfaces, to do everything I wanted it to do.
//
// I will probably create a more general-purpose header file for OT
// and these sorts of #defines will probably end up there int64_t-term.
// Much of OT might be separable out into a more general-purpose utility
// lib, which I will get to whenever it is more important than anything else.
//
#define OTInterface class

#define DeclareInterface(name)                                                 \
    OTInterface name                                                           \
    {                                                                          \
    public:                                                                    \
        virtual ~name()                                                        \
        {                                                                      \
        }

#define DeclareBasedInterface(name, base)                                      \
    class name : public base                                                   \
    {                                                                          \
    public:                                                                    \
        virtual ~name()                                                        \
        {                                                                      \
        }

#define EndInterface                                                           \
    }                                                                          \
    ;

#define implements public

#endif // (not) SWIG

namespace opentxs
{

namespace OTDB
{

// ENUMS:    PackType, StorageType, and StoredObjectType.

// Currently supporting MsgPack and Protocol Buffers.
//
enum PackType            // PACKING TYPE
{ PACK_MESSAGE_PACK = 0, // Using MessagePack as packer.
  PACK_PROTOCOL_BUFFERS, // Using Google Protocol Buffers as packer.
  PACK_TYPE_ERROR        // (Should never be.)
};

// Currently supporting filesystem, with subclasses possible via API.
//
enum StorageType        // STORAGE TYPE
{ STORE_FILESYSTEM = 0, // Filesystem
  STORE_TYPE_SUBCLASS   // (Subclass provided by API client via SWIG.)
};

#ifndef SWIG
extern const char* StoredObjectTypeStrings[];
#endif // (not) SWIG

enum StoredObjectType {
    STORED_OBJ_STRING = 0, // Just a string.
    STORED_OBJ_BLOB, // Used for storing binary data. Bytes of arbitrary length.
    STORED_OBJ_STRING_MAP,   // A StringMap is a list of Key/Value pairs, useful
                             // for storing nearly anything.
    STORED_OBJ_WALLET_DATA,  // The GUI wallet's stored data
    STORED_OBJ_BITCOIN_ACCT, // The GUI wallet's stored data about a Bitcoin
                             // acct
    STORED_OBJ_BITCOIN_SERVER, // The GUI wallet's stored data about a Bitcoin
                               // RPC port.
    STORED_OBJ_RIPPLE_SERVER,  // The GUI wallet's stored data about a Ripple
                               // server.
    STORED_OBJ_LOOM_SERVER, // The GUI wallet's stored data about a Loom server.
    STORED_OBJ_SERVER_INFO, // A Nym has a list of these.
    STORED_OBJ_CONTACT_NYM, // This is a Nym record inside a contact of your
                            // address book.
    STORED_OBJ_CONTACT_ACCT, // This is an account record inside a contact of
                             // your address book.
    STORED_OBJ_CONTACT,      // Your address book has a list of these.
    STORED_OBJ_ADDRESS_BOOK, // Your address book.
    STORED_OBJ_MARKET_DATA,  // The description data for any given Market ID.
    STORED_OBJ_MARKET_LIST,  // A list of MarketDatas.
    STORED_OBJ_BID_DATA,     // Offer details (doesn't contain private details)
    STORED_OBJ_ASK_DATA,     // Offer details (doesn't contain private details)
    STORED_OBJ_OFFER_LIST_MARKET, // A list of offer details, for a specific
                                  // market.
    STORED_OBJ_TRADE_DATA_MARKET, // Trade details (doesn't contain private
                                  // data)
    STORED_OBJ_TRADE_LIST_MARKET, // A list of trade details, for a specific
                                  // market.
    STORED_OBJ_OFFER_DATA_NYM, // Private offer details for a particular Nym and
                               // Offer.
    STORED_OBJ_OFFER_LIST_NYM, // A list of private offer details for a
                               // particular Nym.
    STORED_OBJ_TRADE_DATA_NYM, // Private trade details for a particular Nym and
                               // Trade.
    STORED_OBJ_TRADE_LIST_NYM, // A list of private trade details for a
                               // particular Nym and Offer.
    STORED_OBJ_ERROR           // (Should never be.)
};

#ifndef SWIG

// ABSTRACT BASE CLASSES
//
class Storable; // A storable object
class OTPacker; // A packer (Could be MsgPack, or Google Protocol Buffers, or a
                // json lib...)
class Storage;  // A storage context (database, filesystem, cloud, etc.
                // Swappable.)
class PackedBuffer; // A buffer for containing a PACKED STORABLE. (On its way
                    // to/from storage.)

// OTDB NAMESPACE "CONSTRUCTOR"
//
class InitOTDBDetails
{
public:
    InitOTDBDetails();  // See implementation of this in CPP file for namespace
                        // construction.
    ~InitOTDBDetails(); // Ditto.
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
typedef Storable*(InstantiateFunc)(); // Each storable has one of these as a
                                      // static method.
typedef std::pair<PackType, StoredObjectType> InstantiateFuncKey; // Those
                                                                  // methods are
                                                                  // stored as
                                                                  // function
                                                                  // pointers
                                                                  // here, and
                                                                  // they are
// indexed by Pack Type and Stored Object Type. So if you know "LoomAcct" and
// "protocol buffers", those form the KEY for looking up the LoomAcctPB
// instantiator.
typedef std::map<InstantiateFuncKey, InstantiateFunc*>
    mapOfFunctions; //...basically implementing my own vtable, eh?

// OTDB Namespace PRIVATE MEMBERS
// this "details" naming is a common C++ idiom for "private" in a namespace.
//
namespace details
{
extern OTDB::Storage* s_pStorage;

extern OTDB::mapOfFunctions* pFunctionMap; // This is a pointer so I can control
                                           // what order it is created in, on
                                           // startup.
}

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
DeclareInterface(IStorable) virtual bool onPack(
    PackedBuffer& theBuffer,
    Storable& inObj) = 0; // buffer is output, inObj is input.
virtual bool onUnpack(PackedBuffer& theBuffer,
                      Storable& outObj) = 0; // buffer is input, outObj is
                                             // output.
virtual void hookBeforePack()
{
} // This is called just before packing a storable. (Opportunity to copy
  // values...)
virtual void hookAfterUnpack()
{
} // This is called just after unpacking a storable. (Opportunity to copy
  // values...)
EndInterface

#endif // (not) SWIG

#ifdef SWIG
#define DEFINE_OT_DYNAMIC_CAST(CLASS_NAME_A)                                   \
    CLASS_NAME_A* clone() const                                                \
    {                                                                          \
        std::cerr                                                              \
            << "********* THIS SHOULD NEVER HAPPEN!!!!! *****************"     \
            << std::endl;                                                      \
        return nullptr;                                                        \
    }                                                                          \
    static CLASS_NAME_A* ot_dynamic_cast(Storable* pObject)                    \
    {                                                                          \
        return dynamic_cast<CLASS_NAME_A*>(pObject);                           \
    }
#endif // SWIG

#ifndef SWIG
#define DEFINE_OT_DYNAMIC_CAST(CLASS_NAME)                                     \
    virtual CLASS_NAME* clone() const                                          \
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
#endif // (not) SWIG

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

    std::string m_Type;

public:
    virtual ~Storable()
    {
    }

    // %ignore spam(uint16_t); API users don't need this function, it's for
    // internal purposes.
    EXPORT static Storable* Create(StoredObjectType eType,
                                   PackType thePackType);

#if !defined(__clang__) && !defined(_WIN32)
// -Wuseless-cast does not exist in clang and windows
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wuseless-cast"
#endif

    DEFINE_OT_DYNAMIC_CAST(Storable)

#if !defined(_WIN32) && !defined(__clang__)
#pragma GCC diagnostic pop
#endif
};

#ifndef SWIG

// PACKED BUFFER (for storing PACKED DATA)
//
// %ignore these classes (I think)
//

class PackedBuffer
{
protected:
    PackedBuffer()
    {
    } // Only subclasses of this should be instantiated.
public:
    virtual ~PackedBuffer()
    {
    }

    virtual bool PackString(std::string& theString) = 0;
    virtual bool UnpackString(std::string& theString) = 0;

    virtual bool ReadFromIStream(std::istream& inStream, int64_t lFilesize) = 0;
    virtual bool WriteToOStream(std::ostream& outStream) = 0;

    virtual const uint8_t* GetData() = 0;
    virtual size_t GetSize() = 0;

    virtual void SetData(const uint8_t* pData, size_t theSize) = 0;
};

// PACKER (now OTPacker since MsgPack also has a "Packer" in a #define).
//
// abstract base class for a packer
//

// %ignore spam(uint16_t);  (probably for all packers.)
class OTPacker
{
protected:
    OTPacker()
    {
    }

public:
    virtual ~OTPacker()
    {
    }

    static OTPacker* Create(PackType ePackType);

    PackType GetType() const;

    PackedBuffer* Pack(Storable& inObj);
    EXPORT bool Unpack(PackedBuffer& inBuf, Storable& outObj);

    PackedBuffer* Pack(std::string& inObj);
    bool Unpack(PackedBuffer& inBuf, std::string& outObj);

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
    virtual ~PackerSubclass()
    {
    }

    virtual PackedBuffer* CreateBuffer()
    {
        return new theBufferType;
    }

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
#endif

//
// STORAGE  -- abstract base class
//
class Storage
{
private:
    OTPacker* m_pPacker;

protected:
    Storage()
        : m_pPacker(nullptr)
    {
    }

    Storage(const Storage&)
        : m_pPacker(nullptr)
    {
    } // We don't want to copy the pointer. Let it create its own.

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
    virtual bool onStorePackedBuffer(PackedBuffer& theBuffer,
                                     std::string strFolder,
                                     std::string oneStr = "",
                                     std::string twoStr = "",
                                     std::string threeStr = "") = 0;

    virtual bool onQueryPackedBuffer(PackedBuffer& theBuffer,
                                     std::string strFolder,
                                     std::string oneStr = "",
                                     std::string twoStr = "",
                                     std::string threeStr = "") = 0;

    virtual bool onStorePlainString(std::string& theBuffer,
                                    std::string strFolder,
                                    std::string oneStr = "",
                                    std::string twoStr = "",
                                    std::string threeStr = "") = 0;

    virtual bool onQueryPlainString(std::string& theBuffer,
                                    std::string strFolder,
                                    std::string oneStr = "",
                                    std::string twoStr = "",
                                    std::string threeStr = "") = 0;

    virtual bool onEraseValueByKey(std::string strFolder,
                                   std::string oneStr = "",
                                   std::string twoStr = "",
                                   std::string threeStr = "") = 0;

public:
    // Use GetPacker() to access the Packer, throughout duration of this Storage
    // object.
    // If it doesn't exist yet, this function will create it on the first call.
    // (The
    // parameter allows you the choose what type will be created, other than
    // default.)
    //
    // This way, whenever using an OT Storage, you KNOW the packer is always the
    // right
    // one, and that you don't have to fiddle with it at all. You can also
    // therefore use
    // it for creating instances of various Storables and PackedBuffers, and
    // knowing
    // that the right types will be instantiated automatically, with the buffer
    // being
    // the appropriate subclass for the packer.
    //
    EXPORT OTPacker* GetPacker(PackType ePackType = OTDB_DEFAULT_PACKER);

    // See if the file is there.
    virtual bool Exists(std::string strFolder, std::string oneStr = "",
                        std::string twoStr = "", std::string threeStr = "") = 0;

    virtual int64_t FormPathString(std::string& strOutput,
                                   std::string strFolder,
                                   std::string oneStr = "",
                                   std::string twoStr = "",
                                   std::string threeStr = "") = 0;

    virtual ~Storage()
    {
        if (nullptr != m_pPacker) delete m_pPacker;
        m_pPacker = nullptr;
    }

    // Store/Retrieve a string.

    EXPORT bool StoreString(std::string strContents, std::string strFolder,
                            std::string oneStr = "", std::string twoStr = "",
                            std::string threeStr = "");

    EXPORT std::string QueryString(std::string strFolder,
                                   std::string oneStr = "",
                                   std::string twoStr = "",
                                   std::string threeStr = "");

    EXPORT bool StorePlainString(std::string strContents, std::string strFolder,
                                 std::string oneStr = "",
                                 std::string twoStr = "",
                                 std::string threeStr = "");

    EXPORT std::string QueryPlainString(std::string strFolder,
                                        std::string oneStr = "",
                                        std::string twoStr = "",
                                        std::string threeStr = "");

    // Store/Retrieve an object. (Storable.)

    EXPORT bool StoreObject(Storable& theContents, std::string strFolder,
                            std::string oneStr = "", std::string twoStr = "",
                            std::string threeStr = "");

    // Use %newobject OTDB::Storage::QueryObject();
    EXPORT Storable* QueryObject(StoredObjectType theObjectType,
                                 std::string strFolder, std::string oneStr = "",
                                 std::string twoStr = "",
                                 std::string threeStr = "");
    // Store/Retrieve a Storable object inside an OTASCIIArmor object.

    EXPORT std::string EncodeObject(Storable& theContents);

    // Use %newobject OTDB::Storage::DecodeObject();
    EXPORT Storable* DecodeObject(StoredObjectType theObjectType,
                                  std::string strInput);

    // Erase any value based on its location.

    EXPORT bool EraseValueByKey(std::string strFolder, std::string oneStr = "",
                                std::string twoStr = "",
                                std::string threeStr = "");

    // Note:
    // Make sure to use: %newobject Factory::createObj();  IN OTAPI.i file!
    //
    // That way, Java garbage cleanup will handle object after this.
    // (Instead of leaking because it thinks C++ will clean it up.)
    //
    // Factory for Storable objects.   %newobject Factory::createObj();
    EXPORT Storable* CreateObject(StoredObjectType eType);

    // Factory for Storage itself.  %ignore this in OTAPI.i  (It's accessed
    // through
    // a namespace-level function, whereas this is for internal purposes.)
    //
    EXPORT static Storage* Create(StorageType eStorageType,
                                  PackType ePackType); // FACTORY

    EXPORT StorageType GetType() const;
};

//
// OTDB Namespace PUBLIC INTERFACE
//

EXPORT bool InitDefaultStorage(StorageType eStoreType, PackType ePackType);

// Default Storage instance:
EXPORT Storage* GetDefaultStorage();

// %newobject Factory::createObj();
EXPORT Storage* CreateStorageContext(StorageType eStoreType,
                                     PackType ePackType = OTDB_DEFAULT_PACKER);

EXPORT Storable* CreateObject(StoredObjectType eType);

// BELOW FUNCTIONS use the DEFAULT Storage context for the OTDB Namespace

// Check if the values are good.
//
EXPORT bool CheckStringsExistInOrder(std::string& strFolder,
                                     std::string& oneStr, std::string& twoStr,
                                     std::string& threeStr,
                                     const char* szFuncName = nullptr);

// See if the file is there.
//
EXPORT bool Exists(std::string strFolder, std::string oneStr = "",
                   std::string twoStr = "", std::string threeStr = "");

EXPORT int64_t FormPathString(std::string& strOutput, std::string strFolder,
                              std::string oneStr = "", std::string twoStr = "",
                              std::string threeStr = "");
// Store/Retrieve a string.
//
EXPORT bool StoreString(std::string strContents, std::string strFolder,
                        std::string oneStr = "", std::string twoStr = "",
                        std::string threeStr = "");

EXPORT std::string QueryString(std::string strFolder, std::string oneStr = "",
                               std::string twoStr = "",
                               std::string threeStr = "");

EXPORT bool StorePlainString(std::string strContents, std::string strFolder,
                             std::string oneStr = "", std::string twoStr = "",
                             std::string threeStr = "");

EXPORT std::string QueryPlainString(std::string strFolder,
                                    std::string oneStr = "",
                                    std::string twoStr = "",
                                    std::string threeStr = "");

// Store/Retrieve an object. (Storable.)
//
EXPORT bool StoreObject(Storable& theContents, std::string strFolder,
                        std::string oneStr = "", std::string twoStr = "",
                        std::string threeStr = "");

// Use %newobject OTDB::Storage::Query();
EXPORT Storable* QueryObject(StoredObjectType theObjectType,
                             std::string strFolder, std::string oneStr = "",
                             std::string twoStr = "",
                             std::string threeStr = "");
// Store/Retrieve a Storable object inside an OTASCIIArmor object.

EXPORT std::string EncodeObject(Storable& theContents);

// Use %newobject OTDB::Storage::DecodeObject();
EXPORT Storable* DecodeObject(StoredObjectType theObjectType,
                              std::string strInput);

// Erase any value based on its location.

EXPORT bool EraseValueByKey(std::string strFolder, std::string oneStr = "",
                            std::string twoStr = "", std::string threeStr = "");

#ifdef SWIG
#define DECLARE_GET_ADD_REMOVE(name)                                           \
                                                                               \
protected:                                                                     \
    std::deque<stlplus::simple_ptr_clone<name>> list_##name##s;                \
                                                                               \
public:                                                                        \
    size_t Get##name##Count();                                                 \
    name* Get##name(size_t nIndex);                                            \
    bool Remove##name(size_t nIndex##name);                                    \
    bool Add##name(name& disownObject)

#endif

#ifndef SWIG
#define DECLARE_GET_ADD_REMOVE(name)                                           \
                                                                               \
protected:                                                                     \
    std::deque<stlplus::simple_ptr_clone<name>> list_##name##s;                \
                                                                               \
public:                                                                        \
    EXPORT size_t Get##name##Count();                                          \
    EXPORT name* Get##name(size_t nIndex);                                     \
    EXPORT bool Remove##name(size_t nIndex##name);                             \
    EXPORT bool Add##name(name& disownObject)
#endif // (not) SWIG

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
    {
        m_Type = "OTDBString";
    }
    OTDBString(const std::string& rhs)
        : Storable()
        , m_string(rhs)
    {
        m_Type = "OTDBString";
    }

public:
    virtual ~OTDBString()
    {
    }

    std::string m_string;

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
    {
        m_Type = "Blob";
    }

public:
    virtual ~Blob()
    {
    }

    ot_data_t m_memBuffer; // Where the actual binary data is stored,
                           // before packing.

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
    {
        m_Type = "StringMap";
    }

public:
    virtual ~StringMap()
    {
    }

    std::map<std::string, std::string> the_map;

    void SetValue(const std::string& strKey, const std::string& strValue)
    {
        std::map<std::string, std::string>::iterator ii = the_map.find(strKey);
        if (ii != the_map.end()) the_map.erase(ii);
        the_map[strKey] = strValue;
    }

    std::string GetValue(const std::string& strKey)
    {
        std::string ret_val("");
        std::map<std::string, std::string>::iterator ii = the_map.find(strKey);
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
    {
        m_Type = "Displayable";
    }

public:
    virtual ~Displayable()
    {
    }

    std::string gui_label; // The label that appears in the GUI

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
        , scale("0")
        , total_assets("0")
        , number_bids("0")
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
    virtual ~MarketData()
    {
    }

    using Displayable::gui_label; // The label that appears in the GUI

    std::string server_id;
    std::string market_id;

    std::string asset_type_id;
    std::string currency_type_id;

    std::string scale; // the Market scale. (A trade in any particular asset is
                       // measured in X units of SCALE.)
    // IOW, if the scale is 5000 on the gold market, that means "3 units" is
    // 15000 gold

    std::string total_assets; // total amount of assets available on market for
                              // purchase.

    std::string number_bids; // number of bids that are currently on the market.
    std::string number_asks; // number of asks that are currently on the market.

    std::string last_sale_price; // The price at which the most recent trade
                                 // occurred on this market.
    std::string current_bid;     // The highest bid currently on the market.
    std::string current_ask; // The lowest ask price currently available on the
                             // market.

    std::string volume_trades; // 24-hour period, number of trades.

    std::string volume_assets;   // 24-hour volume, amount of assets traded.
    std::string volume_currency; // 24-hour volume, amount of currency paid for
                                 // assets traded.

    std::string recent_highest_bid; // in a 24hour period, the highest bid to
                                    // hit the market.
    std::string recent_lowest_ask;  // in a 24hour period, the lowest ask to hit
                                    // the market.

    std::string last_sale_date; // (NEW FIELD) The date on which the most recent
                                // trade occurred on this market.

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
    {
        m_Type = "MarketList";
    }

public:
    virtual ~MarketList()
    {
    }

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
    virtual ~OfferDataMarket()
    {
    }

    using Displayable::gui_label; // The label that appears in the GUI

    std::string transaction_id;
    std::string price_per_scale;
    std::string available_assets;

    // Each sale or purchase against (total_assets - finished_so_far) must be in
    // minimum increments.
    // Minimum Increment must be evenly divisible by scale.
    // (This effectively becomes a "FILL OR KILL" order if set to the same value
    // as total_assets. Also, MUST be 1
    // or greater. CANNOT be zero. Enforce this at class level. You cannot sell
    // something in minimum increments of 0.)

    std::string minimum_increment;

    std::string date; // (NEW FIELD) The date this offer was added to the
                      // market.

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
    virtual ~BidData()
    {
    }

    using Displayable::gui_label; // The label that appears in the GUI

    using OfferDataMarket::transaction_id;
    using OfferDataMarket::price_per_scale;
    using OfferDataMarket::available_assets;
    using OfferDataMarket::minimum_increment;
    using OfferDataMarket::date;

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
    virtual ~AskData()
    {
    }

    using Displayable::gui_label; // The label that appears in the GUI

    using OfferDataMarket::transaction_id;
    using OfferDataMarket::price_per_scale;
    using OfferDataMarket::available_assets;
    using OfferDataMarket::minimum_increment;
    using OfferDataMarket::date;

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
    {
        m_Type = "OfferListMarket";
    }

public:
    virtual ~OfferListMarket()
    {
    }

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
    virtual ~TradeDataMarket()
    {
    }

    using Displayable::gui_label; // The label that appears in the GUI

    std::string transaction_id; // (transaction number for this trade.)
    std::string date;           // (The date of this trade's execution)
    std::string price;          // (The price this trade executed at.)
    std::string amount_sold;    // (Amount of asset sold for that price.)

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
    {
        m_Type = "TradeListMarket";
    }

public:
    virtual ~TradeListMarket()
    {
    }

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
        , selling(false)
        , scale("1")
        , price_per_scale("1")
        , transaction_id("0")
        , total_assets("1")
        , finished_so_far("0")
        , minimum_increment("1")
        , stop_price("0")
        , date("0")
    {
        m_Type = "OfferDataNym";
    }

public:
    virtual ~OfferDataNym()
    {
    }

    using Displayable::gui_label; // The label that appears in the GUI

    std::string valid_from;
    std::string valid_to;

    std::string server_id;
    std::string asset_type_id;    // the asset type on offer.
    std::string asset_acct_id;    // the account where the asset is.
    std::string currency_type_id; // the currency being used to purchase the
                                  // asset.
    std::string currency_acct_id; // the account where currency is.

    bool selling; // true for ask, false for bid.

    std::string scale; // 1oz market? 100oz market? 10,000oz market? This
                       // determines size and granularity.
    std::string price_per_scale;

    std::string transaction_id;

    std::string total_assets;
    std::string finished_so_far;

    // Each sale or purchase against (total_assets - finished_so_far) must be in
    // minimum increments.
    // Minimum Increment must be evenly divisible by scale.
    // (This effectively becomes a "FILL OR KILL" order if set to the same value
    // as total_assets. Also, MUST be 1
    // or greater. CANNOT be zero. Enforce this at class level. You cannot sell
    // something in minimum increments of 0.)

    std::string minimum_increment;

    std::string stop_sign;  // If this is a stop order, this will contain '<' or
                            // '>'.
    std::string stop_price; // The price at which the stop order activates (less
                            // than X or greater than X, based on sign.)

    std::string date; // (NEW FIELD) The date on which this offer was added to
                      // the market.

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
    {
        m_Type = "OfferListNym";
    }

public:
    virtual ~OfferListNym()
    {
    }

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
        , currency_paid("0")
    {
        m_Type = "TradeDataNym";
    }

public:
    virtual ~TradeDataNym()
    {
    }

    using Displayable::gui_label; // The label that appears in the GUI

    std::string transaction_id; // (transaction number for original offer.)

    std::string completed_count; // (How many trades have processed for the
                                 // associated offer? We keep count for each
                                 // trade.)
    std::string date;            // (The date of this trade's execution)
    std::string price;           // (The price this trade executed at.)
    std::string amount_sold;     // (Amount of asset sold for that price.)
    std::string updated_id;  // NEW FIELD (Transaction ID for trade receipt.)
    std::string offer_price; // NEW FIELD (price limit on the original offer.)
    std::string finished_so_far; // NEW FIELD (total amount sold across all
                                 // trades.)
    std::string asset_id;        // NEW FIELD asset ID for trade
    std::string currency_id;     // NEW FIELD currency ID for trade
    std::string currency_paid;   // NEW FIELD currency paid for trade

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
    {
        m_Type = "TradeListNym";
    }

public:
    virtual ~TradeListNym()
    {
    }

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
    {
        m_Type = "Acct";
    }

public:
    virtual ~Acct()
    {
    }

    using Displayable::gui_label; // The label that appears in the GUI

    std::string acct_id;
    std::string server_id;

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
    {
        m_Type = "BitcoinAcct";
    }

public:
    virtual ~BitcoinAcct()
    {
    }

    using Displayable::gui_label; // The label that appears in the GUI

    using Acct::acct_id;
    using Acct::server_id;

    std::string bitcoin_acct_name;

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
    {
        m_Type = "ServerInfo";
    }

public:
    virtual ~ServerInfo()
    {
    }

    using Displayable::gui_label; // The label that appears in the GUI

    std::string server_id;
    std::string server_type;

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
    {
        m_Type = "Server";
    }

public:
    virtual ~Server()
    {
    }

    using Displayable::gui_label; // The label that appears in the GUI

    using ServerInfo::server_id;   // in base class
    using ServerInfo::server_type; // in base class

    std::string server_host;
    std::string server_port;

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
    {
        m_Type = "BitcoinServer";
    }

public:
    virtual ~BitcoinServer()
    {
    }

    using Displayable::gui_label; // The label that appears in the GUI

    using ServerInfo::server_id;   // in base class
    using ServerInfo::server_type; // in base class

    using Server::server_host; // in base class
    using Server::server_port; // in base class

    std::string bitcoin_username;
    std::string bitcoin_password;

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
    {
        m_Type = "RippleServer";
    }

public:
    virtual ~RippleServer()
    {
    }

    using Displayable::gui_label; // The label that appears in the GUI

    using ServerInfo::server_id;   // in base class
    using ServerInfo::server_type; // in base class

    using Server::server_host; // in base class
    using Server::server_port; // in base class

    std::string ripple_username;
    std::string ripple_password;

    std::string namefield_id;
    std::string passfield_id;

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
    {
        m_Type = "LoomServer";
    }

public:
    virtual ~LoomServer()
    {
    }

    using Displayable::gui_label; // The label that appears in the GUI

    using ServerInfo::server_id;   // in base class
    using ServerInfo::server_type; // in base class

    using Server::server_host; // in base class
    using Server::server_port; // in base class

    std::string loom_username;

    std::string namefield_id;

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
    {
        m_Type = "ContactNym";
    }

public:
    virtual ~ContactNym();

    using Displayable::gui_label; // The label that appears in the GUI

    std::string nym_type;
    std::string nym_id;
    std::string public_key;
    std::string memo;

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
    {
        m_Type = "WalletData";
    }

public:
    virtual ~WalletData()
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
    {
        m_Type = "ContactAcct";
    }

public:
    virtual ~ContactAcct()
    {
    }

    using Displayable::gui_label; // The label that appears in the GUI

    std::string server_type;
    std::string server_id;
    std::string asset_type_id;
    std::string acct_id;
    std::string nym_id;
    std::string memo;
    std::string public_key;

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
    {
        m_Type = "Contact";
    }

public:
    virtual ~Contact();

    using Displayable::gui_label; // The label that appears in the GUI

    std::string contact_id;
    std::string email;
    std::string memo;
    std::string public_key;

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
    {
        m_Type = "AddressBook";
    }

public:
    virtual ~AddressBook();

    DECLARE_GET_ADD_REMOVE(Contact);

    DEFINE_OT_DYNAMIC_CAST(AddressBook)
};
} // Namespace OTDB

#ifndef SWIG

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
private:
    std::string m_strDataPath;

protected:
    StorageFS(); // You have to use the factory to instantiate (so it can create
                 // the Packer also.)
    // But from there, however you Init, Store, Query, etc is entirely up to
    // you.

    // Confirms if a file exists.  If it exists at path; return length.
    int64_t ConstructAndConfirmPath(std::string& strOutput,
                                    std::string strFolder,
                                    std::string oneStr = "",
                                    std::string twoStr = "",
                                    std::string threeStr = "");

    // Verifies whether path exists AND creates folders where necessary.
    int64_t ConstructAndCreatePath(std::string& strOutput,
                                   std::string strFolder,
                                   std::string oneStr = "",
                                   std::string twoStr = "",
                                   std::string threeStr = "");

private:
    int64_t ConstructAndConfirmPathImp(bool bMakePath, std::string& strOutput,
                                       std::string zeroStr, std::string oneStr,
                                       std::string twoStr,
                                       std::string threeStr);

protected:
    // If you wish to make your own subclass of OTDB::Storage, then use
    // StorageFS as an example.
    // The below 6 methods are the only overrides you need to copy.
    //
    virtual bool onStorePackedBuffer(PackedBuffer& theBuffer,
                                     std::string strFolder,
                                     std::string oneStr = "",
                                     std::string twoStr = "",
                                     std::string threeStr = "");

    virtual bool onQueryPackedBuffer(PackedBuffer& theBuffer,
                                     std::string strFolder,
                                     std::string oneStr = "",
                                     std::string twoStr = "",
                                     std::string threeStr = "");

    virtual bool onStorePlainString(std::string& theBuffer,
                                    std::string strFolder,
                                    std::string oneStr = "",
                                    std::string twoStr = "",
                                    std::string threeStr = "");

    virtual bool onQueryPlainString(std::string& theBuffer,
                                    std::string strFolder,
                                    std::string oneStr = "",
                                    std::string twoStr = "",
                                    std::string threeStr = "");

    virtual bool onEraseValueByKey(std::string strFolder,
                                   std::string oneStr = "",
                                   std::string twoStr = "",
                                   std::string threeStr = "");

public:
    virtual bool Exists(std::string strFolder, std::string oneStr = "",
                        std::string twoStr = "", std::string threeStr = "");

    virtual int64_t FormPathString(std::string& strOutput,
                                   std::string strFolder,
                                   std::string oneStr = "",
                                   std::string twoStr = "",
                                   std::string threeStr = "");

    static StorageFS* Instantiate()
    {
        return new StorageFS;
    }

    virtual ~StorageFS();

    // lower level calls.

    bool ConfirmOrCreateFolder(const char* szFolderName,
                               struct stat* pst = nullptr); // local to
                                                            // data_folder
    bool ConfirmFile(const char* szFileName,
                     struct stat* pst = nullptr); // local to data_folder
};

} // namespace OTDB

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

#endif // (not) SWIG

} // namespace opentxs

#endif // OPENTXS_CORE_OTSTORAGE_HPP
