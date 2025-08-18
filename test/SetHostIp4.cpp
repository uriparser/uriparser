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

static void testIsWellFormedHostIp4(const char * candidate, bool expectedWellFormed) {
	const char * const first = candidate;
	const char * const afterLast = (candidate == NULL) ? NULL : (candidate + strlen(candidate));

	const UriBool actualWellFormed = uriIsWellFormedHostIp4A(first, afterLast);

	ASSERT_EQ(actualWellFormed, expectedWellFormed);
}

}  // namespace

TEST(IsWellFormedHostIp4, Null) {
	testIsWellFormedHostIp4(NULL, false);
}

TEST(IsWellFormedHostIp4, Empty) {
	testIsWellFormedHostIp4("", false);
}

TEST(IsWellFormedHostIp4, AllUnset) {
	testIsWellFormedHostIp4("0.0.0.0", true);
}

TEST(IsWellFormedHostIp4, AllSet) {
	testIsWellFormedHostIp4("255.255.255.255", true);
}

TEST(IsWellFormedHostIp4, ThreeOctets) {
	testIsWellFormedHostIp4("1.2.3", false);
}

TEST(IsWellFormedHostIp4, FiveOctets) {
	testIsWellFormedHostIp4("1.2.3.4.5", false);
}

TEST(IsWellFormedHostIp4, LeadingZeros) {
	testIsWellFormedHostIp4("01.2.3.4", false);
	testIsWellFormedHostIp4("1.02.3.4", false);
	testIsWellFormedHostIp4("1.2.03.4", false);
	testIsWellFormedHostIp4("1.2.3.04", false);
}

TEST(IsWellFormedHostIp4, Overflow) {
	testIsWellFormedHostIp4("256.2.3.4", false);
	testIsWellFormedHostIp4("1.256.3.4", false);
	testIsWellFormedHostIp4("1.2.256.4", false);
	testIsWellFormedHostIp4("1.2.3.256", false);
}
