#ifndef CODUOMP_CASE_SENSITIVE_FS_H
#define CODUOMP_CASE_SENSITIVE_FS_H

#include "../q_shared.h"

#include <stddef.h>
#include <stdio.h>

#ifndef CASE_SENSITIVE_FS
#define CASE_SENSITIVE_FS 1
#endif

qboolean coduomp_resolve_case_path(const char *trustedRoot,
                                   const char *requestedPath,
                                   char *resolvedPath,
                                   size_t resolvedPathSize);
FILE *coduomp_fopen_case_read(const char *trustedRoot,
                              const char *requestedPath);
void coduomp_case_path_cache_clear(void);

#endif
