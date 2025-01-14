// Copyright 2020 Google LLC
// Copyright 2025 Mikhail Khachaiants <mkhachaiants@gmail.com>
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "uriparser/Uri.h"
#include "uriparser/UriIp4.h"
#include "FuzzingUtils.h"
#include <cstddef>
#include <cstring>
#include <vector>



class UriHolder {
public:
	UriHolder() {
		memset((void *)&uri_, 0, sizeof(uri_));
	}

	~UriHolder() {
		URI_FUNC(FreeUriMembers)(&uri_);
	}

	URI_TYPE(Uri) * get() {
		return &uri_;
	}

private:
	URI_TYPE(Uri) uri_;
};



void Escapes(const UriString & uri) {
	const URI_CHAR * first = uri.c_str();
	// Up to 6 bytes per character with normalizeBreaks enabled (\n -> %0D%0A)
	std::vector<URI_CHAR> buf1(uri.size() * 6 + 1);
	// and up to 3 bytes per character otherwise
	std::vector<URI_CHAR> buf2(uri.size() * 3 + 1);

	URI_CHAR * result;
	result = URI_FUNC(Escape)(first, &buf1[0], URI_TRUE, URI_TRUE);
	result = URI_FUNC(Escape)(first, &buf1[0], URI_FALSE, URI_TRUE);
	if (buf1.data()) {
		URI_FUNC(UnescapeInPlace)(&buf1[0]);
	}

	result = URI_FUNC(Escape)(first, &buf2[0], URI_TRUE, URI_FALSE);
	result = URI_FUNC(Escape)(first, &buf2[0], URI_FALSE, URI_FALSE);
	if (buf2.data()) {
		URI_FUNC(UnescapeInPlace)(&buf2[0]);
	}
}



void FileNames(const UriString & uri) {
	const size_t size = 8 + 3 * uri.size() + 1;
	std::vector<URI_CHAR> buf(size);

	URI_FUNC(UnixFilenameToUriString)(uri.c_str(), &buf[0]);
	URI_FUNC(WindowsFilenameToUriString)(uri.c_str(), &buf[0]);
	URI_FUNC(UriStringToUnixFilename)(uri.c_str(), &buf[0]);
	URI_FUNC(UriStringToWindowsFilename)(uri.c_str(), &buf[0]);
}



void Ipv4(const UriString & s) {
	const URI_CHAR * cstr = s.c_str();
	unsigned char result[4] = {};
	URI_FUNC(ParseIpFourAddress)(result, cstr, &cstr[s.size()]);
}



extern "C" int LLVMFuzzerTestOneInput(const uint8_t * data, size_t size) {
	FuzzedDataProvider stream(data, size);
	bool domainRelative = stream.ConsumeBool();

	const UriString uri1 = consumeRandomLengthString(stream);
	const UriString uri2 = consumeRemainingBytesAsString(stream);

	Escapes(uri1);
	Escapes(uri2);

	FileNames(uri1);
	FileNames(uri2);

	Ipv4(uri1);
	Ipv4(uri2);

	UriHolder uriHolder1;
	URI_TYPE(ParserState) state1;
	state1.uri = uriHolder1.get();
	if (URI_FUNC(ParseUri)(&state1, uri1.c_str()) != URI_SUCCESS) {
		return 0;
	}

	URI_CHAR buf[1024 * 8] = {0};
	int written = 0;
	URI_FUNC(ToString)(buf, state1.uri, sizeof(buf) / sizeof(buf[0]), &written);

	UriHolder uriHolder2;
	if (URI_FUNC(ParseSingleUri)(uriHolder2.get(), uri2.c_str(), nullptr) != URI_SUCCESS) {
		return 0;
	}

	URI_FUNC(EqualsUri)(state1.uri, uriHolder2.get());

	unsigned int mask = 0;
	URI_FUNC(NormalizeSyntaxMaskRequiredEx)(state1.uri, &mask);
	URI_FUNC(NormalizeSyntax)(state1.uri);

	URI_TYPE(Uri) absUri;
	URI_FUNC(AddBaseUri)(&absUri, state1.uri, uriHolder2.get());
	URI_FUNC(FreeUriMembers)(&absUri);

	URI_TYPE(Uri) relUri;
	URI_FUNC(RemoveBaseUri)(&relUri, state1.uri, uriHolder2.get(), domainRelative);
	URI_FUNC(FreeUriMembers)(&relUri);

	return 0;
}
