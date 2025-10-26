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
    const char * const afterLast =
        (candidate == NULL) ? NULL : (candidate + strlen(candidate));

    const UriBool actualWellFormed = uriIsWellFormedHostIp4A(first, afterLast);

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

static void assertUriHostIp4Equal(const UriUriA * uri, unsigned char o1, unsigned char o2,
                                  unsigned char o3, unsigned char o4) {
    ASSERT_TRUE(uri->hostData.ip4 != NULL);
    EXPECT_EQ(uri->hostData.ip4->data[0], o1);
    EXPECT_EQ(uri->hostData.ip4->data[1], o2);
    EXPECT_EQ(uri->hostData.ip4->data[2], o3);
    EXPECT_EQ(uri->hostData.ip4->data[3], o4);
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

TEST(SetHostIp4, NullUriOnly) {
    UriUriA * const uri = NULL;
    const char * const first = "1.2.3.4";
    const char * const afterLast = first + strlen(first);
    ASSERT_EQ(uriSetHostIp4A(uri, first, afterLast), URI_ERROR_NULL);
}

TEST(SetHostIp4, NullFirstOnly) {
    UriUriA uri = {};
    const char * const host = "1.2.3.4";
    const char * const first = NULL;
    const char * const afterLast = host + strlen(host);
    ASSERT_EQ(uriSetHostIp4A(&uri, first, afterLast), URI_ERROR_NULL);
}

TEST(SetHostIp4, NullAfterLastOnly) {
    UriUriA uri = {};
    const char * const first = "1.2.3.4";
    const char * const afterLast = NULL;
    ASSERT_EQ(uriSetHostIp4A(&uri, first, afterLast), URI_ERROR_NULL);
}

TEST(SetHostIp4, NullValueLeavesOwnerAtFalse) {
    UriUriA uri = parseWellFormedUri("scheme://host/");
    EXPECT_EQ(uri.owner, URI_FALSE);  // self-test

    EXPECT_EQ(uriSetHostIp4A(&uri, NULL, NULL), URI_SUCCESS);

    EXPECT_EQ(uri.owner, URI_FALSE);  // i.e. still false

    uriFreeUriMembersA(&uri);
}

TEST(SetHostIp4, NonNullValueMakesOwner) {
    UriUriA uri = parseWellFormedUri("scheme://old/");
    const char * const first = "1.2.3.4";
    const char * const afterLast = first + strlen(first);
    EXPECT_EQ(uri.owner, URI_FALSE);  // self-test

    EXPECT_EQ(uriSetHostIp4A(&uri, first, afterLast), URI_SUCCESS);

    EXPECT_EQ(uri.owner, URI_TRUE);  // i.e. now owned

    uriFreeUriMembersA(&uri);
}

TEST(SetHostIp4, NullValueAppliedDotInserted) {
    UriUriA uri = parseWellFormedUri("scheme://host//path1/path2");

    EXPECT_EQ(uriSetHostIp4A(&uri, NULL, NULL), URI_SUCCESS);

    assertUriEqual(&uri, "scheme:/.//path1/path2");  // i.e. not scheme://path1/path2

    uriFreeUriMembersA(&uri);
}

TEST(SetHostIp4, NullValueAppliedDotNotInserted) {
    UriUriA uri = parseWellFormedUri("//host/./path1/path2");

    EXPECT_EQ(uriSetHostIp4A(&uri, NULL, NULL), URI_SUCCESS);

    assertUriEqual(&uri, "/./path1/path2");  // i.e. not /././path1/path2

    uriFreeUriMembersA(&uri);
}

TEST(SetHostIp4, NullValueAppliedPriorNull) {
    UriUriA uri = parseWellFormedUri("scheme:/path");

    EXPECT_EQ(uriSetHostIp4A(&uri, NULL, NULL), URI_SUCCESS);

    assertUriEqual(&uri, "scheme:/path");

    uriFreeUriMembersA(&uri);
}

TEST(SetHostIp4, NullValueAppliedPriorIp4) {
    UriUriA uri = parseWellFormedUri("scheme://1.2.3.4/path");

    EXPECT_EQ(uriSetHostIp4A(&uri, NULL, NULL), URI_SUCCESS);

    assertUriEqual(&uri, "scheme:/path");

    uriFreeUriMembersA(&uri);
}

TEST(SetHostIp4, NullValueAppliedPriorIp6) {
    UriUriA uri = parseWellFormedUri("scheme://[::1]/path");

    EXPECT_EQ(uriSetHostIp4A(&uri, NULL, NULL), URI_SUCCESS);

    assertUriEqual(&uri, "scheme:/path");

    uriFreeUriMembersA(&uri);
}

TEST(SetHostIp4, NullValueAppliedPriorIpFuture) {
    UriUriA uri = parseWellFormedUri("scheme://[v7.host]/path");

    EXPECT_EQ(uriSetHostIp4A(&uri, NULL, NULL), URI_SUCCESS);

    assertUriEqual(&uri, "scheme:/path");

    uriFreeUriMembersA(&uri);
}

TEST(SetHostIp4, NullValueAppliedPriorRegName) {
    UriUriA uri = parseWellFormedUri("scheme://host/path");

    EXPECT_EQ(uriSetHostIp4A(&uri, NULL, NULL), URI_SUCCESS);

    assertUriEqual(&uri, "scheme:/path");

    uriFreeUriMembersA(&uri);
}

TEST(SetHostIp4, NonNullValueAppliedNonEmptyPriorNull) {
    UriUriA uri = parseWellFormedUri("scheme:");
    const char * const first = "1.2.3.4";
    const char * const afterLast = first + strlen(first);

    EXPECT_EQ(uriSetHostIp4A(&uri, first, afterLast), URI_SUCCESS);

    assertUriEqual(&uri, "scheme://1.2.3.4");
    assertUriHostIp4Equal(&uri, 1, 2, 3, 4);

    uriFreeUriMembersA(&uri);
}

TEST(SetHostIp4, NonNullValueAppliedNonEmptyPriorIp4) {
    UriUriA uri = parseWellFormedUri("//1.2.3.4");
    const char * const first = "5.6.7.8";
    const char * const afterLast = first + strlen(first);

    EXPECT_EQ(uriSetHostIp4A(&uri, first, afterLast), URI_SUCCESS);

    assertUriEqual(&uri, "//5.6.7.8");
    assertUriHostIp4Equal(&uri, 5, 6, 7, 8);

    uriFreeUriMembersA(&uri);
}

TEST(SetHostIp4, NonNullValueAppliedNonEmptyPriorIp6) {
    UriUriA uri = parseWellFormedUri("//[::1]");
    const char * const first = "1.2.3.4";
    const char * const afterLast = first + strlen(first);

    EXPECT_EQ(uriSetHostIp4A(&uri, first, afterLast), URI_SUCCESS);

    assertUriEqual(&uri, "//1.2.3.4");
    assertUriHostIp4Equal(&uri, 1, 2, 3, 4);

    uriFreeUriMembersA(&uri);
}

TEST(SetHostIp4, NonNullValueAppliedNonEmptyPriorIpFuture) {
    UriUriA uri = parseWellFormedUri("//[v7.host]");
    const char * const first = "1.2.3.4";
    const char * const afterLast = first + strlen(first);

    EXPECT_EQ(uriSetHostIp4A(&uri, first, afterLast), URI_SUCCESS);

    assertUriEqual(&uri, "//1.2.3.4");
    assertUriHostIp4Equal(&uri, 1, 2, 3, 4);

    uriFreeUriMembersA(&uri);
}

TEST(SetHostIp4, NonNullValueAppliedNonEmptyPriorRegName) {
    UriUriA uri = parseWellFormedUri("//hostname.test");
    const char * const first = "1.2.3.4";
    const char * const afterLast = first + strlen(first);

    EXPECT_EQ(uriSetHostIp4A(&uri, first, afterLast), URI_SUCCESS);

    assertUriEqual(&uri, "//1.2.3.4");
    assertUriHostIp4Equal(&uri, 1, 2, 3, 4);

    uriFreeUriMembersA(&uri);
}

TEST(SetHostIp4, MalformedValueRejected) {
    UriUriA uri = parseWellFormedUri("scheme://host/");
    const char * const first = "not well-formed";
    const char * const afterLast = first + strlen(first);

    EXPECT_EQ(uriSetHostIp4A(&uri, first, afterLast), URI_ERROR_SYNTAX);

    uriFreeUriMembersA(&uri);
}

TEST(SetHostIp4, UriWithPortRejected) {
    UriUriA uri = parseWellFormedUri("//host:1234");
    EXPECT_TRUE(uri.portText.first != NULL);  // self-test

    EXPECT_EQ(uriSetHostIp4A(&uri, NULL, NULL), URI_ERROR_SETHOST_PORT_SET);

    uriFreeUriMembersA(&uri);
}

TEST(SetHostIp4, UriWithUserInfoRejected) {
    UriUriA uri = parseWellFormedUri("//user:password@host");
    EXPECT_TRUE(uri.userInfo.first != NULL);  // self-test

    EXPECT_EQ(uriSetHostIp4A(&uri, NULL, NULL), URI_ERROR_SETHOST_USERINFO_SET);

    uriFreeUriMembersA(&uri);
}
