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
	void testSetScheme(const char *const input, const char *const value, const char *const expected) {
		UriUriA uri;

		ASSERT_EQ(uriParseSingleUriA(&uri, input, NULL), URI_SUCCESS);
		ASSERT_EQ(URI_SUCCESS, uriSetSchemeA(&uri, value, value ? value + strlen(value) : NULL));

		int size = 0;
		ASSERT_EQ(uriToStringCharsRequiredA(&uri, &size), URI_SUCCESS);
		char * const buffer = (char *)malloc(size + 1);
		ASSERT_TRUE(buffer);
		ASSERT_EQ(uriToStringA(buffer, &uri, size + 1, &size), URI_SUCCESS);
		if (strcmp(buffer, expected)) {
			printf("Expected \"%s\" but got \"%s\"\n", expected, buffer);
			ASSERT_TRUE(0);
		}
		free(buffer);

		uriFreeUriMembersA(&uri);
	}
} // namespace

TEST(SetSchemeSuite, SuccessChange) {
	testSetScheme(
		"http://example.com",
		"https",
		"https://example.com"
	);
}

TEST(SetSchemeSuite, SuccessRemoval) {
	testSetScheme(
		"http://example.com",
		NULL,
		"//example.com"
	);
}

TEST(SetSchemeSuite, SuccessAddition) {
	testSetScheme(
		"/test",
		"https",
		"https:/test"
	);
}

TEST(SetSchemeSuite, SuccessSpecialCharacters) {
	testSetScheme(
		"http://example.com",
		"git+ssh",
		"git+ssh://example.com"
	);
}
