/* See LICENSE file for copyright and license details. */
#ifndef UTIL_H
#define UTIL_H

#define MAX(A, B)               ((A) > (B) ? (A) : (B))
#define MIN(A, B)               ((A) < (B) ? (A) : (B))
#define BETWEEN(X, A, B)        ((A) <= (X) && (X) <= (B))
#define LENGTH(X)               (sizeof(X) / sizeof *(X))

void die(const char *fmt, ...);
void *ecalloc(size_t nmemb, size_t size);

#endif
