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

static void assertMalformedHostValueRejected(const char * text) {
    UriUriA uri = parseWellFormedUri("scheme://host/");
    const char * const first = text;
    const char * const afterLast = text + strlen(text);

    EXPECT_EQ(uriSetHostAutoA(&uri, first, afterLast), URI_ERROR_SYNTAX);

    uriFreeUriMembersA(&uri);
}

}  // namespace

TEST(SetHostAuto, NullUriOnly) {
    UriUriA * const uri = NULL;
    const char * const first = "localhost";
    const char * const afterLast = first + strlen(first);
    ASSERT_EQ(uriSetHostAutoA(uri, first, afterLast), URI_ERROR_NULL);
}

TEST(SetHostAuto, NullFirstOnly) {
    UriUriA uri = {};
    const char * const fragment = "localhost";
    const char * const first = NULL;
    const char * const afterLast = fragment + strlen(fragment);
    ASSERT_EQ(uriSetHostAutoA(&uri, first, afterLast), URI_ERROR_NULL);
}

TEST(SetHostAuto, NullAfterLastOnly) {
    UriUriA uri = {};
    const char * const first = "localhost";
    const char * const afterLast = NULL;
    ASSERT_EQ(uriSetHostAutoA(&uri, first, afterLast), URI_ERROR_NULL);
}

TEST(SetHostAuto, NullValueLeavesOwnerAtFalse) {
    UriUriA uri = parseWellFormedUri("scheme://host/");
    EXPECT_EQ(uri.owner, URI_FALSE);  // self-test

    EXPECT_EQ(uriSetHostAutoA(&uri, NULL, NULL), URI_SUCCESS);

    EXPECT_EQ(uri.owner, URI_FALSE);  // i.e. still false

    uriFreeUriMembersA(&uri);
}

TEST(SetHostAuto, NonNullValueMakesOwner) {
    UriUriA uri = parseWellFormedUri("scheme://old/");
    const char * const first = "new";
    const char * const afterLast = first + strlen(first);
    EXPECT_EQ(uri.owner, URI_FALSE);  // self-test

    EXPECT_EQ(uriSetHostAutoA(&uri, first, afterLast), URI_SUCCESS);

    EXPECT_EQ(uri.owner, URI_TRUE);  // i.e. now owned

    uriFreeUriMembersA(&uri);
}

TEST(SetHostAuto, NullValueApplied) {
    UriUriA uri = parseWellFormedUri("scheme://host/path");

    EXPECT_EQ(uriSetHostAutoA(&uri, NULL, NULL), URI_SUCCESS);

    assertUriEqual(&uri, "scheme:/path");

    uriFreeUriMembersA(&uri);
}

TEST(SetHostAuto, NonNullValueAppliedEmpty) {
    UriUriA uri = parseWellFormedUri("scheme://host/path");
    const char * const empty = "";

    EXPECT_EQ(uriSetHostAutoA(&uri, empty, empty), URI_SUCCESS);

    assertUriEqual(&uri, "scheme:///path");

    uriFreeUriMembersA(&uri);
}

TEST(SetHostAuto, NonNullValueAppliedNonEmptyIp4) {
    UriUriA uri = parseWellFormedUri("scheme://host/path");
    const char * const first = "1.2.3.4";
    const char * const afterLast = first + strlen(first);

    EXPECT_EQ(uriSetHostAutoA(&uri, first, afterLast), URI_SUCCESS);

    assertUriEqual(&uri, "scheme://1.2.3.4/path");

    uriFreeUriMembersA(&uri);
}

TEST(SetHostAuto, NonNullValueAppliedNonEmptyIp6) {
    UriUriA uri = parseWellFormedUri("scheme://host/path");
    const char * const first = "[::1]";
    const char * const afterLast = first + strlen(first);

    EXPECT_EQ(uriSetHostAutoA(&uri, first, afterLast), URI_SUCCESS);

    assertUriEqual(&uri, "scheme://[0000:0000:0000:0000:0000:0000:0000:0001]/path");

    uriFreeUriMembersA(&uri);
}

TEST(SetHostAuto, NonNullValueAppliedNonEmptyIpFuture) {
    UriUriA uri = parseWellFormedUri("scheme://host/path");
    const char * const first = "[v7.host]";
    const char * const afterLast = first + strlen(first);

    EXPECT_EQ(uriSetHostAutoA(&uri, first, afterLast), URI_SUCCESS);

    assertUriEqual(&uri, "scheme://[v7.host]/path");

    uriFreeUriMembersA(&uri);
}

TEST(SetHostAuto, NonNullValueAppliedNonEmptyRegName) {
    UriUriA uri = parseWellFormedUri("scheme://old/path");
    const char * const first = "new";
    const char * const afterLast = first + strlen(first);

    EXPECT_EQ(uriSetHostAutoA(&uri, first, afterLast), URI_SUCCESS);

    assertUriEqual(&uri, "scheme://new/path");

    uriFreeUriMembersA(&uri);
}

TEST(SetHostAuto, MalformedValueRejectedIp6BothSquareBracketsMissing) {
    assertMalformedHostValueRejected("::1");
}

TEST(SetHostAuto, MalformedValueRejectedIp6ClosingSquareBracketMissing) {
    assertMalformedHostValueRejected("[::1");
}

TEST(SetHostAuto, MalformedValueRejectedIp6OpeningSquareBracketMissing) {
    assertMalformedHostValueRejected("::1]");
}

TEST(SetHostAuto, MalformedValueRejectedIp6SquareBracketsMissing) {
    assertMalformedHostValueRejected("::1");
}

TEST(SetHostAuto, MalformedValueRejectedIp6Empty) {
    assertMalformedHostValueRejected("[]");
}

TEST(SetHostAuto, MalformedValueRejectedIpFutureClosingSquareBracketMissing) {
    assertMalformedHostValueRejected("[v7.host");
}

TEST(SetHostAuto, MalformedValueRejectedRegNameForbiddenCharacters) {
    assertMalformedHostValueRejected("not well-formed");
}
