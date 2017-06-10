/************************************************************
 *
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

#include "opentxs/core/stdafx.hpp"

#include "opentxs/core/crypto/OTPassword.hpp"

#include "opentxs/api/OT.hpp"
#include "opentxs/core/crypto/CryptoEngine.hpp"
#include "opentxs/core/crypto/CryptoUtil.hpp"
#include "opentxs/core/crypto/OTPasswordData.hpp"
#include "opentxs/core/util/Assert.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/String.hpp"

// For SecureZeroMemory
#ifdef _WIN32
#else // not _WIN32

#include <sys/mman.h>

#ifndef PAGESIZE
#include <unistd.h>

#include <stdint.h>
#include <cstring>
#include <ostream>
#include <string>

#define PAGESIZE sysconf(_SC_PAGESIZE)
#endif

// FT: Credit to the Bitcoin team for the mlock / munlock defines.

#define mlock(a, b)                                                            \
    mlock((reinterpret_cast<void*>((reinterpret_cast<size_t>(a)) &             \
                                   (~((PAGESIZE)-1)))),                        \
          ((((reinterpret_cast<size_t>(a)) + (b)-1) | ((PAGESIZE)-1)) + 1) -   \
              ((reinterpret_cast<size_t>(a)) & (~((PAGESIZE)-1))))
#define munlock(a, b)                                                          \
    munlock((reinterpret_cast<void*>((reinterpret_cast<size_t>(a)) &           \
                                     (~((PAGESIZE)-1)))),                      \
            ((((reinterpret_cast<size_t>(a)) + (b)-1) | ((PAGESIZE)-1)) + 1) - \
                ((reinterpret_cast<size_t>(a)) & (~((PAGESIZE)-1))))
#endif

namespace opentxs
{

// For everything but Windows:
//
#ifndef _WIN32
extern "C" void* ot_secure_memset(void* v, uint8_t c, uint32_t n);

// This function securely overwrites the contents of a memory buffer
// (which can otherwise be optimized out by an overzealous compiler...)
//
void* ot_secure_memset(void* v, uint8_t c, uint32_t n)
{
    OT_ASSERT((nullptr != v) && (n > 0));

    volatile uint8_t* p = static_cast<volatile uint8_t*>(v);
    while (n--) *p++ = c;

    return v;
}
#endif // _WIN32

// TODO, security: Generate a session key, and encrypt the password string to
// that key whenever setting it,
// and decrypt it using that key whenever getting it. Also make sure to use the
// lock / unlock functions
// at that time (below). Also change so that the contents are dynamically
// allocated.
// NOTE: Given that OTSymmetricKey works with OTPassword, this is a bit circular
// in logic. Therefore might
// need to add a function to OTEnvelope so that it takes a const char * instead
// of an OTPassword, in order
// to handle this specific case! Might also need to duplicate some code between
// OTSymmetricKey and OTPassword
// in order to make sure both have the same protections. I'll see if there's a
// way to do this without duplication,
// as I get deeper into it.

// THE PURPOSE OF LOCKING A PAGE:
//
// "So that it won't get swapped to disk, where the secret
// could be recovered maliciously from the swap file."
//
bool OTPassword::ot_lockPage(void* addr, size_t len)
{
#ifdef _WIN32
// return VirtualLock(addr, len);
#elif defined(PREDEF_PLATFORM_UNIX)
    static bool bWarned = false;
    if (mlock(addr, len) && !bWarned) {
        bWarned = true;
        otErr << "ot_lockPage: WARNING: unable to lock memory. \n"
                 "   (Passwords / secret keys may be swapped to disk!)\n";
    }
    return true;
#else
    OT_FAIL_MSG("ASSERT: ot_lockPage unable to lock memory.");
#endif
    return false;
}

// TODO:  Note: may need to add directives here so that mlock and munlock are
// not
// used except where the user is running as a privileged process. (Because that
// may be the only way we CAN use those functions...)

bool OTPassword::ot_unlockPage(void* addr, size_t len)
{
#ifdef _WIN32
//    return VirtualUnlock(addr, len);
#elif defined(PREDEF_PLATFORM_UNIX)
    static bool bWarned = false;
    if (munlock(addr, len) && !bWarned) {
        bWarned = true;
        otErr << "ot_unlockPage: WARNING: unable to unlock memory used for "
                 "storing secrets.\n";
    }
    return true;
#else
    OT_FAIL_MSG("ASSERT: ot_unlockPage unable to unlock secret memory.");
#endif
    return false;
}

// PURPOSE OF ZERO'ING MEMORY:
//
// So the secret is not stored in memory any longer than absolutely necessary.
// Once it has been used, we want to wipe it from memory ASAP. (The least amount
// of time spent in memory, the better.)

void OTPassword::zeroMemory()
{
    size_ = 0;

    OTPassword::zeroMemory(static_cast<void*>(&(data_[0])), getBlockSize());

//
#ifndef _WIN32
    // UNLOCK the page, now that we're AFTER the point where
    // the memory was safely ZERO'd out.
    //
    if (isPageLocked_) {
        if (ot_unlockPage(static_cast<void*>(&(data_[0])), getBlockSize())) {
            isPageLocked_ = false;
        }
        else
            otErr << "OTPassword::zeroMemory: Error: Memory page was locked, "
                     "but then failed to unlock it.\n";
    }
#endif
}

// static
void OTPassword::zeroMemory(void* vMemory, uint32_t theSize)
{
    //  OT_ASSERT_MSG((nullptr != vMemory) && (theSize >
    // 0),"OTPassword::zeroMemory: ASSERT: vMemory is nullptr or theSize is
    // 0.");

    if ((nullptr != vMemory) && (theSize > 0)) {
        uint8_t* szMemory = static_cast<uint8_t*>(vMemory);
        OTPassword::zeroMemory(szMemory, theSize);
    }
}

// static
void OTPassword::zeroMemory(uint8_t* szMemory, uint32_t theSize)
{
#ifdef _WIN32

    //
    SecureZeroMemory(szMemory, theSize);

// NOTE: Both SecureZeroMemory, and the pragma solution commented-out below,
// are acceptable for Windows. I put both here for my notes.
//
//#pragma optimize("", off)
//    memset(szMemory, 0, theSize);
//#pragma optimize("", on)

// Dr. UNIX, I presume? So, we meet again...
#else
    ot_secure_memset(szMemory, static_cast<uint8_t>(0), theSize);
#endif
}

/* WINDOWS:
 errno_t memcpy_s(void* dest,
     size_t   numberOfElements,
     const
     void   * src,
     size_t   count
     );

 FT: Apparently numberOfElements is similar to strcpy_s (where it's the maximum
 size of destination buffer.)
 "numberOfElements is the Maximum number of characters destination string can
 accomodate including the nullptr character"
 (Then count is the actual size being copied.)
 */
