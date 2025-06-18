/*
 * uriparser - RFC 3986 URI parsing library
 *
 * Copyright (C) 2007, Weijia Song <songweijia@gmail.com>
 * Copyright (C) 2007, Sebastian Pipping <sebastian@pipping.org>
 * All rights reserved.
 *
 * Redistribution and use in source  and binary forms, with or without
 * modification, are permitted provided  that the following conditions
 * are met:
 *
 *     1. Redistributions  of  source  code   must  retain  the  above
 *        copyright notice, this list  of conditions and the following
 *        disclaimer.
 *
 *     2. Redistributions  in binary  form  must  reproduce the  above
 *        copyright notice, this list  of conditions and the following
 *        disclaimer  in  the  documentation  and/or  other  materials
 *        provided with the distribution.
 *
 *     3. Neither the  name of the  copyright holder nor the  names of
 *        its contributors may be used  to endorse or promote products
 *        derived from  this software  without specific  prior written
 *        permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND  ANY EXPRESS OR IMPLIED WARRANTIES,  INCLUDING, BUT NOT
 * LIMITED TO,  THE IMPLIED WARRANTIES OF  MERCHANTABILITY AND FITNESS
 * FOR  A  PARTICULAR  PURPOSE  ARE  DISCLAIMED.  IN  NO  EVENT  SHALL
 * THE  COPYRIGHT HOLDER  OR CONTRIBUTORS  BE LIABLE  FOR ANY  DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL,  EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO,  PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA,  OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT  LIABILITY,  OR  TORT (INCLUDING  NEGLIGENCE  OR  OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * @file UriCopy.c
 * Holds the RFC 3986 %URI normalization implementation.
 * NOTE: This source file includes itself twice.
 */

/* What encodings are enabled? */
#include <uriparser/UriDefsConfig.h>
#if (!defined(URI_PASS_ANSI) && !defined(URI_PASS_UNICODE))
/* Include SELF twice */
# ifdef URI_ENABLE_ANSI
#  define URI_PASS_ANSI 1
#  include "UriCopy.c"
#  undef URI_PASS_ANSI
# endif
# ifdef URI_ENABLE_UNICODE
#  define URI_PASS_UNICODE 1
#  include "UriCopy.c"
#  undef URI_PASS_UNICODE
# endif
#else
# ifdef URI_PASS_ANSI
#  include <uriparser/UriDefsAnsi.h>
# else
#  include <uriparser/UriDefsUnicode.h>
#  include <wchar.h>
# endif



#ifndef URI_DOXYGEN
# include <uriparser/Uri.h>
# include "UriCommon.h"
# include "UriMemory.h"
# include "UriCopy.h"
#endif



UriBool URI_FUNC(CopyRangeEngine)(URI_TYPE(TextRange) * destRange,
		const URI_TYPE(TextRange) * sourceRange, UriMemoryManager * memory) {
	const int lenInChars = (int)(sourceRange->afterLast - sourceRange->first);
	const int lenInBytes = lenInChars * sizeof(URI_CHAR);
	URI_CHAR * dup = memory->malloc(memory, lenInBytes);
	if (dup == NULL) {
		return URI_FALSE;
	}
	memcpy(dup, sourceRange->first, lenInBytes);
	destRange->first = dup;
	destRange->afterLast = dup + lenInChars;

	return URI_TRUE;
}



UriBool URI_FUNC(CopyRange)(URI_TYPE(TextRange) * destRange,
		const URI_TYPE(TextRange) * sourceRange, UriBool useSafe, UriMemoryManager * memory) {
	if (sourceRange->first == NULL || sourceRange->afterLast == NULL || (sourceRange->first > sourceRange->afterLast && !useSafe)) {
		destRange->first = NULL;
		destRange->afterLast = NULL;
	} else if (sourceRange->first >= sourceRange->afterLast && useSafe) {
		destRange->first = URI_FUNC(SafeToPointTo);
		destRange->afterLast = URI_FUNC(SafeToPointTo);
	} else {
		return URI_FUNC(CopyRangeEngine)(destRange, sourceRange, memory);
	}

	return URI_TRUE;
}



