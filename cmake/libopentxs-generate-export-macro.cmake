# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

macro(libopentxs_generate_export_macro)
  if(OPENTXS_STANDALONE)
    if(BUILD_SHARED_LIBS)
      set(OT_EXPORT_MACRO "opentxs_EXPORTS")
      set(OT_IMPORT_MACRO "")
    else()
      set(OT_EXPORT_MACRO "OPENTXS_STATIC_DEFINE")
      set(OT_IMPORT_MACRO "OPENTXS_STATIC_DEFINE")
    endif()
  else()
    set(OT_EXPORT_MACRO "OPENTXS_STATIC_DEFINE")
    set(OT_IMPORT_MACRO "OPENTXS_STATIC_DEFINE")
  endif()
endmacro()