// UNIX:
//    void * memcpy(void* restrict s1, const void* restrict s2, size_t n);
//
// static
void* OTPassword::safe_memcpy(void* dest, uint32_t dest_size, const void* src,
                              uint32_t src_length,
                              bool bZeroSource) // if true, sets the
                                                // source buffer to
                                                // zero after copying
                                                // is done.
{
    // Make sure they aren't null.
    OT_ASSERT(nullptr != dest);
    OT_ASSERT(nullptr != src);

    // Make sure they aren't the same pointer.
    OT_ASSERT(src != dest);

    // Make sure it will fit.
    OT_ASSERT_MSG(src_length <= dest_size,
                  "ASSERT: safe_memcpy: destination buffer too small.\n");

    // Make sure they don't overlap.
    // First assert does the beginning of the string, makes sure it's not within
    // the bounds of the destination
    // string. Second assert does the same thing for the end of the string.
    // Finally a third is needed to make sure
    // we're not in a situation where the beginning is less than the dest
    // beginning, yet the end is also more than
    // the dest ending!
    //
    OT_ASSERT_MSG(
        false ==
            ((static_cast<const uint8_t*>(src) > static_cast<uint8_t*>(dest)) &&
             (static_cast<const uint8_t*>(src) <
              (static_cast<uint8_t*>(dest) + dest_size))),
        "ASSERT: safe_memcpy: Unexpected memory overlap, start of src.\n");
    OT_ASSERT_MSG(
        false == (((static_cast<const uint8_t*>(src) + src_length) >
                   static_cast<uint8_t*>(dest)) &&
                  ((static_cast<const uint8_t*>(src) + src_length) <
                   (static_cast<uint8_t*>(dest) + dest_size))),
        "ASSERT: safe_memcpy: Unexpected memory overlap, end of src.\n");
    OT_ASSERT(false == ((static_cast<const uint8_t*>(src) <=
                         static_cast<uint8_t*>(dest)) &&
                        ((static_cast<const uint8_t*>(src) + src_length) >=
                         (static_cast<uint8_t*>(dest) + dest_size))));

#ifdef _WIN32
    bool bSuccess = (0 == memcpy_s(dest, static_cast<size_t>(dest_size), src,
                                   static_cast<size_t>(src_length)));
#else
    bool bSuccess =
        (memcpy(dest, src, static_cast<size_t>(src_length)) == dest);
#endif

    if (bSuccess) {
        if (bZeroSource) {
            OTPassword::zeroMemory(const_cast<void*>(src), src_length);
        }

        return dest;
    }

    return nullptr;
}

