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

#ifndef URI_FUZZING_UTILS_H
#define URI_FUZZING_UTILS_H 1

#include "uriparser/Uri.h"
#include <fuzzer/FuzzedDataProvider.h>
#include <string>



using UriString = std::basic_string<URI_CHAR>;



inline UriString tryConsumeBytesAsString(FuzzedDataProvider & fdp, size_t chars) {
	UriString str(chars, 0);
	const size_t bytes = fdp.ConsumeData(str.data(), chars * sizeof(URI_CHAR));
	// FuzzedDataProvider may provide less data than requested if the input is
	// insufficiently long. We need to adjust the string length accordingly.
	str.resize(bytes / sizeof(URI_CHAR));
	return str;
}



inline UriString consumeRandomLengthString(FuzzedDataProvider & fdp) {
	const size_t max_chars = fdp.remaining_bytes() / sizeof(URI_CHAR);
	const size_t chars = fdp.ConsumeIntegralInRange<size_t>(0, max_chars);
	return tryConsumeBytesAsString(fdp, chars);
}



inline UriString consumeRemainingBytesAsString(FuzzedDataProvider & fdp) {
	const size_t chars = fdp.remaining_bytes() / sizeof(URI_CHAR);
	return tryConsumeBytesAsString(fdp, chars);
}

#endif /* URI_FUZZING_UTILS_H */
