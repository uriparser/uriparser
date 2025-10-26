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

static void testIsWellFormedPath(const char * candidate, bool hasHost,
                                 bool expectedWellFormed) {
    const char * const first = candidate;
    const char * const afterLast =
        (candidate == NULL) ? NULL : (candidate + strlen(candidate));

    const UriBool actualWellFormed = uriIsWellFormedPathA(first, afterLast, hasHost);

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

TEST(IsWellFormedPath, Null) {
    const bool hasHostValues[] = {true, false};
    for (size_t i = 0; i < sizeof(hasHostValues) / sizeof(hasHostValues[0]); i++) {
        const UriBool hasHost = hasHostValues[i] ? URI_TRUE : URI_FALSE;
        testIsWellFormedPath(NULL, hasHost, false);
    }
}

TEST(IsWellFormedPath, Empty) {
    testIsWellFormedPath("", /* hasHost=*/true, false);
    testIsWellFormedPath("", /* hasHost=*/false, true);
}

TEST(IsWellFormedPath, NonEmptyWithoutLeadingSlash) {
    testIsWellFormedPath("no-leading-slash", /* hasHost=*/true, false);
    testIsWellFormedPath("no-leading-slash", /* hasHost=*/false, true);
}

TEST(IsWellFormedPath, NonEmptySingleSlash) {
    const bool hasHostValues[] = {true, false};
    for (size_t i = 0; i < sizeof(hasHostValues) / sizeof(hasHostValues[0]); i++) {
        const UriBool hasHost = hasHostValues[i] ? URI_TRUE : URI_FALSE;
        testIsWellFormedPath("/", hasHost, true);
    }
}

TEST(IsWellFormedPath, NonEmptyTwoSlashes) {
    const bool hasHostValues[] = {true, false};
    for (size_t i = 0; i < sizeof(hasHostValues) / sizeof(hasHostValues[0]); i++) {
        const UriBool hasHost = hasHostValues[i] ? URI_TRUE : URI_FALSE;
        testIsWellFormedPath("//", hasHost, true);
    }
}

TEST(IsWellFormedPath, AllowedCharacters) {
    // The (simplified) related grammar subset is this:
    //
    //   path = *( unreserved / pct-encoded / sub-delims / ":" / "@" / "/" )
    //
    // NOTE: Percent encoding has dedicated tests further down
    const bool hasHostValues[] = {true, false};
    for (size_t i = 0; i < sizeof(hasHostValues) / sizeof(hasHostValues[0]); i++) {
        const UriBool hasHost = hasHostValues[i] ? URI_TRUE : URI_FALSE;
        testIsWellFormedPath("/"
                             "0123456789"
                             "ABCDEF"
                             "abcdef"
                             "gGhHiIjJkKlLmMnNoOpPqQrRsStTuUvVwWxXyYzZ"
                             "-._~"
                             "!$&'()*+,;="
                             ":@",
                             hasHost, true);
    }
}

TEST(IsWellFormedPath, ForbiddenCharacters) {
    const bool hasHostValues[] = {true, false};
    for (size_t i = 0; i < sizeof(hasHostValues) / sizeof(hasHostValues[0]); i++) {
        const UriBool hasHost = hasHostValues[i] ? URI_TRUE : URI_FALSE;
        testIsWellFormedPath("/ ", hasHost, false);
        testIsWellFormedPath("/?", hasHost, false);
        testIsWellFormedPath("/#", hasHost, false);
    }
}

TEST(IsWellFormedPath, PercentEncodingWellFormed) {
    const bool hasHostValues[] = {true, false};
    for (size_t i = 0; i < sizeof(hasHostValues) / sizeof(hasHostValues[0]); i++) {
        const UriBool hasHost = hasHostValues[i] ? URI_TRUE : URI_FALSE;
        testIsWellFormedPath("/"
                             "%"
                             "aa"
                             "%"
                             "AA",
                             hasHost, true);
    }
}

TEST(IsWellFormedPath, PercentEncodingMalformedCutOff1) {
    const bool hasHostValues[] = {true, false};
    for (size_t i = 0; i < sizeof(hasHostValues) / sizeof(hasHostValues[0]); i++) {
        const UriBool hasHost = hasHostValues[i] ? URI_TRUE : URI_FALSE;
        testIsWellFormedPath("/"
                             "%",
                             hasHost, false);
    }
}

TEST(IsWellFormedPath, PercentEncodingMalformedCutOff2) {
    const bool hasHostValues[] = {true, false};
    for (size_t i = 0; i < sizeof(hasHostValues) / sizeof(hasHostValues[0]); i++) {
        const UriBool hasHost = hasHostValues[i] ? URI_TRUE : URI_FALSE;
        testIsWellFormedPath("/"
                             "%"
                             "a",
                             hasHost, false);
    }
}

TEST(IsWellFormedPath, PercentEncodingMalformedForbiddenCharacter1) {
    const bool hasHostValues[] = {true, false};
    for (size_t i = 0; i < sizeof(hasHostValues) / sizeof(hasHostValues[0]); i++) {
        const UriBool hasHost = hasHostValues[i] ? URI_TRUE : URI_FALSE;
        testIsWellFormedPath("/"
                             "%"
                             "ga",
                             hasHost, false);
    }
}

TEST(IsWellFormedPath, PercentEncodingMalformedForbiddenCharacter2) {
    const bool hasHostValues[] = {true, false};
    for (size_t i = 0; i < sizeof(hasHostValues) / sizeof(hasHostValues[0]); i++) {
        const UriBool hasHost = hasHostValues[i] ? URI_TRUE : URI_FALSE;
        testIsWellFormedPath("/"
                             "%"
                             "ag",
                             hasHost, false);
    }
}

TEST(SetPath, NullUriOnly) {
    UriUriA * const uri = NULL;
    const char * const first = "path1/path2";
    const char * const afterLast = first + strlen(first);
    ASSERT_EQ(uriSetPathA(uri, first, afterLast), URI_ERROR_NULL);
}

TEST(SetPath, NullFirstOnly) {
    UriUriA uri = {};
    const char * const path = "path1/path2";
    const char * const first = NULL;
    const char * const afterLast = path + strlen(path);
    ASSERT_EQ(uriSetPathA(&uri, first, afterLast), URI_ERROR_NULL);
}

TEST(SetPath, NullAfterLastOnly) {
    UriUriA uri = {};
    const char * const first = "path1/path2";
    const char * const afterLast = NULL;
    ASSERT_EQ(uriSetPathA(&uri, first, afterLast), URI_ERROR_NULL);
}

TEST(SetPath, NullValueLeavesOwnerAtFalse) {
    UriUriA uri = parseWellFormedUri("/path");
    EXPECT_EQ(uri.owner, URI_FALSE);  // self-test

    EXPECT_EQ(uriSetPathA(&uri, NULL, NULL), URI_SUCCESS);

    EXPECT_EQ(uri.owner, URI_FALSE);  // i.e. still false

    uriFreeUriMembersA(&uri);
}

TEST(SetPath, NonNullValueMakesOwner) {
    UriUriA uri = parseWellFormedUri("//host/old");
    const char * const first = "/new";
    const char * const afterLast = first + strlen(first);
    EXPECT_EQ(uri.owner, URI_FALSE);  // self-test

    EXPECT_EQ(uriSetPathA(&uri, first, afterLast), URI_SUCCESS);

    EXPECT_EQ(uri.owner, URI_TRUE);  // i.e. now owned

    uriFreeUriMembersA(&uri);
}

TEST(SetPath, NullValueAppliedWithHost) {
    UriUriA uri = parseWellFormedUri("//host/path");

    EXPECT_EQ(uriSetPathA(&uri, NULL, NULL), URI_SUCCESS);

    assertUriEqual(&uri, "//host");

    uriFreeUriMembersA(&uri);
}

TEST(SetPath, NullValueAppliedWithoutHost) {
    UriUriA uri = parseWellFormedUri("scheme:/path");

    EXPECT_EQ(uriSetPathA(&uri, NULL, NULL), URI_SUCCESS);

    assertUriEqual(&uri, "scheme:");

    uriFreeUriMembersA(&uri);
}

TEST(SetPath, NonNullValueAppliedSingleSlashWithHost) {
    UriUriA uri = parseWellFormedUri("//host/path");
    const char * const first = "/";
    const char * const afterLast = first + strlen(first);

    EXPECT_EQ(uriSetPathA(&uri, first, afterLast), URI_SUCCESS);

    assertUriEqual(&uri, "//host/");
    EXPECT_EQ(uri.absolutePath, URI_FALSE);  // always false for URIs with host

    uriFreeUriMembersA(&uri);
}

TEST(SetPath, NonNullValueAppliedSingleSlashWithoutHost) {
    UriUriA uri = parseWellFormedUri("scheme:path");
    const char * const first = "/";
    const char * const afterLast = first + strlen(first);

    EXPECT_EQ(uriSetPathA(&uri, first, afterLast), URI_SUCCESS);

    assertUriEqual(&uri, "scheme:/");
    EXPECT_EQ(uri.absolutePath, URI_TRUE);

    uriFreeUriMembersA(&uri);
}

TEST(SetPath, NonNullValueAppliedTwoSlashesWithHost) {
    UriUriA uri = parseWellFormedUri("//host/path");
    const char * const first = "//";
    const char * const afterLast = first + strlen(first);

    EXPECT_EQ(uriSetPathA(&uri, first, afterLast), URI_SUCCESS);

    assertUriEqual(&uri, "//host//");
    EXPECT_EQ(uri.absolutePath, URI_FALSE);  // always false for URIs with host

    uriFreeUriMembersA(&uri);
}

TEST(SetPath, NonNullValueAppliedTwoSlashesWithoutHostDotInserted) {
    UriUriA uri = parseWellFormedUri("scheme:path");
    const char * const first = "//";
    const char * const afterLast = first + strlen(first);

    EXPECT_EQ(uriSetPathA(&uri, first, afterLast), URI_SUCCESS);

    assertUriEqual(&uri, "scheme:/.//");  // i.e. not scheme://
    EXPECT_EQ(uri.absolutePath, URI_TRUE);

    uriFreeUriMembersA(&uri);
}

TEST(SetPath, NonNullValueAppliedThreeSlashesWithHost) {
    UriUriA uri = parseWellFormedUri("//host/path");
    const char * const first = "///";
    const char * const afterLast = first + strlen(first);

    EXPECT_EQ(uriSetPathA(&uri, first, afterLast), URI_SUCCESS);

    assertUriEqual(&uri, "//host///");
    EXPECT_EQ(uri.absolutePath, URI_FALSE);  // always false for URIs with host

    uriFreeUriMembersA(&uri);
}

TEST(SetPath, NonNullValueAppliedThreeSlashesWithoutHostDotInserted) {
    UriUriA uri = parseWellFormedUri("scheme:path");
    const char * const first = "///";
    const char * const afterLast = first + strlen(first);

    EXPECT_EQ(uriSetPathA(&uri, first, afterLast), URI_SUCCESS);

    assertUriEqual(&uri, "scheme:/.///");  // i.e. not scheme:///
    EXPECT_EQ(uri.absolutePath, URI_TRUE);

    uriFreeUriMembersA(&uri);
}

TEST(SetPath, NonNullValueAppliedEmptyWithHost) {
    UriUriA uri = parseWellFormedUri("//host/path");
    const char * const empty = "";

    EXPECT_EQ(uriSetPathA(&uri, empty, empty), URI_ERROR_SYNTAX);

    uriFreeUriMembersA(&uri);
}

TEST(SetPath, NonNullValueAppliedEmptyWithoutHost) {
    UriUriA uri = parseWellFormedUri("scheme:path");
    const char * const empty = "";

    EXPECT_EQ(uriSetPathA(&uri, empty, empty), URI_SUCCESS);

    assertUriEqual(&uri, "scheme:");
    EXPECT_TRUE(uri.pathHead == NULL);
    EXPECT_TRUE(uri.pathTail == NULL);

    uriFreeUriMembersA(&uri);
}

TEST(SetPath, NonNullValueAppliedNonEmptyWithEmptyHost) {
    UriUriA uri = parseWellFormedUri("file:///old1/old2");
    const char * const first = "/new1/new2";
    const char * const afterLast = first + strlen(first);

    EXPECT_EQ(uriSetPathA(&uri, first, afterLast), URI_SUCCESS);

    assertUriEqual(&uri, "file:///new1/new2");
    EXPECT_EQ(uri.absolutePath, URI_FALSE);  // always false for URIs with host

    uriFreeUriMembersA(&uri);
}

TEST(SetPath, NonNullValueAppliedNonEmptyWithNonEmptyHost) {
    UriUriA uri = parseWellFormedUri("//host/old1/old2");
    const char * const first = "/new1/new2";
    const char * const afterLast = first + strlen(first);

    EXPECT_EQ(uriSetPathA(&uri, first, afterLast), URI_SUCCESS);

    assertUriEqual(&uri, "//host/new1/new2");
    EXPECT_EQ(uri.absolutePath, URI_FALSE);  // always false for URIs with host

    uriFreeUriMembersA(&uri);
}

TEST(SetPath, NonNullValueAppliedNonEmptyWithoutHostRel) {
    UriUriA uri = parseWellFormedUri("/old1/old2");
    const char * const first = "new1/new2";
    const char * const afterLast = first + strlen(first);

    EXPECT_EQ(uriSetPathA(&uri, first, afterLast), URI_SUCCESS);

    assertUriEqual(&uri, "new1/new2");

    uriFreeUriMembersA(&uri);
}

TEST(SetPath, NonNullValueAppliedNonEmptyWithoutHostRelDotInserted) {
    UriUriA uri = parseWellFormedUri("/old1/old2");
    const char * const first = "path1:/path2";
    const char * const afterLast = first + strlen(first);

    EXPECT_EQ(uriSetPathA(&uri, first, afterLast), URI_SUCCESS);

    assertUriEqual(&uri, "./path1:/path2");  // i.e. not path1:/path2

    uriFreeUriMembersA(&uri);
}

TEST(SetPath, NonNullValueAppliedNonEmptyWithoutHostAbs) {
    UriUriA uri = parseWellFormedUri("old1/old2");
    const char * const first = "/new1/new2";
    const char * const afterLast = first + strlen(first);

    EXPECT_EQ(uriSetPathA(&uri, first, afterLast), URI_SUCCESS);

    assertUriEqual(&uri, "/new1/new2");
    EXPECT_EQ(uri.absolutePath, URI_TRUE);

    uriFreeUriMembersA(&uri);
}

TEST(SetPath, NonNullValueAppliedNonEmptyWithoutHostAbsDotInserted) {
    UriUriA uri = parseWellFormedUri("old1/old2");
    const char * const first = "//path1/path2";
    const char * const afterLast = first + strlen(first);

    EXPECT_EQ(uriSetPathA(&uri, first, afterLast), URI_SUCCESS);

    assertUriEqual(&uri, "/.//path1/path2");  // i.e. not //path1/path2
    EXPECT_EQ(uri.absolutePath, URI_TRUE);

    uriFreeUriMembersA(&uri);
}

TEST(SetPath, NonNullValueAppliedNonEmptyWithoutHostWithScheme) {
    UriUriA uri = parseWellFormedUri("scheme:");
    const char * const first = "path1:/path2/path3";
    const char * const afterLast = first + strlen(first);

    EXPECT_EQ(uriSetPathA(&uri, first, afterLast), URI_SUCCESS);

    assertUriEqual(&uri, "scheme:path1:/path2/path3");

    uriFreeUriMembersA(&uri);
}

TEST(SetPath, MalformedValueRejected) {
    UriUriA uri = parseWellFormedUri("/path");
    const char * const first = "not well-formed";
    const char * const afterLast = first + strlen(first);

    EXPECT_EQ(uriSetPathA(&uri, first, afterLast), URI_ERROR_SYNTAX);

    uriFreeUriMembersA(&uri);
}