// OTPassword thePass; will create a text password.
// But use the below function if you want one that has
// a text buffer of size (versus a 0 size.) This is for
// cases where you need the buffer to pre-exist so that
// some other function can populate that buffer directly.
// (Such as the OpenSSL password callback...)
// CALLER IS RESPONSIBLE TO DELETE.
//
// static
OTPassword* OTPassword::CreateTextBuffer() // asserts already.
{
    // Caller MUST delete!

    char throwaway_text[OT_DEFAULT_BLOCKSIZE];
    for (int32_t tt = 0; tt < OT_DEFAULT_BLOCKSIZE; ++tt) {
        throwaway_text[tt] = 'A'; // todo optimization...
    }
    throwaway_text[OT_DEFAULT_BLOCKSIZE - 1] = '\0';
    // We don't use the above memory, except to force OTPassword to create
    // itself
    // at a certain password size, so we can pass that buffer and size on to any
    // C-style function that needs them to "already exist."
    //
    OTPassword* pPassUserInput = new OTPassword(
        &(throwaway_text[0]), OT_DEFAULT_BLOCKSIZE - 1); // text mode.
    OT_ASSERT_MSG(nullptr != pPassUserInput,
                  "OTPassword::CreateTextBuffer: ASSERT: OTPassword * "
                  "pPassUserInput = new OTPassword(&(throwaway_text[0]), "
                  "OT_DEFAULT_BLOCKSIZE-1);");
    // Below this point, pPassUserInput must be returned, or deleted. (Or it
    // will leak.)

    return pPassUserInput;
}

OTPassword::OTPassword()
    : size_(0)
    , isText_(true)
    , isBinary_(false)
    , isPageLocked_(false)
{
    data_[0] = '\0';
    setPassword_uint8(reinterpret_cast<const uint8_t*>(""), 0);
}

OTPassword& OTPassword::operator=(const OTPassword& rhs)
{
    if (rhs.isPassword()) {
        setPassword_uint8(rhs.getPassword_uint8(), rhs.getPasswordSize());
    }
    else if (rhs.isMemory()) {
        setMemory(rhs.getMemory_uint8(), rhs.getMemorySize());
    }

    return *this;
}

OTPassword::OTPassword(const OTPassword& rhs)
    : size_(0)
    , isText_(rhs.isPassword())
    , isBinary_(rhs.isMemory())
    , isPageLocked_(false)
    , blockSize_(
          rhs.blockSize_) // The buffer has this size+1 as its static size.
{
    if (isText_) {
        data_[0] = '\0';
        setPassword_uint8(rhs.getPassword_uint8(), rhs.getPasswordSize());
    }
    else if (isBinary_) {
        setMemory(rhs.getMemory_uint8(), rhs.getMemorySize());
    }
}

OTPassword::OTPassword(const char* szInput, uint32_t nInputSize)
    : size_(0)
    , isText_(true)
    , isBinary_(false)
    , isPageLocked_(false)
{
    data_[0] = '\0';

    setPassword_uint8(reinterpret_cast<const uint8_t*>(szInput), nInputSize);
}

