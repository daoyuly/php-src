/*
   +----------------------------------------------------------------------+
   | Zend Engine                                                          |
   +----------------------------------------------------------------------+
   | Copyright (c) 1998-2014 Zend Technologies Ltd. (http://www.zend.com) |
   +----------------------------------------------------------------------+
   | This source file is subject to version 2.00 of the Zend license,     |
   | that is bundled with this package in the file LICENSE, and is        | 
   | available through the world-wide-web at the following url:           |
   | http://www.zend.com/license/2_00.txt.                                |
   | If you did not receive a copy of the Zend license and are unable to  |
   | obtain it through the world-wide-web, please send a note to          |
   | license@zend.com so we can mail you a copy immediately.              |
   +----------------------------------------------------------------------+
   | Authors: Sterling Hughes <sterling@php.net>                          |
   +----------------------------------------------------------------------+
*/

/* $Id$ */

#include "zend.h"
#include "zend_qsort.h"

static void zend_sort_3(void *a, void *b, void *c, swap_func_t swp, compare_func_t cmp) /* {{{ */ {
	if (cmp(b, a) >= 0) {
		if (cmp(c, b) >= 0) {
			return;
		}
		swp(b, c);
		if (cmp(a, b) > 0) {
			swp(a, b);
		}
		return;
	}
	if (cmp(b, c) >= 0) {
		swp(a, c);
		return;
	}
	swp(a, b);
	if (cmp(b, c) > 0) {
		swp(b, c);
	}
}
/* }}} */

static void zend_sort_4(void *a, void *b, void *c, void *d, swap_func_t swp, compare_func_t cmp) /* {{{ */ {
	zend_sort_3(a, b, c, swp, cmp);
	if (cmp(d, c) < 0) {
		swp(d, c);
		if (cmp(c, b) < 0) {
			swp(c, b);
			if (cmp(b, a) < 0) {
				swp(b, a);
			}
		}
	}
}
/* }}} */

static void zend_sort_5(void *a, void *b, void *c, void *d, void *e, swap_func_t swp, compare_func_t cmp) /* {{{ */ {
	zend_sort_4(a, b, c, d, swp, cmp);
	if (cmp(e, d) < 0) {
		swp(e, d);
		if (cmp(d, c) < 0) {
			swp(d, c);
			if (cmp(c, b) < 0) {
				swp(c, b);
				if (cmp(b, a) < 0) {
					swp(b, a);
				}
			}
		}
	}
}
/* }}} */

ZEND_API void zend_insert_sort(void *base, size_t nmemb, size_t siz, swap_func_t swp, compare_func_t cmp) /* {{{ */{
	if (nmemb < 2) {
		return;
	} else {
		char *i, *j, *k;
		char *start = (char *)base;
		char *end = start + (nmemb * siz);
		char *sentry = start + ((nmemb < 6? nmemb : 6) * siz);
		for (i = start + siz; i < sentry; i += siz) {
			j = i - siz;
			if (cmp(j, i) <= 0) {
				continue;
			}
			while (j != start) {
				j -= siz;
				if (cmp(j, i) <= 0) {
					j += siz;
					break;
				}
			}
			for (k = i; k > j; k -= siz) {
				swp(k, k - siz);
			}
		}
		for (i = sentry; i < end; i += siz) {
			j = i - siz;
			if (cmp(j, i) <= 0) {
				continue;
			}
			do {
				j -= siz * 2;
				if (cmp(j, i) <= 0) {
					j += siz;
					if (cmp(j, i) <= 0) {
						j+=siz;
					}
					break;
				}
				if (j == start) {
					break;
				}
				if (j == start + siz) {
					j -= siz;
					if (cmp(j, i) < 0) {
						j += siz;
					}
					break;
				}
			} while (1);
			for (k = i; k > j; k -= siz) {
				swp(k, k - siz);
			}
		}
	}
}
/* }}} */

ZEND_API void zend_sort(void *base, size_t nmemb, size_t siz, swap_func_t swp, compare_func_t cmp) /* {{{ */
{
	while (1) {
		if (nmemb <= 16) {
			return zend_insert_sort(base, nmemb, siz, swp, cmp);
		} else {
			char *i, *j;
			char *start = (char *)base;
			char *end = start + (nmemb * siz);
			size_t offset = nmemb >> Z_L(1);
			char *pivot = start + ((offset - 1) * siz);

			if (nmemb >= 1024) {
				size_t delta;
				offset = offset >> Z_L(1);
				delta = (offset - 1) * siz;
				zend_sort_5(start, start + delta, pivot, pivot + delta, end - siz, swp, cmp);
			} else {
				zend_sort_3(start, pivot, end - siz, swp, cmp);
			}
			swp(start + siz, pivot);
			pivot = start + siz;
			i = pivot + siz;
			j = end - siz;
			while (1) {
				while (cmp(pivot, i) > 0) {
					i += siz;
					if (UNEXPECTED(i == j)) {
						goto done;
					}
				}
				j -= siz;
				if (UNEXPECTED(j == i)) {
					goto done;
				}
				while (cmp(j, pivot) > 0) {
					j -= siz;
					if (UNEXPECTED(j == i)) {
						goto done;
					}
				}
				swp(i, j);
				i += siz;
				if (UNEXPECTED(i == j)) {
					goto done;
				}
			}
done:
			swp(pivot, i - siz);
			if ((i - siz) - start < end - i) {
				zend_sort(start, (i - start)/siz - 1, siz, swp, cmp);
				base = i;
				nmemb = (end - i)/siz;
			} else {
				zend_sort(i, (end - i)/siz, siz, swp, cmp);
				nmemb = (i - start)/siz - 1;
			}
		}
	}
}
/* }}} */

/* 
 * Local Variables:
 * c-basic-offset: 4 
 * tab-width: 4
 * End:
 * vim600: fdm=marker
 * vim: noet sw=4 ts=4
 */
