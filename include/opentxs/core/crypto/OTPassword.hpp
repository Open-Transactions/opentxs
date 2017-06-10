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

#ifndef OPENTXS_CORE_CRYPTO_OTPASSWORD_HPP
#define OPENTXS_CORE_CRYPTO_OTPASSWORD_HPP

#include <cstddef>
#include <string>

namespace opentxs
{

/*
 To use:

 OTPassword thePass;
 (Or...)
 OTPassword thePass(strPassword, strPassword.length());

 const char * szPassword    = thePass.getPassword();
 const int32_t    nPassLength    = thePass.getPasswordSize();

 If the instance of OTPassword is not going to be destroyed immediately
 after the password is used, then make sure to call zeroMemory() after
 using the password. (Otherwise the destructor will handle this anyway.)

 (The primary purpose of this class is that it zeros its memory out when
 it is destructed.)

 This class gives me a safe way to hand-off a password, and off-load the
 handling risk to the user.  This class will be included as part of the
 OT-API SWIG interface so that it's available inside other languages.

 */

#define OT_PW_DISPLAY "Enter master passphrase for wallet."
#define OT_DEFAULT_BLOCKSIZE 256
#define OT_DEFAULT_MEMSIZE 257

// Originally written for the safe storage of passwords.
// Now used for symmetric keys as well.
// Specifically: when the clear version of a password or key must be stored
// usually for temporary reasons, it must be stored in memory locked from
// swapping to disk, and in an object like OTPassword that zeros the memory as
// soon as we're done.
//
// OTPassword tries to store a piece of data more securely.
// During the time I have to take a password from the user and pass it to
// OpenSSL,
// I want it stored as securely as possible, and that's what this class was
// written for.
// Now I'm adding the ability to store binary data in here, not just a
// text-based password.
// That way, OTSymmetricKey can store its plain key in an OTPassword. Well,
// it actually stores
// its key in an encrypted format, but whenever, for what brief moments that
// key is decrypted and
// USED, the decrypted form of it will be stored in an OTPassword (in binary
// mode.)
// This is basically just to save me from duplicating work that's already
// done here in OTPassword.
//
class Data;

class OTPassword
{
public:
    EXPORT explicit OTPassword();
    EXPORT explicit OTPassword(const OTPassword& rhs);
    EXPORT explicit OTPassword(const char* input, uint32_t size);
    EXPORT explicit OTPassword(const uint8_t* input, uint32_t size);
    EXPORT explicit OTPassword(const void* input, uint32_t size);

    EXPORT ~OTPassword();
    EXPORT OTPassword& operator=(const OTPassword& rhs);

    EXPORT bool isPassword() const;
    EXPORT const uint8_t* getPassword_uint8() const;

    EXPORT const char* getPassword() const;
    EXPORT uint8_t* getPasswordWritable();
    EXPORT char* getPasswordWritable_char();
    // (FYI, truncates if nInputSize larger than getBlockSize.)
    EXPORT int32_t setPassword(const std::string& input);
    EXPORT int32_t setPassword(const char* input, int32_t size);
    // (FYI, truncates if nInputSize larger than getBlockSize.)
    EXPORT int32_t setPassword_uint8(const uint8_t* input, uint32_t size);
    EXPORT bool addChar(uint8_t c);
    EXPORT int32_t randomizePassword(uint32_t size = OT_DEFAULT_BLOCKSIZE);
    EXPORT static bool randomizePassword_uint8(uint8_t* destination,
                                               uint32_t size);
    EXPORT static bool randomizePassword(char* destination, uint32_t size);
    EXPORT bool isMemory() const;
    EXPORT const void* getMemory() const;
    EXPORT const uint8_t* getMemory_uint8() const;
    EXPORT void* getMemoryWritable();
    // (FYI, truncates if size larger than getBlockSize.)
    EXPORT int32_t setMemory(const Data& data);
    EXPORT int32_t setMemory(const void* input, uint32_t size);
    // (FYI, truncates if size + getPasswordSize() is larger than
    // getBlockSize.)
    EXPORT int32_t addMemory(const void* append, uint32_t size);
    EXPORT int32_t randomizeMemory(uint32_t size = OT_DEFAULT_BLOCKSIZE);
    EXPORT static bool randomizeMemory_uint8(uint8_t* destination,
                                             uint32_t size);
    EXPORT static bool randomizeMemory(void* destination, uint32_t size);
    EXPORT std::size_t getBlockSize() const;
    EXPORT bool Compare(OTPassword& rhs) const;
    EXPORT uint32_t getPasswordSize() const;
    EXPORT uint32_t getMemorySize() const;
    EXPORT void zeroMemory();
    EXPORT static void zeroMemory(uint8_t* szMemory, uint32_t size);
    EXPORT static void zeroMemory(void* vMemory, uint32_t size);
    EXPORT static void* safe_memcpy(void* dest, uint32_t dsize, const void* src,
                                    uint32_t ssize, bool zeroSource = false);
    inline void reset()
    {
        position_ = 0;
    }
    EXPORT uint32_t OTfread(uint8_t* data, uint32_t size);

    // OTPassword thePass; will create a text password.
    // But use the below function if you want one that has
    // a text buffer of size (versus a 0 size.) This is for
    // cases where you need the buffer to pre-exist so that
    // some other function can populate that buffer directly.
    // (Such as the OpenSSL password callback...)
    // CALLER IS RESPONSIBLE TO DELETE.
    // asserts already.
    EXPORT static OTPassword* CreateTextBuffer();

    // There are certain weird cases, like in
    // OTSymmetricKey::GetPassphraseFromUser,
    // where we set the password using the getPassword_writable, and it's
    // properly
    // null-terminated, yet this instance still doesn't know its actual size
    // (even though
    // the size is known.) Therefore I added this call in order to set the size
    // in
    // those odd cases where it's necessary. That being said, YOU should
    // normally NEVER
    // need to use this function, so just pretend it doesn't exist.
    EXPORT bool SetSize(uint32_t size);

private:
    std::size_t size_{0};
    std::uint8_t data_[OT_DEFAULT_MEMSIZE]{};
    bool isText_{false};
    bool isBinary_{false};
    bool isPageLocked_{false};
    const std::size_t blockSize_{OT_DEFAULT_BLOCKSIZE};
    std::uint32_t position_{};

    bool ot_lockPage(void* addr, size_t len);
    bool ot_unlockPage(void* addr, size_t len);
};

} // namespace opentxs

#endif // OPENTXS_CORE_CRYPTO_OTPASSWORD_HPP
