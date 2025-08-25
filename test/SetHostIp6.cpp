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

TEST(SetHostIp6, NullUriOnly) {
	UriUriA * const uri = NULL;
	const char * const first = "::1";
	const char * const afterLast = first + strlen(first);
	ASSERT_EQ(uriSetHostIp6A(uri, first, afterLast), URI_ERROR_NULL);
}

TEST(SetHostIp6, NullFirstOnly) {
	UriUriA uri = {};
	const char * const host = "::1";
	const char * const first = NULL;
	const char * const afterLast = host + strlen(host);
	ASSERT_EQ(uriSetHostIp6A(&uri, first, afterLast), URI_ERROR_NULL);
}

TEST(SetHostIp6, NullAfterLastOnly) {
	UriUriA uri = {};
	const char * const first = "::1";
	const char * const afterLast = NULL;
	ASSERT_EQ(uriSetHostIp6A(&uri, first, afterLast), URI_ERROR_NULL);
}

TEST(SetHostIp6, NullValueLeavesOwnerAtFalse) {
	UriUriA uri = parseWellFormedUri("scheme://host/");
	EXPECT_EQ(uri.owner, URI_FALSE);  // self-test

	EXPECT_EQ(uriSetHostIp6A(&uri, NULL, NULL), URI_SUCCESS);

	EXPECT_EQ(uri.owner, URI_FALSE);  // i.e. still false

	uriFreeUriMembersA(&uri);
}

TEST(SetHostIp6, NonNullValueMakesOwner) {
	UriUriA uri = parseWellFormedUri("scheme://old/");
	const char * const first = "::1";
	const char * const afterLast = first + strlen(first);
	EXPECT_EQ(uri.owner, URI_FALSE);  // self-test

	EXPECT_EQ(uriSetHostIp6A(&uri, first, afterLast), URI_SUCCESS);

	EXPECT_EQ(uri.owner, URI_TRUE);  // i.e. now owned

	uriFreeUriMembersA(&uri);
}

TEST(SetHostIp6, NullValueAppliedDotInserted) {
	UriUriA uri = parseWellFormedUri("scheme://host//path1/path2");

	EXPECT_EQ(uriSetHostIp6A(&uri, NULL, NULL), URI_SUCCESS);

	assertUriEqual(&uri, "scheme:/.//path1/path2");  // i.e. not scheme://path1/path2

	uriFreeUriMembersA(&uri);
}

TEST(SetHostIp6, NullValueAppliedDotNotInserted) {
	UriUriA uri = parseWellFormedUri("//host/./path1/path2");

	EXPECT_EQ(uriSetHostIp6A(&uri, NULL, NULL), URI_SUCCESS);

	assertUriEqual(&uri, "/./path1/path2");  // i.e. not /././path1/path2

	uriFreeUriMembersA(&uri);
}

TEST(SetHostIp6, NullValueAppliedPriorNull) {
	UriUriA uri = parseWellFormedUri("scheme:/path");

	EXPECT_EQ(uriSetHostIp6A(&uri, NULL, NULL), URI_SUCCESS);

	assertUriEqual(&uri, "scheme:/path");

	uriFreeUriMembersA(&uri);
}

TEST(SetHostIp6, NullValueAppliedPriorIp4) {
	UriUriA uri = parseWellFormedUri("scheme://1.2.3.4/path");

	EXPECT_EQ(uriSetHostIp6A(&uri, NULL, NULL), URI_SUCCESS);

	assertUriEqual(&uri, "scheme:/path");

	uriFreeUriMembersA(&uri);
}

TEST(SetHostIp6, NullValueAppliedPriorIp6) {
	UriUriA uri = parseWellFormedUri("scheme://[::1]/path");

	EXPECT_EQ(uriSetHostIp6A(&uri, NULL, NULL), URI_SUCCESS);

	assertUriEqual(&uri, "scheme:/path");

	uriFreeUriMembersA(&uri);
}

TEST(SetHostIp6, NullValueAppliedPriorIpFuture) {
	UriUriA uri = parseWellFormedUri("scheme://[v7.host]/path");

	EXPECT_EQ(uriSetHostIp6A(&uri, NULL, NULL), URI_SUCCESS);

	assertUriEqual(&uri, "scheme:/path");

	uriFreeUriMembersA(&uri);
}