OTPassword::OTPassword(const uint8_t* szInput, uint32_t nInputSize)
    : size_(0)
    , isText_(true)
    , isBinary_(false)
    , isPageLocked_(false)
{
    data_[0] = '\0';

    setPassword_uint8(szInput, nInputSize);
}

OTPassword::OTPassword(const void* vInput, uint32_t nInputSize)
    : size_(0)
    , isText_(false)
    , isBinary_(true)
    , isPageLocked_(false)
{
    setMemory(vInput, nInputSize);
}

OTPassword::~OTPassword()
{
    if (size_ > 0) zeroMemory();
}

bool OTPassword::isPassword() const
{
    return isText_;
}

bool OTPassword::isMemory() const
{
    return isBinary_;
}

const char* OTPassword::getPassword() const // asserts if isText_ is false.
{
    return reinterpret_cast<const char*>(getPassword_uint8());
}

// getPassword returns "" if empty, otherwise returns the password.
//
const uint8_t* OTPassword::getPassword_uint8() const
{
    OT_ASSERT(isText_);
    return (size_ <= 0) ? reinterpret_cast<const uint8_t*>("") : &(data_[0]);
}

uint8_t* OTPassword::getPasswordWritable()
{
    OT_ASSERT(isText_);
    return (size_ <= 0)
               ? nullptr
               : static_cast<uint8_t*>(static_cast<void*>(&(data_[0])));
}

char* OTPassword::getPasswordWritable_char()
{
    OT_ASSERT(isText_);
    return (size_ <= 0) ? nullptr
                        : static_cast<char*>(static_cast<void*>(&(data_[0])));
}

// getMemory returns nullptr if empty, otherwise returns the password.
//
const void* OTPassword::getMemory() const
{
    return reinterpret_cast<const void*>(getMemory_uint8());
}

const uint8_t* OTPassword::getMemory_uint8() const
{
    OT_ASSERT(isBinary_);
    return (size_ <= 0) ? nullptr : &(data_[0]);
}

// getMemoryWritable returns nullptr if empty, otherwise returns the password.
void* OTPassword::getMemoryWritable()
{
    OT_ASSERT(isBinary_);
    return (size_ <= 0) ? nullptr : static_cast<void*>(&(data_[0]));
}

std::size_t OTPassword::getBlockSize() const
{
    return blockSize_;
}

uint32_t OTPassword::getPasswordSize() const
{
    OT_ASSERT(isText_);
    return size_;
}

uint32_t OTPassword::getMemorySize() const
{
    OT_ASSERT(isBinary_);
    return size_;
}

bool OTPassword::addChar(uint8_t theChar)
{
    OT_ASSERT(isPassword());
    if (getPasswordSize() < getBlockSize()) {
        data_[size_] = theChar;
        ++size_;
        data_[size_] = '\0';
        return true;
    }
    return false;
}

bool OTPassword::Compare(OTPassword& rhs) const
{
    OT_ASSERT(isPassword() || isMemory());
    OT_ASSERT(rhs.isPassword() || rhs.isMemory());

    if (isPassword() && !rhs.isPassword()) return false;
    if (isMemory() && !rhs.isMemory()) return false;

    const uint32_t nThisSize =
        isPassword() ? getPasswordSize() : getMemorySize();
    const uint32_t nRhsSize =
        rhs.isPassword() ? rhs.getPasswordSize() : rhs.getMemorySize();

    if (nThisSize != nRhsSize) return false;

    if (0 ==
        memcmp(isPassword() ? getPassword_uint8() : getMemory_uint8(),
               rhs.isPassword() ? rhs.getPassword_uint8()
                                : rhs.getMemory_uint8(),
               rhs.isPassword() ? rhs.getPasswordSize() : rhs.getMemorySize()))
        return true;

    return false;
}

int32_t OTPassword::setPassword(const std::string& input)
{
    return setPassword(input.data(), input.size());
}

// Returns size of password (in case truncation is necessary.)
// Returns -1 in case of error.
//
int32_t OTPassword::setPassword(const char* szInput, int32_t nInputSize)
{
    return setPassword_uint8(reinterpret_cast<const uint8_t*>(szInput),
                             static_cast<uint32_t>(nInputSize));
}

