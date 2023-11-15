// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "core/String.hpp"  // IWYU pragma: associated

extern "C" {
#if __has_include(<wordexp.h>)
#include <wordexp.h>
#endif
}

#include <utility>

#include "opentxs/util/Log.hpp"

namespace opentxs::implementation
{
auto String::tokenize_basic(
    UnallocatedMap<UnallocatedCString, UnallocatedCString>& mapOutput) const
    -> bool
{
    // simple parser that allows for one level of quotes nesting but no escaped
    // quotes
    if (!Exists()) { return true; }

    const char* txt = Get();
    UnallocatedCString buf = txt;
    for (std::int32_t i = 0; txt[i] != 0;) {
        while (txt[i] == ' ') { i++; }
        std::int32_t k = i;
        std::int32_t k2{};
        if (txt[i] == '\'' || txt[i] == '"') {
            // quoted string
            char quote = txt[i++];
            k = i;
            while (txt[i] != quote && txt[i] != 0) { i++; }
            if (txt[i] != quote) {
                LogError()()("Unmatched quotes in: ")(txt)(".").Flush();
                return false;
            }
            k2 = i;
            i++;
        } else {
            while (txt[i] != ' ' && txt[i] != 0) { i++; }
            k2 = i;
        }
        const UnallocatedCString key = buf.substr(k, k2 - k);

        while (txt[i] == ' ') { i++; }
        std::int32_t v = i;
        std::int32_t v2{};
        if (txt[i] == '\'' || txt[i] == '"') {
            // quoted string
            char quote = txt[i++];
            v = i;
            while (txt[i] != quote && txt[i] != 0) { i++; }
            if (txt[i] != quote) {
                LogError()()("Unmatched quotes in: ")(txt)(".").Flush();
                return false;
            }
            v2 = i;
            i++;
        } else {
            while (txt[i] != ' ' && txt[i] != 0) { i++; }
            v2 = i;
        }
        const UnallocatedCString value = buf.substr(v, v2 - v);

        if (key.length() != 0 && value.length() != 0) {
            LogVerbose()()("Parsed: ")(key)(" = ")(value).Flush();
            mapOutput.insert(
                std::pair<UnallocatedCString, UnallocatedCString>(key, value));
        }
    }
    return true;
}

auto String::tokenize_enhanced(
    UnallocatedMap<UnallocatedCString, UnallocatedCString>& mapOutput) const
    -> bool
{
#if __has_include(<wordexp.h>)
    // fabcy-pansy parser that allows for multiple level of quotes nesting and
    // escaped quotes
    if (!Exists()) { return true; }

    wordexp_t exp_result;

    exp_result.we_wordc = 0;
    exp_result.we_wordv = nullptr;
    exp_result.we_offs = 0;

    if (wordexp(Get(), &exp_result, 0))  // non-zero == failure.
    {
        LogError()()("Error calling wordexp() "
                     "(to expand user-defined script args). Data: ")(
            static_cast<const opentxs::String&>(*this))(".")
            .Flush();
        //        wordfree(&exp_result);
        return false;
    }

    if ((exp_result.we_wordc > 0) && (nullptr != exp_result.we_wordv)) {
        // wordexp tokenizes by space (as well as expands, which is why I'm
        // using it.)
        // Therefore we need to iterate through the tokens, and create a single
        // string
        // with spaces between the tokens.
        //
        for (std::uint32_t i = 0;
             (i < (exp_result.we_wordc - 1)) &&
             (exp_result.we_wordv[i] != nullptr) &&
             (exp_result.we_wordv[i + 1] !=
              nullptr);  // odd man out. Only PAIRS of strings are processed!
             i += 2) {
            const UnallocatedCString str_key = exp_result.we_wordv[i];
            const UnallocatedCString str_val = exp_result.we_wordv[i + 1];

            LogVerbose()()("Parsed: ")(str_key)(" = ")(str_val).Flush();
            mapOutput.insert(std::pair<UnallocatedCString, UnallocatedCString>(
                str_key, str_val));
        }

        wordfree(&exp_result);
    }

    return true;
#else

    return false;
#endif
}
}  // namespace opentxs::implementation
