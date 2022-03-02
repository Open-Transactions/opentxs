//
// Created by adam.tkaczyk on 2/28/22.
//

#pragma once

#include <gmock/gmock.h>

#include "opentxs/api/crypto/Encode.hpp"

namespace opentxs
{

namespace api::crypto::internal
{
    class Encode : public api::crypto::Encode
    {
    public:
        Encode() = default;
    };
}

class EncodeMock : public api::crypto::internal::Encode
{
public:
    EncodeMock() = default;
    MOCK_METHOD(
        UnallocatedCString,
        DataEncode,
        (const UnallocatedCString&),
        (const, final));

    MOCK_METHOD(
        UnallocatedCString,
        DataEncode,
        (const Data&),
        (const, final));

    MOCK_METHOD(
        UnallocatedCString,
        DataDecode,
        (const UnallocatedCString&),
        (const, final));

    MOCK_METHOD(
        UnallocatedCString,
        IdentifierEncode,
        (const Data&),
        (const, final));

    MOCK_METHOD(
        UnallocatedCString,
        IdentifierDecode,
        (const UnallocatedCString&),
        (const, final));

    MOCK_METHOD(
        bool,
        IsBase62,
        (const UnallocatedCString&),
        (const, final));

    MOCK_METHOD(
        OTString,
        Nonce,
        (const std::uint32_t),
        (const, final));

    MOCK_METHOD(
        OTString,
        Nonce,
        (const std::uint32_t, Data&),
        (const, final));

    MOCK_METHOD(
        UnallocatedCString,
        RandomFilename,
        (),
        (const, final));

    MOCK_METHOD(
        UnallocatedCString,
        SanatizeBase58,
        (const UnallocatedCString&),
        (const, final));

    MOCK_METHOD(
        UnallocatedCString,
        SanatizeBase64,
        (const UnallocatedCString&),
        (const, final));

    MOCK_METHOD(
        UnallocatedCString,
        Z85Encode,
        (const Data&),
        (const, final));

    MOCK_METHOD(
        UnallocatedCString,
        Z85Encode,
        (const UnallocatedCString&),
        (const, final));

    MOCK_METHOD(
        OTData,
        Z85Decode,
        (const Data&),
        (const, final));

    MOCK_METHOD(
        UnallocatedCString,
        Z85Decode,
        (const UnallocatedCString&),
        (const, final));

    auto InternalEncode() const noexcept -> const api::crypto::internal::Encode& final
    {
        return *this;
    }

    auto InternalEncode() noexcept -> api::crypto::internal::Encode& final
    {
        return *this;
    }
};

}