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
#include "FuzzingUtils.h"
#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>



extern "C" int LLVMFuzzerTestOneInput(const uint8_t * data, size_t size) {
	FuzzedDataProvider fdp(data, size);
	const UriString query = consumeRemainingBytesAsString(fdp);

	URI_TYPE(QueryList) * query_list = nullptr;
	int item_count = -1;

	const URI_CHAR * query_start = query.c_str();
	const URI_CHAR * query_end = query_start + query.size();

	// Break a query like "a=b&2=3" into key/value pairs.
	int result = URI_FUNC(DissectQueryMalloc)(&query_list, &item_count, query_start, query_end);

	if (query_list == nullptr || result != URI_SUCCESS || item_count < 0) {
		return 0;
	}

	int chars_required = 0;
	if (URI_FUNC(ComposeQueryCharsRequired)(query_list, &chars_required) != URI_SUCCESS) {
		return 0;
	}

	if (!chars_required) {
		URI_FUNC(FreeQueryList)(query_list);
		return 0;
	}

	// URI_FUNC(ComposeQuery) requires number of characters including terminator
	const int buf_size = chars_required + 1;

	std::vector<URI_CHAR> buf(buf_size, 0);
	int written = -1;

	// Reverse the process of uriDissectQueryMallocA.
	result = URI_FUNC(ComposeQuery)(buf.data(), query_list, buf_size, &written);

	URI_FUNC(FreeQueryList)(query_list);
	return 0;
}
