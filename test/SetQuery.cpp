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

#undef NDEBUG  // because we rely on assert(3) further down

#include <cassert>

#include <gtest/gtest.h>

#include <uriparser/Uri.h>

namespace {

static void testIsWellFormedQuery(const char * candidate, bool expectedWellFormed) {
	const char * const first = candidate;
	const char * const afterLast = (candidate == NULL) ? NULL : (candidate + strlen(candidate));

	const UriBool actualWellFormed = uriIsWellFormedQueryA(first, afterLast);

	ASSERT_EQ(actualWellFormed, expectedWellFormed);
}

static UriUriA parseWellFormedUri(const char * text) {
	UriUriA uri;
	const int error = uriParseSingleUriA(&uri, text, NULL);
	// NOTE: we cannot use ASSERT_EQ here because of the outer non-void return type
	assert(error == URI_SUCCESS);
	return uri;
}

static void assertUriEqual(const UriUriA * uri, const char * expected) {
	int charsRequired = -1;
	ASSERT_EQ(uriToStringCharsRequiredA(uri, &charsRequired), URI_SUCCESS);
	ASSERT_TRUE(charsRequired >= 0);

	char * const buffer = (char *)malloc(charsRequired + 1);
	ASSERT_TRUE(buffer != NULL);

	ASSERT_EQ(uriToStringA(buffer, uri, charsRequired + 1, NULL), URI_SUCCESS);

	EXPECT_STREQ(buffer, expected);

	free(buffer);
}

}  // namespace

TEST(IsWellFormedQuery, Null) {
	testIsWellFormedQuery(NULL, false);
}

TEST(IsWellFormedQuery, Empty) {
	testIsWellFormedQuery("", true);
}

TEST(IsWellFormedQuery, AllowedCharacters) {
	// The related grammar subset is this:
	//
	//   query       = *( pchar / "/" / "?" )
	//   pchar       = unreserved / pct-encoded / sub-delims / ":" / "@"
	//   unreserved  = ALPHA / DIGIT / "-" / "." / "_" / "~"
	//   pct-encoded = "%" HEXDIG HEXDIG
	//   sub-delims  = "!" / "$" / "&" / "'" / "(" / ")"
	//               / "*" / "+" / "," / ";" / "="
	//
	// NOTE: Percent encoding has dedicated tests further down
	testIsWellFormedQuery(
			"0123456789"
			"ABCDEF"
			"abcdef"
			"gGhHiIjJkKlLmMnNoOpPqQrRsStTuUvVwWxXyYzZ"
			"-._~"
			"!$&'()*+,;="
			":@"
			"/?",
			true);
}

TEST(IsWellFormedQuery, ForbiddenCharacters) {
	testIsWellFormedQuery(" ", false);
	testIsWellFormedQuery("#", false);
}

TEST(IsWellFormedQuery, PercentEncodingWellFormed) {
	testIsWellFormedQuery("%" "aa" "%" "AA", true);
}

TEST(IsWellFormedQuery, PercentEncodingMalformedCutOff1) {
	testIsWellFormedQuery("%", false);
}

TEST(IsWellFormedQuery, PercentEncodingMalformedCutOff2) {
	testIsWellFormedQuery("%" "a", false);
}

TEST(IsWellFormedQuery, PercentEncodingMalformedForbiddenCharacter1) {
	testIsWellFormedQuery("%" "ga", false);
}

TEST(IsWellFormedQuery, PercentEncodingMalformedForbiddenCharacter2) {
	testIsWellFormedQuery("%" "ag", false);
}

TEST(SetQuery, NullUriOnly) {
	UriUriA * const uri = NULL;
	const char * const first = "k1=v1";
	const char * const afterLast = first + strlen(first);
	ASSERT_EQ(uriSetQueryA(uri, first, afterLast), URI_ERROR_NULL);
}

TEST(SetQuery, NullFirstOnly) {
	UriUriA uri = {};
	const char * const query = "k1=v1";
	const char * const first = NULL;
	const char * const afterLast = query + strlen(query);
	ASSERT_EQ(uriSetQueryA(&uri, first, afterLast), URI_ERROR_NULL);
}

TEST(SetQuery, NullAfterLastOnly) {
	UriUriA uri = {};
	const char * const first = "k1=v1";
	const char * const afterLast = NULL;
	ASSERT_EQ(uriSetQueryA(&uri, first, afterLast), URI_ERROR_NULL);
}

TEST(SetQuery, NullValueLeavesOwnerAtFalse) {
	UriUriA uri = parseWellFormedUri("scheme://host/?query");
	EXPECT_EQ(uri.owner, URI_FALSE);  // self-test

	EXPECT_EQ(uriSetQueryA(&uri, NULL, NULL), URI_SUCCESS);

	EXPECT_EQ(uri.owner, URI_FALSE);  // i.e. still false

	uriFreeUriMembersA(&uri);
}

TEST(SetQuery, NonNullValueMakesOwner) {
	UriUriA uri = parseWellFormedUri("scheme://host/?old");
	const char * const first = "new";
	const char * const afterLast = first + strlen(first);
	EXPECT_EQ(uri.owner, URI_FALSE);  // self-test

	EXPECT_EQ(uriSetQueryA(&uri, first, afterLast), URI_SUCCESS);

	EXPECT_EQ(uri.owner, URI_TRUE);  // i.e. now owned

	uriFreeUriMembersA(&uri);
}

TEST(SetQuery, NullValueApplied) {
	UriUriA uri = parseWellFormedUri("scheme://host/?query");

	EXPECT_EQ(uriSetQueryA(&uri, NULL, NULL), URI_SUCCESS);

	assertUriEqual(&uri, "scheme://host/");

	uriFreeUriMembersA(&uri);
}

TEST(SetQuery, NonNullValueAppliedEmpty) {
	UriUriA uri = parseWellFormedUri("scheme://host/?query");
	const char * const empty = "";

	EXPECT_EQ(uriSetQueryA(&uri, empty, empty), URI_SUCCESS);

	assertUriEqual(&uri, "scheme://host/?");

	uriFreeUriMembersA(&uri);
}

TEST(SetQuery, NonNullValueAppliedNonEmpty) {
	UriUriA uri = parseWellFormedUri("scheme://host/?old");
	const char * const first = "new";
	const char * const afterLast = first + strlen(first);

	EXPECT_EQ(uriSetQueryA(&uri, first, afterLast), URI_SUCCESS);

	assertUriEqual(&uri, "scheme://host/?new");

	uriFreeUriMembersA(&uri);
}

TEST(SetQuery, MalformedValueRejected) {
	UriUriA uri = parseWellFormedUri("scheme://host/?query");
	const char * const first = "not well-formed";
	const char * const afterLast = first + strlen(first);

	EXPECT_EQ(uriSetQueryA(&uri, first, afterLast), URI_ERROR_SYNTAX);

	uriFreeUriMembersA(&uri);
}

TEST(SetQuery, UriWithoutHostTolerated) {
	UriUriA uri = parseWellFormedUri("/no/host/here");
	const char * const first = "k1=v1";
	const char * const afterLast = first + strlen(first);
	EXPECT_TRUE(uri.hostText.first == NULL);  // self-test

	EXPECT_EQ(uriSetQueryA(&uri, first, afterLast), URI_SUCCESS);

	assertUriEqual(&uri, "/no/host/here?k1=v1");

	uriFreeUriMembersA(&uri);
}
