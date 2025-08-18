/*
 * uriparser - RFC 3986 URI parsing library
 *
 * Copyright (C) 2025, Sebastian Pipping <sebastian@pipping.org>
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

#include <gtest/gtest.h>

#include <uriparser/Uri.h>

namespace {

static void testIsWellFormedHostIp6(const char * candidate, bool expectedWellFormed) {
	const char * const first = candidate;
	const char * const afterLast = (candidate == NULL) ? NULL : (candidate + strlen(candidate));

	const UriBool actualWellFormed = (uriIsWellFormedHostIp6A(first, afterLast) == URI_SUCCESS)
		? URI_TRUE
		: URI_FALSE;

	ASSERT_EQ(actualWellFormed, expectedWellFormed);
}

}  // namespace

TEST(IsWellFormedHostIp6, Null) {
	testIsWellFormedHostIp6(NULL, false);
}

TEST(IsWellFormedHostIp6, Empty) {
	testIsWellFormedHostIp6("", false);
}

TEST(IsWellFormedHostIp6, Ip4EmbeddingAllUnset) {
	testIsWellFormedHostIp6("::0.0.0.0", true);
}

TEST(IsWellFormedHostIp6, Ip4EmbeddingAllSet) {
	testIsWellFormedHostIp6("::255.255.255.255", true);
}

TEST(IsWellFormedHostIp6, Ip4EmbeddingThreeOctets) {
	testIsWellFormedHostIp6("::1.2.3", false);
}

TEST(IsWellFormedHostIp6, Ip4EmbeddingFiveOctets) {
	testIsWellFormedHostIp6("::1.2.3.4.5", false);
}

TEST(IsWellFormedHostIp6, Ip4EmbeddingLeadingZeros) {
	testIsWellFormedHostIp6("::01.2.3.4", false);
	testIsWellFormedHostIp6("::1.02.3.4", false);
	testIsWellFormedHostIp6("::1.2.03.4", false);
	testIsWellFormedHostIp6("::1.2.3.04", false);
}

TEST(IsWellFormedHostIp6, Ip4EmbeddingOverflow) {
	testIsWellFormedHostIp6("::256.2.3.4", false);
	testIsWellFormedHostIp6("::1.256.3.4", false);
	testIsWellFormedHostIp6("::1.2.256.4", false);
	testIsWellFormedHostIp6("::1.2.3.256", false);
}

TEST(IsWellFormedHostIp6, Ip4EmbeddingHex) {
	testIsWellFormedHostIp6("::a.2.3.4", false);
	testIsWellFormedHostIp6("::1.a.3.4", false);
	testIsWellFormedHostIp6("::1.2.a.4", false);
	testIsWellFormedHostIp6("::1.2.3.a", false);
}

TEST(IsWellFormedHostIp6, Uppercase) {
    testIsWellFormedHostIp6("ABCD:EF01:2345:6789:ABCD:EF01:2345:6789", true);
}

TEST(IsWellFormedHostIp6, Lowercase) {
    testIsWellFormedHostIp6("abcd:ef01:2345:6789:abcd:ef01:2345:6789", true);
}

TEST(IsWellFormedHostIp6, MaxLengthViolation) {
    testIsWellFormedHostIp6("aaaa:aaaa:aaaa:aaaa:aaaa:aaaa:aaaa:aaaa" "X", false);
}

TEST(IsWellFormedHostIp6, NineQuads) {
    testIsWellFormedHostIp6("1:2:3:4:5:6:7:8:9", false);
}

TEST(IsWellFormedHostIp6, SevenQuads) {
    testIsWellFormedHostIp6("1:2:3:4:5:6:7", false);
}

TEST(IsWellFormedHostIp6, AllUnset) {
    testIsWellFormedHostIp6("::", true);
}

TEST(IsWellFormedHostIp6, Loopback) {
    testIsWellFormedHostIp6("::1", true);
}

TEST(IsWellFormedHostIp6, SparseLeadingZeros) {
    testIsWellFormedHostIp6("01:02:03:04:05:06:07:08", true);
}

TEST(IsWellFormedHostIp6, SingleZipper) {
    testIsWellFormedHostIp6("1::8", true);
}

TEST(IsWellFormedHostIp6, TwoZippers) {
    testIsWellFormedHostIp6("1::4::8", false);
}

TEST(IsWellFormedHostIp6, Overzipped) {
    testIsWellFormedHostIp6("::1:2:3:4:5:6:7:8", false);
    testIsWellFormedHostIp6("1:2:3:4::5:6:7:8", false);
    testIsWellFormedHostIp6("1:2:3:4:5:6:7:8::", false);
}

TEST(IsWellFormedHostIp6, NonHex) {
    testIsWellFormedHostIp6("000g::", false);
    testIsWellFormedHostIp6("00g0::", false);
    testIsWellFormedHostIp6("0g00::", false);
    testIsWellFormedHostIp6("g000::", false);

    testIsWellFormedHostIp6("000G::", false);
    testIsWellFormedHostIp6("00G0::", false);
    testIsWellFormedHostIp6("0G00::", false);
    testIsWellFormedHostIp6("G000::", false);
}

TEST(IsWellFormedHostIp6, IpFuture) {
    testIsWellFormedHostIp6("v7.host", false);
    testIsWellFormedHostIp6("V7.host", false);
}