// This adds a null terminator.
//
int32_t OTPassword::setPassword_uint8(const uint8_t* szInput,
                                      uint32_t nInputSize)
{
    OT_ASSERT(nullptr != szInput);

    // cppcheck-suppress variableScope
    const char* szFunc = "OTPassword::setPassword";

    // Wipe whatever was in there before.
    //
    if (size_ > 0) zeroMemory();

    isBinary_ = false;
    isText_ = true;

    if (0 == nInputSize) return 0;

    // Make sure no input size is larger than our block size
    const size_t maxSize = getBlockSize();

    if (nInputSize > maxSize) {
        nInputSize = maxSize; // Truncated password beyond max size.
    }

    // The szInput string passed into this function should never
    // be a different size than what is passed in. For example it shouldn't
    // be SMALLER than what the user claims either. If it is, we error out.
    //
    if (String::safe_strlen(reinterpret_cast<const char*>(szInput), static_cast<size_t>(nInputSize))
        < static_cast<size_t>(nInputSize))
    {
        otErr
            << szFunc
            << ": ERROR: string length of szInput did not match nInputSize.\n";
        return (-1);
    }

#ifndef _WIN32

    //
    // Lock the memory page, before we copy the data over.
    // (If it's not already locked, which I doubt it will be.)
    //
    // it won't be locked already, since we just zero'd it
    // (above.) But I check this anyway...
    if (!isPageLocked_) {
        if (ot_lockPage(static_cast<void*>(&(data_[0])), getBlockSize())) {
            isPageLocked_ = true;
        }
        else {
            otErr << szFunc
                  << ": Error: Failed attempting to lock memory page.\n";
        }
    }
#endif

#ifdef _WIN32
    strncpy_s(reinterpret_cast<char*>(data_), (1 + nInputSize),
              reinterpret_cast<const char*>(szInput), nInputSize);
#else
    strncpy(reinterpret_cast<char*>(data_),
            reinterpret_cast<const char*>(szInput), nInputSize);
#endif

    // force a null terminator in the 129th byte (at index 128.)
    // (Or at the 6th byte (at index 5), if the size is 5 bytes int64_t.)
    //
    data_[nInputSize] = '\0';
    size_ = nInputSize;

    return size_;
}

// OTPassword::SetSize   (Low-level)
//
// There are certain weird cases, like in OTSymmetricKey::GetPassphraseFromUser,
// where we set the password using the getPassword_writable, and it's properly
// null-terminated, yet this instance still doesn't know its actual size (even
// though
// the size is known.) Therefore I added this call in order to set the size in
// those odd cases where it's necessary. That being said, YOU should normally
// NEVER
// need to use this function, so just pretend it doesn't exist.
//
// This adds a null terminator, IF we're in text mode (not binary mode.)
//
bool OTPassword::SetSize(uint32_t uSize)
{
    if (isBinary_) {
        if (uSize > getBlockSize())
            uSize = getBlockSize(); // Truncated password beyond max size.
        size_ = uSize;
        return true;
    }
    else if (isText_) {
        // Cannot be as much as the blocksize,
        // because no room for null-terminator.
        if (uSize >= getBlockSize()) {
            uSize = getBlockSize() - 1; // Truncated password to blocksize-1.
        }
        // The actual null-terminator.
        data_[uSize] = '\0';
        // If size is 3, the terminator is at
        size_ = uSize;
        // data_[3] (which is the 4th byte.)
        return true;
    }
    otErr << __FUNCTION__ << ": Error: isBinary_ and isText_ are both "
                             "false. (Should never happen.)\n";
    return false;
}

// static
bool OTPassword::randomizePassword(char* szDestination, uint32_t nNewSize)
{
    return OTPassword::randomizePassword_uint8(
        reinterpret_cast<uint8_t*>(szDestination), nNewSize);
}

// static
bool OTPassword::randomizePassword_uint8(uint8_t* szDestination,
                                         uint32_t nNewSize)
{
    OT_ASSERT(nullptr != szDestination);
    OT_ASSERT(nNewSize > 0);

    //  const char * szFunc = "OTPassword::randomizePassword(static)";

    if (OTPassword::randomizeMemory_uint8(szDestination, nNewSize)) {

        // This loop converts an array of binary bytes into the
        // same array, where each byte is translated to a byte
        // between the values of 33 and 122 (visible ASCII.)
        //
        for (uint32_t i = 0; i < nNewSize; ++i) {
            uint8_t temp = (((szDestination[i]) % 89) + 33);
            szDestination[i] = temp;
        }

        // Add the nullptr terminator...
        //
        szDestination[nNewSize - 1] = '\0';

        return true;
    }
    return false;
}

