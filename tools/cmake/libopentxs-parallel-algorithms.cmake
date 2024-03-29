# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

macro(libopentxs_parallel_algorithms)
  if(OT_USE_PSTL)
    target_sources(opentxs-common PRIVATE "pstl.cpp")

    if(OT_PSTL_NEEDS_TBB)
      libopentxs_link_external(TBB::tbb)
    endif()
  elseif(OT_WITH_TBB)
    target_sources(opentxs-common PRIVATE "tbb.cpp")
    libopentxs_link_internal(TBB::tbb)
    libopentxs_link_external(TBB::tbb)
  else()
    target_sources(opentxs-common PRIVATE "sequential.cpp")
  endif()
endmacro()
