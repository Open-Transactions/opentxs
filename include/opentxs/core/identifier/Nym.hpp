// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CORE_IDENTIFIER_NYM_HPP
#define OPENTXS_CORE_IDENTIFIER_NYM_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/core/Identifier.hpp"

#ifdef SWIG
// clang-format off
%ignore opentxs::identifier::Nym::Factory;
%extend opentxs::identifier::Nym {
    static OTNymID Factory()
    {
        return opentxs::identifier::Nym::Factory();
    }
    static OTNymID Factory(
        const std::string& rhs)
    {
        return opentxs::identifier::Nym::Factory(rhs);
    }
}
%rename (NymID) opentxs::identifier::Nym;
%template(OTNymID) opentxs::Pimpl<opentxs::identifier::Nym>;
// clang-format on
#endif

namespace opentxs
{

#ifndef SWIG
bool operator==(
    const opentxs::Pimpl<opentxs::identifier::Nym>& lhs,
    const opentxs::identifier::Nym& rhs);
bool operator!=(
    const opentxs::Pimpl<opentxs::identifier::Nym>& lhs,
    const opentxs::identifier::Nym& rhs);
bool operator<(
    const opentxs::Pimpl<opentxs::identifier::Nym>& lhs,
    const opentxs::Pimpl<opentxs::identifier::Nym>& rhs);
#endif

namespace identifier
{
class Nym : virtual public opentxs::Identifier
{
public:
#ifndef SWIG
    EXPORT static OTNymID Factory();
    EXPORT static OTNymID Factory(const std::string& rhs);
    EXPORT static OTNymID Factory(const String& rhs);
    EXPORT static OTNymID Factory(const opentxs::Nym& nym);
#endif

    EXPORT virtual ~Nym() = default;

protected:
    Nym() = default;

private:
    friend OTNymID;

    virtual Nym* clone() const = 0;

    Nym(const Nym&) = delete;
    Nym(Nym&&) = delete;
    Nym& operator=(const Nym&) = delete;
    Nym& operator=(Nym&&) = delete;
};
}  // namespace identifier
}  // namespace opentxs
#endif