TEST(SetHostIp6, NullValueAppliedPriorRegName) {
	UriUriA uri = parseWellFormedUri("scheme://host/path");

	EXPECT_EQ(uriSetHostIp6A(&uri, NULL, NULL), URI_SUCCESS);

	assertUriEqual(&uri, "scheme:/path");

	uriFreeUriMembersA(&uri);
}

TEST(SetHostIp6, NonNullValueAppliedNonEmptyPriorNull) {
	UriUriA uri = parseWellFormedUri("scheme:");
	const char * const first = "::1";
	const char * const afterLast = first + strlen(first);

	EXPECT_EQ(uriSetHostIp6A(&uri, first, afterLast), URI_SUCCESS);

	assertUriEqual(&uri, "scheme://[0000:0000:0000:0000:0000:0000:0000:0001]");

	uriFreeUriMembersA(&uri);
}

TEST(SetHostIp6, NonNullValueAppliedNonEmptyPriorIp4) {
	UriUriA uri = parseWellFormedUri("//1.2.3.4");
	const char * const first = "::1";
	const char * const afterLast = first + strlen(first);

	EXPECT_EQ(uriSetHostIp6A(&uri, first, afterLast), URI_SUCCESS);

	assertUriEqual(&uri, "//[0000:0000:0000:0000:0000:0000:0000:0001]");

	uriFreeUriMembersA(&uri);
}

TEST(SetHostIp6, NonNullValueAppliedNonEmptyPriorIp6) {
	UriUriA uri = parseWellFormedUri("//[::1]");
	const char * const first = "::2";
	const char * const afterLast = first + strlen(first);

	EXPECT_EQ(uriSetHostIp6A(&uri, first, afterLast), URI_SUCCESS);

	assertUriEqual(&uri, "//[0000:0000:0000:0000:0000:0000:0000:0002]");

	uriFreeUriMembersA(&uri);
}

TEST(SetHostIp6, NonNullValueAppliedNonEmptyPriorIpFuture) {
	UriUriA uri = parseWellFormedUri("//[v7.host]");
	const char * const first = "::1";
	const char * const afterLast = first + strlen(first);

	EXPECT_EQ(uriSetHostIp6A(&uri, first, afterLast), URI_SUCCESS);

	assertUriEqual(&uri, "//[0000:0000:0000:0000:0000:0000:0000:0001]");

	uriFreeUriMembersA(&uri);
}

TEST(SetHostIp6, NonNullValueAppliedNonEmptyPriorRegName) {
	UriUriA uri = parseWellFormedUri("//hostname.test");
	const char * const first = "::1";
	const char * const afterLast = first + strlen(first);

	EXPECT_EQ(uriSetHostIp6A(&uri, first, afterLast), URI_SUCCESS);

	assertUriEqual(&uri, "//[0000:0000:0000:0000:0000:0000:0000:0001]");

	uriFreeUriMembersA(&uri);
}

TEST(SetHostIp6, MalformedValueRejected) {
	UriUriA uri = parseWellFormedUri("scheme://host/");
	const char * const first = "not well-formed";
	const char * const afterLast = first + strlen(first);

	EXPECT_EQ(uriSetHostIp6A(&uri, first, afterLast), URI_ERROR_SYNTAX);

	uriFreeUriMembersA(&uri);
}

TEST(SetHostIp6, UriWithPortRejected) {
	UriUriA uri = parseWellFormedUri("//host:1234");
	EXPECT_TRUE(uri.portText.first != NULL);  // self-test

	EXPECT_EQ(uriSetHostIp6A(&uri, NULL, NULL), URI_ERROR_SETHOST_PORT_SET);

	uriFreeUriMembersA(&uri);
}

TEST(SetHostIp6, UriWithUserInfoRejected) {
	UriUriA uri = parseWellFormedUri("//user:password@host");
	EXPECT_TRUE(uri.userInfo.first != NULL);  // self-test

	EXPECT_EQ(uriSetHostIp6A(&uri, NULL, NULL), URI_ERROR_SETHOST_USERINFO_SET);

	uriFreeUriMembersA(&uri);
}
