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
	void testCopyUri(UriUriA * destUri, const char *const sourceUriString) {
		UriUriA sourceUri;

		int result = uriParseSingleUriExA(&sourceUri, sourceUriString, sourceUriString + strlen(sourceUriString), NULL);
		ASSERT_EQ(URI_SUCCESS, result);

		ASSERT_EQ(URI_SUCCESS, uriCopyUriA(destUri, &sourceUri));
		ASSERT_EQ(URI_TRUE, uriEqualsUriA(destUri, &sourceUri));

		uriFreeUriMembersA(&sourceUri);
	}
} // namespace

TEST(CopyUriSuite, ErrorUriNull) {
	UriUriA destUri;

	ASSERT_EQ(uriCopyUriA(&destUri, NULL), URI_ERROR_NULL);
}

TEST(CopyUriSuite, ErrorDestUriNull) {
	UriUriA sourceUri;
	const char * const sourceUriString = "https://example.com";

	int result = uriParseSingleUriExA(&sourceUri, sourceUriString, sourceUriString + strlen(sourceUriString), NULL);
	ASSERT_EQ(URI_SUCCESS, result);

	ASSERT_EQ(URI_ERROR_NULL, uriCopyUriA(NULL, &sourceUri));

	uriFreeUriMembersA(&sourceUri);
}

TEST(CopyUriSuite, ErrorIncompleteMemoryManager) {
	UriUriA sourceUri;
	const char * const sourceUriString = "https://example.com";
	UriMemoryManager memory = {NULL, NULL, NULL, NULL, NULL, NULL};

	int result = uriParseSingleUriExA(&sourceUri, sourceUriString, sourceUriString + strlen(sourceUriString), NULL);
	ASSERT_EQ(URI_SUCCESS, result);

	UriUriA destUri;
	ASSERT_EQ(URI_ERROR_MEMORY_MANAGER_INCOMPLETE, uriCopyUriMmA(&destUri, &sourceUri, &memory));

	uriFreeUriMembersA(&sourceUri);
}

TEST(CopyUriSuite, Success) {
	UriUriA destUri;
	testCopyUri(&destUri, "https://somehost.com");

	ASSERT_TRUE(destUri.hostData.ip4 == NULL);
	ASSERT_TRUE(destUri.hostData.ip6 == NULL);
	ASSERT_TRUE(destUri.hostData.ipFuture.first == NULL);
	ASSERT_TRUE(destUri.hostData.ipFuture.afterLast == NULL);

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

TEST(CopyUriSuite, SucessEmail) {
	UriUriA destUri;
	testCopyUri(&destUri, "mailto:fred@example.com");

	uriFreeUriMembersA(&destUri);
}

TEST(CopyUriSuite, SuccessIpV4) {
	UriUriA destUri;
	testCopyUri(&destUri, "http://192.168.0.1/");

	ASSERT_EQ(192, destUri.hostData.ip4->data[0]);
	ASSERT_EQ(168, destUri.hostData.ip4->data[1]);
	ASSERT_EQ(0, destUri.hostData.ip4->data[2]);
	ASSERT_EQ(1, destUri.hostData.ip4->data[3]);

	ASSERT_TRUE(destUri.hostData.ip6 == NULL);
	ASSERT_TRUE(destUri.hostData.ipFuture.first == NULL);
	ASSERT_TRUE(destUri.hostData.ipFuture.afterLast == NULL);

	uriFreeUriMembersA(&destUri);
}

TEST(CopyUriSuite, SuccessIpV6) {
	UriUriA destUri;
	testCopyUri(&destUri, "https://[2001:0db8:0001:0000:0000:0ab9:C0A8:0102]");

	ASSERT_EQ(32, destUri.hostData.ip6->data[0]);
	ASSERT_EQ(1, destUri.hostData.ip6->data[1]);
	ASSERT_EQ(13, destUri.hostData.ip6->data[2]);
	ASSERT_EQ(184, destUri.hostData.ip6->data[3]);
	ASSERT_EQ(0, destUri.hostData.ip6->data[4]);
	ASSERT_EQ(1, destUri.hostData.ip6->data[5]);
	ASSERT_EQ(0, destUri.hostData.ip6->data[6]);
	ASSERT_EQ(0, destUri.hostData.ip6->data[7]);
	ASSERT_EQ(0, destUri.hostData.ip6->data[8]);
	ASSERT_EQ(0, destUri.hostData.ip6->data[9]);
	ASSERT_EQ(10, destUri.hostData.ip6->data[10]);
	ASSERT_EQ(185, destUri.hostData.ip6->data[11]);
	ASSERT_EQ(192, destUri.hostData.ip6->data[12]);
	ASSERT_EQ(168, destUri.hostData.ip6->data[13]);
	ASSERT_EQ(1, destUri.hostData.ip6->data[14]);
	ASSERT_EQ(2, destUri.hostData.ip6->data[15]);

	ASSERT_TRUE(destUri.hostData.ip4 == NULL);
	ASSERT_TRUE(destUri.hostData.ipFuture.first == NULL);
	ASSERT_TRUE(destUri.hostData.ipFuture.afterLast == NULL);

	uriFreeUriMembersA(&destUri);
}

TEST(CopyUriSuite, SuccessIpFuture) {
	UriUriA destUri;
	testCopyUri(&destUri, "//[v7.host]/source");

	ASSERT_EQ(0, strncmp(destUri.hostData.ipFuture.first, "v7.host", 7));

	ASSERT_TRUE(destUri.hostData.ip4 == NULL);
	ASSERT_TRUE(destUri.hostData.ip6 == NULL);

	uriFreeUriMembersA(&destUri);
}
