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

static void testIsWellFormedHostIpFuture(const char * candidate, bool expectedWellFormed) {
	const char * const first = candidate;
	const char * const afterLast = (candidate == NULL) ? NULL : (candidate + strlen(candidate));

	const UriBool actualWellFormed = (uriIsWellFormedHostIpFutureA(first, afterLast) == URI_SUCCESS)
		? URI_TRUE
		: URI_FALSE;

	ASSERT_EQ(actualWellFormed, expectedWellFormed);
}

}  // namespace

TEST(IsWellFormedHostIpFuture, Null) {
	testIsWellFormedHostIpFuture(NULL, false);
}

TEST(IsWellFormedHostIpFuture, Empty) {
	testIsWellFormedHostIpFuture("", false);
}

TEST(IsWellFormedHostIpFuture, Ip6) {
    testIsWellFormedHostIpFuture("abcd:ef01:2345:6789:abcd:ef01:2345:6789", false);
}

TEST(IsWellFormedHostIpFuture, Lowercase) {
    testIsWellFormedHostIpFuture("v7.host", true);
}

TEST(IsWellFormedHostIpFuture, Uppercase) {
    testIsWellFormedHostIpFuture("V7.HOST", true);
}