// Returns size of memory (in case truncation is necessary.)
// Returns -1 in case of error.
//
int32_t OTPassword::randomizePassword(uint32_t nNewSize)
{
    uint32_t nSize = nNewSize;

    // Wipe whatever was in there before.
    //
    if (size_ > 0) zeroMemory();

    isBinary_ = false;
    isText_ = true;

    if (0 == nSize) return 0;

    // Make sure no input size is larger than our block size
    //
    if (nSize > getBlockSize())
        nSize = getBlockSize(); // Truncated password beyond max size.
#ifndef _WIN32
    //
    // Lock the memory page, before we randomize 'size bytes' of the data.
    // (If it's not already locked, which I doubt it will be.)
    //
    if (!isPageLocked_) // it won't be locked already, since we just zero'd it
                        // (above.) But I check this anyway...
    {
        if (ot_lockPage(static_cast<void*>(&(data_[0])), getBlockSize())) {
            isPageLocked_ = true;
        }
        else {
            otErr << __FUNCTION__
                  << ": Error: Failed attempting to lock memory page.\n";
        }
    }
#endif

    //
    if (!OTPassword::randomizePassword_uint8(&(data_[0]),
                                             static_cast<int32_t>(nSize + 1))) {
        // randomizeMemory (above) already logs, so I'm not logging again twice
        // here.
        //
        zeroMemory();
        return -1;
    }

    size_ = nSize;

    return size_;
}

// static
bool OTPassword::randomizeMemory(void* szDestination, uint32_t nNewSize)
{
    return OTPassword::randomizeMemory_uint8(
        reinterpret_cast<uint8_t*>(szDestination), nNewSize);
}

// static
bool OTPassword::randomizeMemory_uint8(uint8_t* szDestination,
                                       uint32_t nNewSize)
{
    return OT::App().Crypto().Util().RandomizeMemory(szDestination, nNewSize);
}

// Returns size of memory (in case truncation is necessary.)
// Returns -1 in case of error.
//
int32_t OTPassword::randomizeMemory(uint32_t nNewSize)
{
    uint32_t nSize = nNewSize;

    // Wipe whatever was in there before.
    //
    if (size_ > 0) zeroMemory();

    isBinary_ = true;
    isText_ = false;

    if (0 == nSize) return 0;

    // Make sure no input size is larger than our block size
    //
    if (nSize > getBlockSize())
        nSize = getBlockSize(); // Truncated password beyond max size.

#ifndef _WIN32
    //
    // Lock the memory page, before we randomize 'size bytes' of the data.
    // (If it's not already locked, which I doubt it will be.)
    //
    if (!isPageLocked_) // it won't be locked already, since we just zero'd it
                        // (above.) But I check this anyway...
    {
        if (ot_lockPage(static_cast<void*>(&(data_[0])), getBlockSize())) {
            isPageLocked_ = true;
        }
        else {
            otErr << __FUNCTION__
                  << ": Error: Failed attempting to lock memory page.\n";
        }
    }
#endif

    //
    if (!OTPassword::randomizeMemory_uint8(&(data_[0]), nSize)) {
        // randomizeMemory (above) already logs, so I'm not logging again twice
        // here.
        //
        zeroMemory();
        return -1;
    }

    size_ = nSize;

    return size_;
}

