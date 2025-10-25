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

static void testIsWellFormedScheme(const char * candidate, bool expectedWellFormed) {
    const char * const first = candidate;
    const char * const afterLast =
        (candidate == NULL) ? NULL : (candidate + strlen(candidate));

    const UriBool actualWellFormed = uriIsWellFormedSchemeA(first, afterLast);

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

TEST(IsWellFormedScheme, Null) {
    testIsWellFormedScheme(NULL, false);
}

TEST(IsWellFormedScheme, Empty) {
    testIsWellFormedScheme("", false);
}

TEST(IsWellFormedScheme, AllowedCharacters) {
    // The related grammar subset is this:
    //
    //   scheme      = ALPHA *( ALPHA / DIGIT / "+" / "-" / "." )
    //
    testIsWellFormedScheme("ABCDEF"
                           "abcdef"
                           "gGhHiIjJkKlLmMnNoOpPqQrRsStTuUvVwWxXyYzZ"
                           "0123456789"
                           "+-.",
                           true);
}

TEST(IsWellFormedScheme, ForbiddenCharacters) {
    testIsWellFormedScheme(" ", false);
}

TEST(SetScheme, NullUriOnly) {
    UriUriA * const uri = NULL;
    const char * const first = "ssh";
    const char * const afterLast = first + strlen(first);
    ASSERT_EQ(uriSetSchemeA(uri, first, afterLast), URI_ERROR_NULL);
}

TEST(SetScheme, NullFirstOnly) {
    UriUriA uri = {};
    const char * const scheme = "ssh";
    const char * const first = NULL;
    const char * const afterLast = scheme + strlen(scheme);
    ASSERT_EQ(uriSetSchemeA(&uri, first, afterLast), URI_ERROR_NULL);
}

TEST(SetScheme, NullAfterLastOnly) {
    UriUriA uri = {};
    const char * const first = "ssh";
    const char * const afterLast = NULL;
    ASSERT_EQ(uriSetSchemeA(&uri, first, afterLast), URI_ERROR_NULL);
}

TEST(SetScheme, NullValueLeavesOwnerAtFalse) {
    UriUriA uri = parseWellFormedUri("scheme://");
    EXPECT_EQ(uri.owner, URI_FALSE);  // self-test

    EXPECT_EQ(uriSetSchemeA(&uri, NULL, NULL), URI_SUCCESS);

    EXPECT_EQ(uri.owner, URI_FALSE);  // i.e. still false

    uriFreeUriMembersA(&uri);
}

TEST(SetScheme, NonNullValueMakesOwner) {
    UriUriA uri = parseWellFormedUri("//host/");
    const char * const first = "ssh";
    const char * const afterLast = first + strlen(first);
    EXPECT_EQ(uri.owner, URI_FALSE);  // self-test

    EXPECT_EQ(uriSetSchemeA(&uri, first, afterLast), URI_SUCCESS);

    EXPECT_EQ(uri.owner, URI_TRUE);  // i.e. now owned

    uriFreeUriMembersA(&uri);
}

TEST(SetScheme, NullValueAppliedHost) {
    UriUriA uri = parseWellFormedUri("ssh://host/");

    EXPECT_EQ(uriSetSchemeA(&uri, NULL, NULL), URI_SUCCESS);

    assertUriEqual(&uri, "//host/");

    uriFreeUriMembersA(&uri);
}

TEST(SetScheme, NullValueAppliedPathWithoutColon) {
    UriUriA uri = parseWellFormedUri("scheme:path1/path2/path3");

    EXPECT_EQ(uriSetSchemeA(&uri, NULL, NULL), URI_SUCCESS);

    assertUriEqual(&uri, "path1/path2/path3");

    uriFreeUriMembersA(&uri);
}

TEST(SetScheme, NullValueAppliedPathWithColonRelativeDotPrepended) {
    UriUriA uri = parseWellFormedUri("scheme:path1:/path2/path3");

    EXPECT_EQ(uriSetSchemeA(&uri, NULL, NULL), URI_SUCCESS);

    assertUriEqual(&uri, "./path1:/path2/path3");  // i.e. not path1:/path2/path3

    uriFreeUriMembersA(&uri);
}

TEST(SetScheme, NullValueAppliedPathWithColonRelativeDotNotPrepended) {
    UriUriA uri = parseWellFormedUri("scheme:path1/path2:/path3");

    EXPECT_EQ(uriSetSchemeA(&uri, NULL, NULL), URI_SUCCESS);

    assertUriEqual(&uri, "path1/path2:/path3");  // i.e. not ./path1/path2:/path3

    uriFreeUriMembersA(&uri);
}

TEST(SetScheme, NullValueAppliedPathWithColonAbsolute) {
    UriUriA uri = parseWellFormedUri("scheme:/path1:/path2/path3");

    EXPECT_EQ(uriSetSchemeA(&uri, NULL, NULL), URI_SUCCESS);

    assertUriEqual(&uri, "/path1:/path2/path3");

    uriFreeUriMembersA(&uri);
}

TEST(SetScheme, NullValueAppliedPathWithColonAndHost) {
    UriUriA uri = parseWellFormedUri("scheme://host/path1:/path2/path3");

    EXPECT_EQ(uriSetSchemeA(&uri, NULL, NULL), URI_SUCCESS);

    assertUriEqual(&uri, "//host/path1:/path2/path3");

    uriFreeUriMembersA(&uri);
}

TEST(SetScheme, NonNullValueApplied) {
    UriUriA uri = parseWellFormedUri("old://host/");
    const char * const first = "new";
    const char * const afterLast = first + strlen(first);

    EXPECT_EQ(uriSetSchemeA(&uri, first, afterLast), URI_SUCCESS);

    assertUriEqual(&uri, "new://host/");

    uriFreeUriMembersA(&uri);
}

TEST(SetScheme, MalformedValueRejected) {
    UriUriA uri = parseWellFormedUri("scheme://host/");
    const char * const first = "not well-formed";
    const char * const afterLast = first + strlen(first);

    EXPECT_EQ(uriSetSchemeA(&uri, first, afterLast), URI_ERROR_SYNTAX);

    uriFreeUriMembersA(&uri);
}

TEST(SetScheme, UriWithoutHostTolerated) {
    UriUriA uri = parseWellFormedUri("/no/host/here");
    const char * const first = "scheme";
    const char * const afterLast = first + strlen(first);
    EXPECT_TRUE(uri.hostText.first == NULL);  // self-test

    EXPECT_EQ(uriSetSchemeA(&uri, first, afterLast), URI_SUCCESS);

    assertUriEqual(&uri, "scheme:/no/host/here");

    uriFreeUriMembersA(&uri);
}
