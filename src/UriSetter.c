/*
 * uriparser - RFC 3986 URI parsing library
 *
 * Copyright (C) 2007, Weijia Song <songweijia@gmail.com>
 * Copyright (C) 2007, Sebastian Pipping <sebastian@pipping.org>
 * Copyright (C) 2025, Máté Kocsis <kocsismate@php.net>
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
 * @file UriSetter.c
 * Holds the RFC 3986 %URI setter implementation.
 * NOTE: This source file includes itself twice.
 */

/* What encodings are enabled? */
#include <uriparser/UriDefsConfig.h>
#if (!defined(URI_PASS_ANSI) && !defined(URI_PASS_UNICODE))
/* Include SELF twice */
# ifdef URI_ENABLE_ANSI
#  define URI_PASS_ANSI 1
#  include "UriSetter.c"
#  undef URI_PASS_ANSI
# endif
# ifdef URI_ENABLE_UNICODE
#  define URI_PASS_UNICODE 1
#  include "UriSetter.c"
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
# include "UriNormalizeBase.h"
# include "UriCommon.h"
# include "UriMemory.h"
#endif



#include <assert.h>



static int URI_FUNC(SetSchemeEngine)(URI_TYPE(Uri) * uri,
		const URI_CHAR * first, const URI_CHAR * afterLast,
		UriMemoryManager * memory);

static URI_INLINE UriBool URI_FUNC(SetRange)(URI_TYPE(TextRange) * range,
		const URI_CHAR * first, const URI_CHAR * afterLast,
		UriBool owner, UriMemoryManager * memory);



int URI_FUNC(SetSchemeEx)(URI_TYPE(Uri) * uri,
		const URI_CHAR * first, const URI_CHAR * afterLast) {
	return URI_FUNC(SetSchemeExMm)(uri, first, afterLast, NULL);
}



int URI_FUNC(SetSchemeExMm)(URI_TYPE(Uri) * uri,
		const URI_CHAR * first, const URI_CHAR * afterLast,
		UriMemoryManager * memory) {
	URI_CHECK_MEMORY_MANAGER(memory);  /* may return */
	return URI_FUNC(SetSchemeEngine)(uri, first, afterLast, memory);
}



int URI_FUNC(SetScheme)(URI_TYPE(Uri) * uri,
		const URI_CHAR * first, const URI_CHAR * afterLast) {
	return URI_FUNC(SetSchemeEx)(uri, first, afterLast);
}



static UriBool URI_FUNC(ValidateScheme)(const URI_CHAR * first,
	const URI_CHAR * afterLast) {
	const URI_CHAR * i;

	switch (*first) {
	case URI_SET_ALPHA:
		break;
	default:
		return URI_FALSE;
	}

	for (i = first + 1; i < afterLast; i++) {
		switch (*first) {
		case _UT('.'):
		case _UT('+'):
		case _UT('-'):
		case URI_SET_ALPHA:
		case URI_SET_DIGIT:
			continue;
		default:
			return URI_FALSE;
		}
	}

	return URI_TRUE;
}



static int URI_FUNC(SetSchemeEngine)(URI_TYPE(Uri) * uri,
		const URI_CHAR * first, const URI_CHAR * afterLast,
		UriMemoryManager * memory) {
	if (uri == NULL) {
		return URI_ERROR_NULL;
	}

	if (first != NULL && URI_FUNC(ValidateScheme)(first, afterLast) == URI_FALSE) {
		return URI_ERROR_SYNTAX;
	}

	if (URI_FUNC(SetRange)(&uri->scheme, first, afterLast, uri->owner, memory) == URI_FALSE) {
		return URI_ERROR_MALLOC;
	}

	int charsRequired;
	if (URI_FUNC(ToStringCharsRequired)(uri, &charsRequired) != URI_SUCCESS) {
		return URI_ERROR_MALLOC;
	}

	charsRequired++;
	int bytesRequired = charsRequired * sizeof(URI_CHAR);

	URI_CHAR * uriString = memory->malloc(memory, bytesRequired);
	if (uriString == NULL) {
		return URI_ERROR_MALLOC;
	}

	if (URI_FUNC(ToString)(uriString, uri, charsRequired, NULL) != URI_SUCCESS) {
		memory->free(memory, uriString);
		return URI_ERROR_MALLOC;
	}

	URI_TYPE(Uri) *uri2 = memory->malloc(memory, sizeof(URI_TYPE(Uri)));
	if (uriString == NULL) {
		memory->free(memory, uriString);
		return URI_ERROR_MALLOC;
	}
	int result = URI_FUNC(ParseSingleUriExMm)(uri2, uriString, uriString + bytesRequired - 1, NULL, memory);

	URI_FUNC(FreeUriMembersMm)(uri2, memory);
	memory->free(memory, uriString);

	return result;
}



static URI_INLINE UriBool URI_FUNC(SetRange)(URI_TYPE(TextRange) * range,
		const URI_CHAR * first, const URI_CHAR * afterLast,
		UriBool owner, UriMemoryManager * memory) {
	if ((first != NULL) && (afterLast != NULL) && (afterLast > first)) {
		const int lenInChars = (int)(afterLast - first);
		const int lenInBytes = lenInChars * sizeof(URI_CHAR);
		URI_CHAR * dup = memory->malloc(memory, lenInBytes);
		if (dup == NULL) {
			return URI_FALSE; /* Raises malloc error */
		}

		if (owner && range->first != range->afterLast) {
			memory->free(memory, (URI_CHAR *)range->first);
		}

		memcpy(dup, first, lenInBytes);
		range->first = dup;
		range->afterLast = dup + lenInChars;
	} else {
		range->first = NULL;
		range->afterLast = NULL;
	}

	return URI_TRUE;
}

#endif
