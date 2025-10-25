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

static void testIsWellFormedPort(const char * candidate, bool expectedWellFormed) {
    const char * const first = candidate;
    const char * const afterLast =
        (candidate == NULL) ? NULL : (candidate + strlen(candidate));

    const UriBool actualWellFormed = uriIsWellFormedPortA(first, afterLast);

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

TEST(IsWellFormedPort, Null) {
    testIsWellFormedPort(NULL, false);
}

TEST(IsWellFormedPort, Empty) {
    testIsWellFormedPort("", true);
}

TEST(IsWellFormedPort, AllowedCharacters) {
    testIsWellFormedPort("0123456789", true);
}

TEST(IsWellFormedPort, ForbiddenCharacters) {
    testIsWellFormedPort(" ", false);
}

TEST(SetPortText, NullUriOnly) {
    UriUriA * const uri = NULL;
    const char * const first = "443";
    const char * const afterLast = first + strlen(first);
    ASSERT_EQ(uriSetPortTextA(uri, first, afterLast), URI_ERROR_NULL);
}

TEST(SetPortText, NullFirstOnly) {
    UriUriA uri = {};
    const char * const postText = "443";
    const char * const first = NULL;
    const char * const afterLast = postText + strlen(postText);
    ASSERT_EQ(uriSetPortTextA(&uri, first, afterLast), URI_ERROR_NULL);
}

TEST(SetPortText, NullAfterLastOnly) {
    UriUriA uri = {};
    const char * const first = "443";
    const char * const afterLast = NULL;
    ASSERT_EQ(uriSetPortTextA(&uri, first, afterLast), URI_ERROR_NULL);
}

TEST(SetPortText, NullValueLeavesOwnerAtFalse) {
    UriUriA uri = parseWellFormedUri("https://host:443/");
    EXPECT_EQ(uri.owner, URI_FALSE);  // self-test

    EXPECT_EQ(uriSetPortTextA(&uri, NULL, NULL), URI_SUCCESS);

    EXPECT_EQ(uri.owner, URI_FALSE);  // i.e. still false

    uriFreeUriMembersA(&uri);
}

TEST(SetPortText, NonNullValueMakesOwner) {
    UriUriA uri = parseWellFormedUri("https://host:443/");
    const char * const first = "50443";
    const char * const afterLast = first + strlen(first);
    EXPECT_EQ(uri.owner, URI_FALSE);  // self-test

    EXPECT_EQ(uriSetPortTextA(&uri, first, afterLast), URI_SUCCESS);

    EXPECT_EQ(uri.owner, URI_TRUE);  // i.e. now owned

    uriFreeUriMembersA(&uri);
}

TEST(SetPortText, NullValueApplied) {
    UriUriA uri = parseWellFormedUri("https://host:443/");

    EXPECT_EQ(uriSetPortTextA(&uri, NULL, NULL), URI_SUCCESS);

    assertUriEqual(&uri, "https://host/");

    uriFreeUriMembersA(&uri);
}

TEST(SetPortText, NonNullValueAppliedEmpty) {
    UriUriA uri = parseWellFormedUri("https://host:443/");
    const char * const empty = "";

    EXPECT_EQ(uriSetPortTextA(&uri, empty, empty), URI_SUCCESS);

    assertUriEqual(&uri, "https://host:/");

    uriFreeUriMembersA(&uri);
}

TEST(SetPortText, NonNullValueAppliedNonEmpty) {
    UriUriA uri = parseWellFormedUri("https://host:443/");
    const char * const first = "50443";
    const char * const afterLast = first + strlen(first);

    EXPECT_EQ(uriSetPortTextA(&uri, first, afterLast), URI_SUCCESS);

    assertUriEqual(&uri, "https://host:50443/");

    uriFreeUriMembersA(&uri);
}

TEST(SetPortText, MalformedValueRejected) {
    UriUriA uri = parseWellFormedUri("https://host:443/");
    const char * const first = "not well-formed";
    const char * const afterLast = first + strlen(first);

    EXPECT_EQ(uriSetPortTextA(&uri, first, afterLast), URI_ERROR_SYNTAX);

    uriFreeUriMembersA(&uri);
}

TEST(SetPortText, UriWithoutHostNullTolerated) {
    const char * originalText = "/no/host/here";
    UriUriA uri = parseWellFormedUri(originalText);

    EXPECT_EQ(uriSetPortTextA(&uri, NULL, NULL), URI_SUCCESS);

    assertUriEqual(&uri, originalText);

    uriFreeUriMembersA(&uri);
}

TEST(SetPortText, UriWithoutHostNonNullRejected) {
    UriUriA uri = parseWellFormedUri("/no/host/here");
    const char * const first = "443";
    const char * const afterLast = first + strlen(first);
    EXPECT_TRUE(uri.hostText.first == NULL);  // self-test

    EXPECT_EQ(uriSetPortTextA(&uri, first, afterLast), URI_ERROR_SETPORT_HOST_NOT_SET);

    uriFreeUriMembersA(&uri);
}
