// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#ifndef CHAISCRIPT_NO_THREADS
#define CHAISCRIPT_NO_THREADS
#endif

#ifndef CHAISCRIPT_NO_THREADS_WARNING
#define CHAISCRIPT_NO_THREADS_WARNING
#endif

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Warray-bounds"
#pragma GCC diagnostic ignored "-Wdefaulted-function-deleted"
#include <chaiscript/chaiscript.hpp>         // IWYU pragma: export
#include <chaiscript/chaiscript_stdlib.hpp>  // IWYU pragma: export

#pragma GCC diagnostic pop
