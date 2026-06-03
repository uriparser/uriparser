/*
 * uriparser - RFC 3986 URI parsing library
 *
 * Copyright (C) 2026, Joshua W. Windle <joshua.w.windle@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <uriparser/Uri.h>
#include <gtest/gtest.h>

#include <cstdint>
#include <cstring>

#if !defined(_WIN32)
#  include <sys/mman.h>  // mmap, mprotect, munmap
#  include <unistd.h>  // sysconf
#endif

extern "C" {
bool uriRangeEqualsA(const UriTextRangeA * a, const UriTextRangeA * b);
}

#if !defined(_WIN32) && (UINTPTR_MAX > UINT32_MAX)
namespace {
static size_t roundUpToPageSize(size_t value, size_t pageSize) {
    const size_t remainder = value % pageSize;
    if (remainder == 0U) {
        return value;
    }

    const size_t padding = pageSize - remainder;
    EXPECT_LE(padding, SIZE_MAX - value);
    return value + padding;
}

static size_t queryPageSize() {
    const long pageSize = sysconf(_SC_PAGESIZE);
    EXPECT_GT(pageSize, 0);
    return static_cast<size_t>(pageSize);
}
}  // namespace

TEST(UriSuite, TestRangeComparisonDoesNotWrapLengthChecksOn64Bit) {
    const size_t hugeLen = (static_cast<size_t>(1) << 32) + 10U;
    const size_t shortLen = 10U;
    const size_t pageSize = queryPageSize();
    const size_t hugeMapLen = roundUpToPageSize(hugeLen, pageSize);
    const size_t shortMapLen = pageSize * 2U;

    // Reserve a huge virtual range, with access disabled by default.
    char * const hugeBase = static_cast<char *>(
            mmap(NULL, hugeMapLen, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0));
    ASSERT_NE(hugeBase, MAP_FAILED);
    // Allow access to the first page (and that only).
    ASSERT_EQ(0, mprotect(hugeBase, pageSize, PROT_READ | PROT_WRITE));
    // Fill the readable prefix so the vulnerable implementation reaches
    // content comparison after truncating the large length.
    memset(hugeBase, 'a', pageSize);

    char * const shortBase = static_cast<char *>(mmap(NULL, shortMapLen,
            PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0));
    ASSERT_NE(shortBase, MAP_FAILED);
    // Guard the following page so any over-read beyond shortLen faults.
    ASSERT_EQ(0, mprotect(shortBase + pageSize, pageSize, PROT_NONE));

    char * const shortStart = shortBase + pageSize - shortLen;
    memset(shortStart, 'a', shortLen);

    UriTextRangeA hugeRange;
    hugeRange.first = hugeBase;
    hugeRange.afterLast = hugeBase + hugeLen;

    UriTextRangeA shortRange;
    shortRange.first = shortStart;
    shortRange.afterLast = shortStart + shortLen;

    const bool comparison = uriRangeEqualsA(&hugeRange, &shortRange);

    EXPECT_EQ(false, comparison);

    EXPECT_EQ(0, munmap(hugeBase, hugeMapLen));
    EXPECT_EQ(0, munmap(shortBase, shortMapLen));
}
#endif