int URI_FUNC(CopyUriMm)(URI_TYPE(Uri) * destUri,
		 const URI_TYPE(Uri) * sourceUri, UriMemoryManager * memory) {
	if (sourceUri == NULL) {
		return URI_ERROR_NULL;
	}

	URI_CHECK_MEMORY_MANAGER(memory); /* may return */

	unsigned int doneMask = URI_NORMALIZED;

	if (URI_FUNC(CopyRange)(&destUri->scheme, &sourceUri->scheme, URI_FALSE, memory) == URI_FALSE) {
		return URI_ERROR_MALLOC;
	}

	doneMask |= URI_NORMALIZE_SCHEME;

	if (URI_FUNC(CopyRange)(&destUri->userInfo, &sourceUri->userInfo, URI_FALSE, memory) == URI_FALSE) {
		URI_FUNC(PreventLeakage)(destUri, doneMask, memory);
		return URI_ERROR_MALLOC;
	}

	doneMask |= URI_NORMALIZE_USER_INFO;

	if (URI_FUNC(CopyRange)(&destUri->hostText, &sourceUri->hostText, URI_TRUE, memory) == URI_FALSE) {
		URI_FUNC(PreventLeakage)(destUri, doneMask, memory);
		return URI_ERROR_MALLOC;
	}

	doneMask |= URI_NORMALIZE_USER_INFO;

	if (sourceUri->hostData.ip4 == NULL) {
		destUri->hostData.ip4 = NULL;
	} else {
		destUri->hostData.ip4 = memory->malloc(memory, sizeof(UriIp4));
		if (destUri->hostData.ip4 == NULL) {
			URI_FUNC(PreventLeakage)(destUri, doneMask, memory);
			return URI_ERROR_MALLOC;
		}
		*(destUri->hostData.ip4) = *(sourceUri->hostData.ip4);
	}

	doneMask |= URI_NORMALIZE_HOST;

	if (sourceUri->hostData.ip6 == NULL) {
		destUri->hostData.ip6 = NULL;
	} else {
		destUri->hostData.ip6 = memory->malloc(memory, sizeof(UriIp6));
		if (destUri->hostData.ip6 == NULL) {
			URI_FUNC(PreventLeakage)(destUri, doneMask, memory);
			return URI_ERROR_MALLOC;
		}
		*(destUri->hostData.ip6) = *(sourceUri->hostData.ip6);
	}

	if (URI_FUNC(CopyRange)(&destUri->hostData.ipFuture, &sourceUri->hostData.ipFuture, URI_FALSE, memory) == URI_FALSE) {
		URI_FUNC(PreventLeakage)(destUri, doneMask, memory);
		return URI_ERROR_MALLOC;
	}

	if (URI_FUNC(CopyRange)(&destUri->portText, &sourceUri->portText, URI_FALSE, memory) == URI_FALSE) {
		URI_FUNC(PreventLeakage)(destUri, doneMask, memory);
		return URI_ERROR_MALLOC;
	}

	doneMask |= URI_NORMALIZE_PORT;

	if (sourceUri->pathHead != NULL && sourceUri->pathTail != NULL) {
		URI_TYPE(PathSegment) *walker;
		URI_TYPE(PathSegment) *walkerNew;

		destUri->pathHead = memory->malloc(memory, sizeof(URI_TYPE(PathSegment)));
		if (destUri->pathHead == NULL) {
			URI_FUNC(PreventLeakage)(destUri, doneMask, memory);
			return URI_ERROR_MALLOC;
		}

		if (URI_FUNC(CopyRange)(&destUri->pathHead->text, &sourceUri->pathHead->text, URI_TRUE, memory) == URI_FALSE) {
			URI_FUNC(PreventLeakage)(destUri, doneMask, memory);
			memory->free(memory, destUri->pathHead);
			return URI_ERROR_MALLOC;
		}
		destUri->pathHead->reserved = NULL;

		doneMask |= URI_NORMALIZE_PATH;

		walker = sourceUri->pathHead->next;
		walkerNew = destUri->pathHead;
		while (walker != NULL && (walker->text.first != walker->text.afterLast || walker->text.first == URI_FUNC(SafeToPointTo))) {
			walkerNew->next = memory->malloc(memory, sizeof(URI_TYPE(PathSegment)));
			if (walkerNew->next == NULL) {
				URI_FUNC(PreventLeakage)(destUri, doneMask, memory);
				return URI_ERROR_MALLOC;
			}

			walkerNew = walkerNew->next;
			if (URI_FUNC(CopyRange)(&walkerNew->text, &walker->text, URI_TRUE, memory) == URI_FALSE) {
				URI_FUNC(PreventLeakage)(destUri, doneMask, memory);
				return URI_ERROR_MALLOC;
			}
			walkerNew->reserved = NULL;
			walker = walker->next;
		}
		walkerNew->next = NULL;
		destUri->pathTail = walkerNew;
	} else {
		destUri->pathHead = NULL;
		destUri->pathTail = NULL;
	}

	if (URI_FUNC(CopyRange)(&destUri->query, &sourceUri->query, URI_FALSE, memory) == URI_FALSE) {
		URI_FUNC(PreventLeakage)(destUri, doneMask, memory);
		return URI_ERROR_MALLOC;
	}

	doneMask |= URI_NORMALIZE_QUERY;

	if (URI_FUNC(CopyRange)(&destUri->fragment, &sourceUri->fragment, URI_FALSE, memory) == URI_FALSE) {
		URI_FUNC(PreventLeakage)(destUri, doneMask, memory);
		return URI_ERROR_MALLOC;
	}

	destUri->absolutePath = sourceUri->absolutePath;
	destUri->owner = URI_TRUE;
	destUri->reserved = NULL;

	return URI_SUCCESS;
}



int URI_FUNC(CopyUri)(URI_TYPE(Uri) * destUri,
		const URI_TYPE(Uri) * sourceUri) {
	return URI_FUNC(CopyUriMm)(destUri, sourceUri, NULL);
}

#endif
