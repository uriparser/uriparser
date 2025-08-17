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

static void testIsWellFormedHostRegName(const char * candidate, bool expectedWellFormed) {
	const char * const first = candidate;
	const char * const afterLast = (candidate == NULL) ? NULL : (candidate + strlen(candidate));

	const UriBool actualWellFormed = uriIsWellFormedHostRegNameA(first, afterLast);

	ASSERT_EQ(actualWellFormed, expectedWellFormed);
}

}  // namespace

TEST(IsWellFormedHostRegName, Null) {
	testIsWellFormedHostRegName(NULL, false);
}

TEST(IsWellFormedHostRegName, Empty) {
	testIsWellFormedHostRegName("", true);
}

TEST(IsWellFormedHostRegName, AllowedCharacters) {
	// The related grammar subset is this:
	//
	//   reg-name = *( unreserved / pct-encoded / sub-delims )
	//   unreserved  = ALPHA / DIGIT / "-" / "." / "_" / "~"
	//   pct-encoded = "%" HEXDIG HEXDIG
	//   sub-delims  = "!" / "$" / "&" / "'" / "(" / ")"
	//               / "*" / "+" / "," / ";" / "="
	//
	// NOTE: Percent encoding has dedicated tests further down
	testIsWellFormedHostRegName(
			"0123456789"
			"ABCDEF"
			"abcdef"
			"gGhHiIjJkKlLmMnNoOpPqQrRsStTuUvVwWxXyYzZ"
			"-._~"
			"!$&'()*+,;=",
			true);
}

TEST(IsWellFormedHostRegName, ForbiddenCharacters) {
	testIsWellFormedHostRegName(" ", false);
}

TEST(IsWellFormedHostRegName, PercentEncodingWellFormed) {
	testIsWellFormedHostRegName("%" "aa" "%" "AA", true);
}

TEST(IsWellFormedHostRegName, PercentEncodingMalformedCutOff1) {
	testIsWellFormedHostRegName("%", false);
}

TEST(IsWellFormedHostRegName, PercentEncodingMalformedCutOff2) {
	testIsWellFormedHostRegName("%" "a", false);
}

TEST(IsWellFormedHostRegName, PercentEncodingMalformedForbiddenCharacter1) {
	testIsWellFormedHostRegName("%" "ga", false);
}

TEST(IsWellFormedHostRegName, PercentEncodingMalformedForbiddenCharacter2) {
	testIsWellFormedHostRegName("%" "ag", false);
}
