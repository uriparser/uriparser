/*
 * uriparser - RFC 3986 URI parsing library
 *
 * Copyright (C) 2007, Weijia Song <songweijia@gmail.com>
 * Copyright (C) 2007, Sebastian Pipping <sebastian@pipping.org>
 * Copyright (C) 2025, Máté Kocsis <kocsismate@php.net>
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

#include <uriparser/Uri.h>
#include <uriparser/UriIp4.h>
#include <gtest/gtest.h>
#include <memory>
#include <cstdio>
#include <cstdlib>
#include <cwchar>
#include <string>

using namespace std;

namespace {
void testCopyUri(UriUriA * destUri, const char * const sourceUriString) {
    UriUriA sourceUri;

    ASSERT_EQ(uriParseSingleUriA(&sourceUri, sourceUriString, NULL), URI_SUCCESS);

    ASSERT_EQ(URI_SUCCESS, uriCopyUriA(destUri, &sourceUri));
    ASSERT_EQ(URI_TRUE, uriEqualsUriA(destUri, &sourceUri));

    uriFreeUriMembersA(&sourceUri);
}
}  // namespace

TEST(CopyUriSuite, ErrorSourceUriNull) {
    UriUriA destUri;

    ASSERT_EQ(uriCopyUriA(&destUri, NULL), URI_ERROR_NULL);
}

TEST(CopyUriSuite, ErrorDestUriNull) {
    UriUriA sourceUri;
    const char * const sourceUriString = "https://example.com";

    ASSERT_EQ(uriParseSingleUriA(&sourceUri, sourceUriString, NULL), URI_SUCCESS);

    ASSERT_EQ(URI_ERROR_NULL, uriCopyUriA(NULL, &sourceUri));

    uriFreeUriMembersA(&sourceUri);
}

TEST(CopyUriSuite, ErrorIncompleteMemoryManager) {
    UriUriA sourceUri;
    const char * const sourceUriString = "https://example.com";
    UriMemoryManager memory = {NULL, NULL, NULL, NULL, NULL, NULL};

    ASSERT_EQ(uriParseSingleUriA(&sourceUri, sourceUriString, NULL), URI_SUCCESS);

    UriUriA destUri;
    ASSERT_EQ(URI_ERROR_MEMORY_MANAGER_INCOMPLETE,
              uriCopyUriMmA(&destUri, &sourceUri, &memory));

    uriFreeUriMembersA(&sourceUri);
}

TEST(CopyUriSuite, SuccessRegName) {
    UriUriA destUri;
    testCopyUri(&destUri, "https://somehost.com");

    ASSERT_TRUE(destUri.hostData.ip4 == NULL);
    ASSERT_TRUE(destUri.hostData.ip6 == NULL);
    ASSERT_TRUE(destUri.hostData.ipFuture.first == NULL);
    ASSERT_TRUE(destUri.hostData.ipFuture.afterLast == NULL);
    ASSERT_EQ(0, strncmp(destUri.hostText.first, "somehost.com", strlen("somehost.com")));

    uriFreeUriMembersA(&destUri);
}

TEST(CopyUriSuite, SuccessCompleteUri) {
    UriUriA destUri;
    testCopyUri(&destUri, "https://user:pass@somehost.com:80/path?query#frag");

    uriFreeUriMembersA(&destUri);
}

TEST(CopyUriSuite, SuccessRelativeReference) {
    UriUriA destUri;
    testCopyUri(&destUri, "/foo/bar/baz");

    uriFreeUriMembersA(&destUri);
}

TEST(CopyUriSuite, SuccessEmail) {
    UriUriA destUri;
    testCopyUri(&destUri, "mailto:fred@example.com");

    uriFreeUriMembersA(&destUri);
}

TEST(CopyUriSuite, SuccessIpV4) {
    UriUriA destUri;
    testCopyUri(&destUri, "http://192.168.0.1/");

    const unsigned char expected[4] = {192, 168, 0, 1};
    ASSERT_EQ(memcmp(expected, destUri.hostData.ip4->data, sizeof(expected)), 0);

    ASSERT_TRUE(destUri.hostData.ip6 == NULL);
    ASSERT_TRUE(destUri.hostData.ipFuture.first == NULL);
    ASSERT_TRUE(destUri.hostData.ipFuture.afterLast == NULL);
    ASSERT_EQ(0, strncmp(destUri.hostText.first, "192.168.0.1", strlen("192.168.0.1")));

    uriFreeUriMembersA(&destUri);
}

TEST(CopyUriSuite, SuccessIpV6) {
    UriUriA destUri;
    testCopyUri(&destUri,
                "https://[2001:0db8:0001:0000:0000:0ab9:c0a8:0102]");  // RFC 3849

    const unsigned char expected[16] = {0x20, 0x01, 0x0d, 0xb8, 0x00, 0x01, 0x00, 0x00,
                                        0x00, 0x00, 0x0a, 0xb9, 0xc0, 0xa8, 0x01, 0x02};
    ASSERT_EQ(memcmp(expected, destUri.hostData.ip6->data, sizeof(expected)), 0);

    ASSERT_TRUE(destUri.hostData.ip4 == NULL);
    ASSERT_TRUE(destUri.hostData.ipFuture.first == NULL);
    ASSERT_TRUE(destUri.hostData.ipFuture.afterLast == NULL);
    ASSERT_EQ(0,
              strncmp(destUri.hostText.first, "2001:0db8:0001:0000:0000:0ab9:c0a8:0102",
                      strlen("2001:0db8:0001:0000:0000:0ab9:c0a8:0102")));

    uriFreeUriMembersA(&destUri);
}

TEST(CopyUriSuite, SuccessIpFuture) {
    UriUriA destUri;
    testCopyUri(&destUri, "//[v7.host]/source");

    ASSERT_EQ(0, strncmp(destUri.hostData.ipFuture.first, "v7.host", strlen("v7.host")));

    ASSERT_TRUE(destUri.hostData.ip4 == NULL);
    ASSERT_TRUE(destUri.hostData.ip6 == NULL);
    ASSERT_TRUE(destUri.hostText.first == destUri.hostData.ipFuture.first);
    ASSERT_TRUE(destUri.hostText.afterLast == destUri.hostData.ipFuture.afterLast);

    uriFreeUriMembersA(&destUri);
}

TEST(CopyUriSuite, SuccessEmptyPort) {
    UriUriA destUri;
    testCopyUri(&destUri, "http://example.com:/");

    ASSERT_TRUE(destUri.portText.first != NULL);
    ASSERT_TRUE(destUri.portText.afterLast != NULL);
    ASSERT_EQ(destUri.portText.first, destUri.portText.afterLast);

    uriFreeUriMembersA(&destUri);
}

TEST(CopyUriSuite, SuccessEmptyUserInfo) {
    UriUriA destUri;
    testCopyUri(&destUri, "http://@example.com/");

    ASSERT_TRUE(destUri.userInfo.first != NULL);
    ASSERT_TRUE(destUri.userInfo.afterLast != NULL);
    ASSERT_EQ(destUri.userInfo.first, destUri.userInfo.afterLast);

    uriFreeUriMembersA(&destUri);
}

TEST(CopyUriSuite, SuccessEmptyQuery) {
    UriUriA destUri;
    testCopyUri(&destUri, "http://example.com/?");

    ASSERT_TRUE(destUri.query.first != NULL);
    ASSERT_TRUE(destUri.query.afterLast != NULL);
    ASSERT_EQ(destUri.query.first, destUri.query.afterLast);

    uriFreeUriMembersA(&destUri);
}

TEST(CopyUriSuite, SuccessEmptyFragment) {
    UriUriA destUri;
    testCopyUri(&destUri, "http://example.com/#");

    ASSERT_TRUE(destUri.fragment.first != NULL);
    ASSERT_TRUE(destUri.fragment.afterLast != NULL);
    ASSERT_EQ(destUri.fragment.first, destUri.fragment.afterLast);

    uriFreeUriMembersA(&destUri);
}
