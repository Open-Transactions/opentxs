# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

if(OT_BLOCKCHAIN_EXPORT)
  target_sources(opentxs-common PRIVATE "PaymentCode.hpp")
  libopentxs_link_internal(Boost::headers)

  if(OT_ENABLE_MATTERFI)
    target_sources(opentxs-common PRIVATE "PaymentCode.cpp")
  else()
    target_sources(opentxs-common PRIVATE "PaymentCode-disabled.cpp")
  endif()
endif()