// (FYI, truncates if nAppendSize + getPasswordSize() is larger than
// getBlockSize.)
// Returns number of bytes appended, or -1 for error.
//
int32_t OTPassword::addMemory(const void* vAppend, uint32_t nAppendSize)
{
    OT_ASSERT(nullptr != vAppend);

    //  const char * szFunc = "OTPassword::addMemory";

    if (0 == nAppendSize) return 0;

    // If I'm currently at a 0 size, then call setMemory instead.
    //
    if (size_ == 0) return setMemory(vAppend, nAppendSize);
    //
    // By this point, I know I already have some memory allocated,
    // and I'm actually appending some other memory onto the end of it.
    //
    // Should already be set from the above setMemory call.
    OT_ASSERT(isBinary_);

    // Make sure total new size isn't larger than our block size
    //
    if ((nAppendSize + size_) > getBlockSize()) {
        // Truncated password beyond max size.
        nAppendSize = (getBlockSize() - size_);
    }
    //  OT_ASSERT(nAppendSize >= 0);

    if (0 == nAppendSize) return 0;

    // By this point, I know nAppendSize is larger than 0, AND that appending it
    // onto the
    // existing memory of this object will not exceed the total allowed block
    // size.
    //
    // Because we use setMemory when empty, and only use addMemory when we KNOW
    // something
    // is already there, therefore we know the page is already locked, so no
    // need to go
    // trying to lock it again.

    OTPassword::safe_memcpy(static_cast<void*>(&(data_[size_])),
                            nAppendSize, // dest size is based on the source
                                         // size, but guaranteed to be >0 and
                                         // <=getBlockSize
                            vAppend,
                            nAppendSize); // Since dest size is
                                          // known to be src size or
                                          // less (and >0) we use it
                                          // as src size. (We may
                                          // have truncated... and
                                          // we certainly don't want
                                          // to copy beyond our own
                                          // truncation.)

    size_ += nAppendSize;

    return nAppendSize;
}

int32_t OTPassword::setMemory(const Data& data)
{
    const uint32_t dataSize = data.GetSize();
    uint32_t returnedSize = dataSize;

    bool memorySet = setMemory(data.GetPointer(), returnedSize);
    // TODO maybe we should check for truncation?
    return memorySet;
}

// Returns size of memory (in case truncation is necessary.)
// Returns -1 in case of error.
//
int32_t OTPassword::setMemory(const void* vInput, uint32_t nInputSize)
{
    OT_ASSERT(nullptr != vInput);

    // Wipe whatever was in there before.
    //
    if (size_ > 0) zeroMemory();

    isBinary_ = true;
    isText_ = false;

    if (0 == nInputSize) return 0;

    // Make sure no input size is larger than our block size
    //
    if (nInputSize > getBlockSize())
        nInputSize = getBlockSize(); // Truncated password beyond max size.

#ifndef _WIN32
    //
    // Lock the memory page, before we copy the data over.
    // (If it's not already locked, which I doubt it will be.)
    //
    if (!isPageLocked_) // it won't be locked already, since we just zero'd it
                        // (above.) But I check this anyway...
    {
        if (ot_lockPage(static_cast<void*>(&(data_[0])), getBlockSize())) {
            isPageLocked_ = true;
        }
        else {
            otErr << __FUNCTION__
                  << ": Error: Failed attempting to lock memory page.\n";
        }
    }
#endif

    OTPassword::safe_memcpy(static_cast<void*>(&(data_[0])),
                            // dest size is based on the source
                            // size, but guaranteed to be >0 and
                            // <=getBlockSize
                            nInputSize,
                            // Since dest size is known
                            // to be src size or less
                            // (and >0) we use it as src
                            // size. (We may have
                            // truncated... and we
                            // certainly don't want to
                            // copy beyond our own
                            // truncation.)
                            vInput, nInputSize);

    size_ = nInputSize;
    return size_;
}

// First use reset() to set the internal position to 0.
// Then you pass in the buffer where the results go.
// You pass in the length of that buffer.
// It returns how much was actually read.
// If you start at position 0, and read 100 bytes, then
// you are now on position 100, and the next OTfread will
// proceed from that position. (Unless you reset().)
uint32_t OTPassword::OTfread(uint8_t* data, uint32_t size)
{
    OT_ASSERT(data != nullptr && size > 0);

    uint32_t sizeToRead = 0;

    if (position_ < size_) {
        // If the size is 20, and position is 5 (I've already read the first 5
        // bytes) then the size remaining to read is 15. That is, GetSize()
        // minus position_.
        sizeToRead = size_ - position_;

        if (size < sizeToRead) {
            sizeToRead = size;
        }
        safe_memcpy(data, sizeToRead, getMemory_uint8(), sizeToRead);
        position_ += sizeToRead;
    }

    return sizeToRead;
}

} // namespace opentxs
