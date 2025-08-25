/*
 * uriparser - RFC 3986 URI parsing library
 *
 * Copyright (C) 2025, Sebastian Pipping <sebastian@pipping.org>
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
 * @file UriSetHostCommon.c
 * Holds code used by multiple SetHost* functions.
 * NOTE: This source file includes itself twice.
 */

/* What encodings are enabled? */
#include <uriparser/UriDefsConfig.h>
#if (!defined(URI_PASS_ANSI) && !defined(URI_PASS_UNICODE))
/* Include SELF twice */
# ifdef URI_ENABLE_ANSI
#  define URI_PASS_ANSI 1
#  include "UriSetHostCommon.c"
#  undef URI_PASS_ANSI
# endif
# ifdef URI_ENABLE_UNICODE
#  define URI_PASS_UNICODE 1
#  include "UriSetHostCommon.c"
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
# include "UriSetHostBase.h"
# include "UriSetHostCommon.h"
#endif



#include <assert.h>



int URI_FUNC(InternalSetHostMm)(URI_TYPE(Uri) * uri,
		UriHostType hostType,
		const URI_CHAR * first,
		const URI_CHAR * afterLast,
		UriMemoryManager * memory) {
	/* Superficial input validation (before making any changes) */
	if ((uri == NULL) || ((first == NULL) != (afterLast == NULL))) {
		return URI_ERROR_NULL;
	}

	URI_CHECK_MEMORY_MANAGER(memory);  /* may return */

	/* The RFC 3986 grammar reads:
	 *   authority = [ userinfo "@" ] host [ ":" port ]
	 * So no user info or port without a host. */
	if (first == NULL) {
		if (uri->userInfo.first != NULL) {
			return URI_ERROR_SETHOST_USERINFO_SET;
		} else if (uri->portText.first != NULL) {
			return URI_ERROR_SETHOST_PORT_SET;
		}
	}

	/* Syntax-check the new value */
	if (first != NULL) {
		switch (hostType) {
			case URI_HOST_TYPE_REGNAME:
				if (URI_FUNC(IsWellFormedHostRegName)(first, afterLast) == URI_FALSE) {
					return URI_ERROR_SYNTAX;
				}
				break;
			default:
				assert(0 && "Unsupported URI host type");
		}
	}

	{
		/* Clear old value */
		const UriBool hadHostBefore = URI_FUNC(HasHost)(uri);
		if (uri->hostData.ipFuture.first != NULL) {
			/* NOTE: .hostData.ipFuture holds the very same range pointers
			 *       as .hostText; we must not free memory twice. */
			uri->hostText.first = NULL;
			uri->hostText.afterLast = NULL;

			if ((uri->owner == URI_TRUE) && (uri->hostData.ipFuture.first != uri->hostData.ipFuture.afterLast)) {
				memory->free(memory, (URI_CHAR *)uri->hostData.ipFuture.first);
			}
			uri->hostData.ipFuture.first = NULL;
			uri->hostData.ipFuture.afterLast = NULL;
		} else if (uri->hostText.first != NULL) {
			if ((uri->owner == URI_TRUE) && (uri->hostText.first != uri->hostText.afterLast)) {
				memory->free(memory, (URI_CHAR *)uri->hostText.first);
			}
			uri->hostText.first = NULL;
			uri->hostText.afterLast = NULL;
		}

		if (uri->hostData.ip4 != NULL) {
			memory->free(memory, uri->hostData.ip4);
			uri->hostData.ip4 = NULL;
		} else if (uri->hostData.ip6 != NULL) {
			memory->free(memory, uri->hostData.ip6);
			uri->hostData.ip6 = NULL;
		}

		/* Already done setting? */
		if (first == NULL) {
			/* Yes, but disambiguate as needed */
			if (hadHostBefore == URI_TRUE) {
				uri->absolutePath = URI_TRUE;

				{
					const UriBool success = URI_FUNC(EnsureThatPathIsNotMistakenForHost)(uri, memory);
					return (success == URI_TRUE)
							? URI_SUCCESS
							: URI_ERROR_MALLOC;
				}
			}

			return URI_SUCCESS;
		}
	}

	assert(first != NULL);

	/* Ensure owned */
	if (uri->owner == URI_FALSE) {
		const int res = URI_FUNC(MakeOwnerMm)(uri, memory);
		if (res != URI_SUCCESS) {
			return res;
		}
	}

	assert(uri->owner == URI_TRUE);

	/* Apply new value; NOTE that .hostText is set for all four host types */
	{
		URI_TYPE(TextRange) sourceRange;
		sourceRange.first = first;
		sourceRange.afterLast = afterLast;

		if (URI_FUNC(CopyRangeAsNeeded)(&uri->hostText, &sourceRange, memory) == URI_FALSE) {
			return URI_ERROR_MALLOC;
		}

		uri->absolutePath = URI_FALSE;  /* always URI_FALSE for URIs with host  */

		/* Fill .hostData as needed */
		switch (hostType) {
			case URI_HOST_TYPE_REGNAME:
				break;
			default:
				assert(0 && "Unsupported URI host type");
		}
	}

	return URI_SUCCESS;
}



#endif
