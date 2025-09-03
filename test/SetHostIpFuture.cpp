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

static void assertIpFutureMatchHostText(const UriUriA * uri) {
	ASSERT_TRUE(uri != NULL);
	EXPECT_TRUE(uri->hostText.first != NULL);
	EXPECT_TRUE(uri->hostText.afterLast != NULL);
	EXPECT_EQ(uri->hostText.first, uri->hostData.ipFuture.first);
	EXPECT_EQ(uri->hostText.afterLast, uri->hostData.ipFuture.afterLast);
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

TEST(SetHostIpFuture, NullUriOnly) {
	UriUriA * const uri = NULL;
	const char * const first = "v7.host";
	const char * const afterLast = first + strlen(first);
	ASSERT_EQ(uriSetHostIpFutureA(uri, first, afterLast), URI_ERROR_NULL);
}

TEST(SetHostIpFuture, NullFirstOnly) {
	UriUriA uri = {};
	const char * const host = "v7.host";
	const char * const first = NULL;
	const char * const afterLast = host + strlen(host);
	ASSERT_EQ(uriSetHostIpFutureA(&uri, first, afterLast), URI_ERROR_NULL);
}

TEST(SetHostIpFuture, NullAfterLastOnly) {
	UriUriA uri = {};
	const char * const first = "v7.host";
	const char * const afterLast = NULL;
	ASSERT_EQ(uriSetHostIpFutureA(&uri, first, afterLast), URI_ERROR_NULL);
}

TEST(SetHostIpFuture, NullValueLeavesOwnerAtFalse) {
	UriUriA uri = parseWellFormedUri("scheme://host/");
	EXPECT_EQ(uri.owner, URI_FALSE);  // self-test

	EXPECT_EQ(uriSetHostIpFutureA(&uri, NULL, NULL), URI_SUCCESS);

	EXPECT_EQ(uri.owner, URI_FALSE);  // i.e. still false

	uriFreeUriMembersA(&uri);
}

TEST(SetHostIpFuture, NonNullValueMakesOwner) {
	UriUriA uri = parseWellFormedUri("scheme://old/");
	const char * const first = "v7.host";
	const char * const afterLast = first + strlen(first);
	EXPECT_EQ(uri.owner, URI_FALSE);  // self-test

	EXPECT_EQ(uriSetHostIpFutureA(&uri, first, afterLast), URI_SUCCESS);

	EXPECT_EQ(uri.owner, URI_TRUE);  // i.e. now owned

	uriFreeUriMembersA(&uri);
}

TEST(SetHostIpFuture, NullValueAppliedDotInserted) {
	UriUriA uri = parseWellFormedUri("scheme://host//path1/path2");

	EXPECT_EQ(uriSetHostIpFutureA(&uri, NULL, NULL), URI_SUCCESS);

	assertUriEqual(&uri, "scheme:/.//path1/path2");  // i.e. not scheme://path1/path2

	uriFreeUriMembersA(&uri);
}

TEST(SetHostIpFuture, NullValueAppliedDotNotInserted) {
	UriUriA uri = parseWellFormedUri("//host/./path1/path2");

	EXPECT_EQ(uriSetHostIpFutureA(&uri, NULL, NULL), URI_SUCCESS);

	assertUriEqual(&uri, "/./path1/path2");  // i.e. not /././path1/path2

	uriFreeUriMembersA(&uri);
}

TEST(SetHostIpFuture, NullValueAppliedPriorNull) {
	UriUriA uri = parseWellFormedUri("scheme:/path");

	EXPECT_EQ(uriSetHostIpFutureA(&uri, NULL, NULL), URI_SUCCESS);

	assertUriEqual(&uri, "scheme:/path");

	uriFreeUriMembersA(&uri);
}

TEST(SetHostIpFuture, NullValueAppliedPriorIp4) {
	UriUriA uri = parseWellFormedUri("scheme://1.2.3.4/path");

	EXPECT_EQ(uriSetHostIpFutureA(&uri, NULL, NULL), URI_SUCCESS);

	assertUriEqual(&uri, "scheme:/path");

	uriFreeUriMembersA(&uri);
}

TEST(SetHostIpFuture, NullValueAppliedPriorIp6) {
	UriUriA uri = parseWellFormedUri("scheme://[::1]/path");

	EXPECT_EQ(uriSetHostIpFutureA(&uri, NULL, NULL), URI_SUCCESS);

	assertUriEqual(&uri, "scheme:/path");

	uriFreeUriMembersA(&uri);
}

TEST(SetHostIpFuture, NullValueAppliedPriorIpFuture) {
	UriUriA uri = parseWellFormedUri("scheme://[v7.host]/path");

	EXPECT_EQ(uriSetHostIpFutureA(&uri, NULL, NULL), URI_SUCCESS);

	assertUriEqual(&uri, "scheme:/path");

	uriFreeUriMembersA(&uri);
}

TEST(SetHostIpFuture, NullValueAppliedPriorRegName) {
	UriUriA uri = parseWellFormedUri("scheme://host/path");

	EXPECT_EQ(uriSetHostIpFutureA(&uri, NULL, NULL), URI_SUCCESS);

	assertUriEqual(&uri, "scheme:/path");

	uriFreeUriMembersA(&uri);
}

TEST(SetHostIpFuture, NonNullValueAppliedNonEmptyPriorNull) {
	UriUriA uri = parseWellFormedUri("scheme:");
	const char * const first = "v7.host";
	const char * const afterLast = first + strlen(first);

	EXPECT_EQ(uriSetHostIpFutureA(&uri, first, afterLast), URI_SUCCESS);

	assertUriEqual(&uri, "scheme://[v7.host]");
	assertIpFutureMatchHostText(&uri);

	uriFreeUriMembersA(&uri);
}

TEST(SetHostIpFuture, NonNullValueAppliedNonEmptyPriorIp4) {
	UriUriA uri = parseWellFormedUri("//1.2.3.4");
	const char * const first = "v7.host";
	const char * const afterLast = first + strlen(first);

	EXPECT_EQ(uriSetHostIpFutureA(&uri, first, afterLast), URI_SUCCESS);

	assertUriEqual(&uri, "//[v7.host]");
	assertIpFutureMatchHostText(&uri);

	uriFreeUriMembersA(&uri);
}

TEST(SetHostIpFuture, NonNullValueAppliedNonEmptyPriorIp6) {
	UriUriA uri = parseWellFormedUri("//[::1]");
	const char * const first = "v7.host";
	const char * const afterLast = first + strlen(first);

	EXPECT_EQ(uriSetHostIpFutureA(&uri, first, afterLast), URI_SUCCESS);

	assertUriEqual(&uri, "//[v7.host]");
	assertIpFutureMatchHostText(&uri);

	uriFreeUriMembersA(&uri);
}

TEST(SetHostIpFuture, NonNullValueAppliedNonEmptyPriorIpFuture) {
	UriUriA uri = parseWellFormedUri("//[v7.old]");
	const char * const first = "v7.new";
	const char * const afterLast = first + strlen(first);

	EXPECT_EQ(uriSetHostIpFutureA(&uri, first, afterLast), URI_SUCCESS);

	assertUriEqual(&uri, "//[v7.new]");
	assertIpFutureMatchHostText(&uri);

	uriFreeUriMembersA(&uri);
}

TEST(SetHostIpFuture, NonNullValueAppliedNonEmptyPriorRegName) {
	UriUriA uri = parseWellFormedUri("//hostname.test");
	const char * const first = "v7.host";
	const char * const afterLast = first + strlen(first);

	EXPECT_EQ(uriSetHostIpFutureA(&uri, first, afterLast), URI_SUCCESS);

	assertUriEqual(&uri, "//[v7.host]");
	assertIpFutureMatchHostText(&uri);

	uriFreeUriMembersA(&uri);
}

TEST(SetHostIpFuture, MalformedValueRejected) {
	UriUriA uri = parseWellFormedUri("scheme://host/");
	const char * const first = "not well-formed";
	const char * const afterLast = first + strlen(first);

	EXPECT_EQ(uriSetHostIpFutureA(&uri, first, afterLast), URI_ERROR_SYNTAX);

	uriFreeUriMembersA(&uri);
}

TEST(SetHostIpFuture, UriWithPortRejected) {
	UriUriA uri = parseWellFormedUri("//host:1234");
	EXPECT_TRUE(uri.portText.first != NULL);  // self-test

	EXPECT_EQ(uriSetHostIpFutureA(&uri, NULL, NULL), URI_ERROR_SETHOST_PORT_SET);

	uriFreeUriMembersA(&uri);
}

TEST(SetHostIpFuture, UriWithUserInfoRejected) {
	UriUriA uri = parseWellFormedUri("//user:password@host");
	EXPECT_TRUE(uri.userInfo.first != NULL);  // self-test

	EXPECT_EQ(uriSetHostIpFutureA(&uri, NULL, NULL), URI_ERROR_SETHOST_USERINFO_SET);

	uriFreeUriMembersA(&uri);
}
