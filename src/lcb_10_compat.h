#ifndef LCB_10_COMPAT_H_
#define LCB_10_COMPAT_H_

#include <stdlib.h>
#include <stdio.h>
#include <libcouchbase/couchbase.h>

#ifdef LCB_VERSION
/* libcouchbase 2.0 */

#define LIBCOUCHBASE_SUCCESS           LCB_SUCCESS
#define LIBCOUCHBASE_BUCKET_ENOENT     LCB_BUCKET_ENOENT
#define LIBCOUCHBASE_AUTH_ERROR        LCB_AUTH_ERROR
#define LIBCOUCHBASE_CONNECT_ERROR     LCB_CONNECT_ERROR
#define LIBCOUCHBASE_NETWORK_ERROR     LCB_NETWORK_ERROR
#define LIBCOUCHBASE_ENOMEM            LCB_ENOMEM
#define LIBCOUCHBASE_KEY_ENOENT        LCB_KEY_ENOENT
#define LIBCOUCHBASE_ETIMEDOUT         LCB_ETIMEDOUT
#define LIBCOUCHBASE_ETMPFAIL          LCB_ETMPFAIL
#define LIBCOUCHBASE_APPEND            LCB_APPEND
#define LIBCOUCHBASE_PREPEND           LCB_PREPEND
#define LIBCOUCHBASE_SET               LCB_SET
#define LIBCOUCHBASE_REPLACE           LCB_REPLACE
#define LIBCOUCHBASE_ADD               LCB_ADD


#define libcouchbase_t lcb_t
#define libcouchbase_size_t            lcb_size_t
#define libcouchbase_cas_t             lcb_cas_t
#define libcouchbase_uint16_t          lcb_uint16_t
#define libcouchbase_uint32_t          lcb_uint32_t
#define libcouchbase_int64_t           lcb_int64_t
#define libcouchbase_time_t            lcb_time_t
#define libcouchbase_error_t           lcb_error_t
#define libcouchbase_get_cookie        lcb_get_cookie
#define libcouchbase_set_cookie        lcb_set_cookie
#define libcouchbase_get_version       lcb_get_version
#define libcouchbase_destroy           lcb_destroy
#define libcouchbase_connect           lcb_connect
#define libcouchbase_set_timeout       lcb_set_timeout
#define libcouchbase_set_error_callback lcb_set_error_callback
#define libcouchbase_set_touch_callback lcb_set_touch_callback
#define libcouchbase_set_remove_callback lcb_set_remove_callback
#define libcouchbase_set_get_callback  lcb_set_get_callback
#define libcouchbase_strerror          lcb_strerror
#define libcouchbase_wait              lcb_wait
#define libcouchbase_storage_t         lcb_storage_t

#define CMDFLD(cmd) (cmd).v.v0

#ifdef __cplusplus
extern "C" {
#endif

extern const char *SDKD_Conncache_Path;

static inline libcouchbase_error_t
libcouchbase_store(lcb_t instance,
                   const void *cookie,
                   libcouchbase_storage_t operation,
                   const void *key, size_t nkey,
                   const void *value, size_t nvalue,
                   uint64_t cas,
                   time_t exp,
                   uint32_t flags)
{
    struct lcb_store_cmd_st storecmd;
    const struct lcb_store_cmd_st *cmdp;

    memset(&storecmd, 0, sizeof(storecmd));
    storecmd.v.v0.key = key;
    storecmd.v.v0.nkey = nkey;
    storecmd.v.v0.bytes = value;
    storecmd.v.v0.nbytes = nvalue;
    storecmd.v.v0.exptime = exp;
    storecmd.v.v0.cas = cas;
    storecmd.v.v0.flags = flags;
    storecmd.v.v0.operation = operation;
    cmdp = &storecmd;

    return lcb_store(instance, cookie, 1, &cmdp);
}

static inline libcouchbase_error_t
libcouchbase_remove(lcb_t instance,
                    const void *cookie,
                    const void *key, size_t nkey,
                    uint64_t cas)
{
    struct lcb_remove_cmd_st rmcmd;
    const struct lcb_remove_cmd_st *cmdp;
    CMDFLD(rmcmd).cas = cas;
    CMDFLD(rmcmd).key = key;
    CMDFLD(rmcmd).nkey = nkey;
    cmdp = &rmcmd;
    return lcb_remove(instance, cookie, 1, &cmdp);
}

static inline libcouchbase_error_t
libcouchbase__mtouch_mget_common(lcb_t instance,
                                 const void *cookie,
                                 int is_get,
                                 size_t nkeys,
                                 const void * const *keys,
                                 const size_t *sizes,
                                 const time_t *exps)
{
    int is_allocated = 0;
    lcb_touch_cmd_t stacked[64];
    lcb_touch_cmd_t *allocated = NULL;
    lcb_touch_cmd_t *current;
    lcb_error_t retval;
    unsigned ii;

    if (nkeys <= 64) {
        current = stacked;
        memset(stacked, 0, sizeof(stacked));
    } else {
        allocated = (lcb_touch_cmd_t*)calloc(nkeys, sizeof(*current));
        current = allocated;
        is_allocated = 1;
    }

    for (ii = 0; ii < nkeys; ii++) {
        CMDFLD(current[ii]).key = keys[ii];
        CMDFLD(current[ii]).nkey = sizes[ii];
        if (exps) {
            CMDFLD(current[ii]).exptime = exps[ii];
        }
    }

    if (is_get) {
        retval = lcb_get(instance, cookie, nkeys, &current);
    } else {
        retval = lcb_touch(instance, cookie, nkeys, &current);
    }

    if (is_allocated) {
        free (allocated);
    }
    return retval;
}

static inline libcouchbase_error_t
libcouchbase_mget(lcb_t instance, const void *cookie,
                  size_t nkeys,
                  const void * const * keys,
                  const size_t *sizes,
                  const time_t *exps)
{
    return libcouchbase__mtouch_mget_common(instance,
            cookie, 1, nkeys, keys, sizes, exps);
}

static inline libcouchbase_error_t
libcouchbase_mtouch(lcb_t instance, const void *cookie,
                    size_t nkeys, const void * const * keys,
                    const size_t *sizes,
                    const time_t *exps)
{
    return libcouchbase__mtouch_mget_common(instance,
            cookie, 0, nkeys, keys, sizes, exps);
}

static inline lcb_t
libcouchbase_create(const char *host,
                    const char *username,
                    const char *password,
                    const char *bucket,
                    lcb_io_opt_st *iops)
{
    struct lcb_create_st cropts;
    lcb_t obj = NULL;
    lcb_error_t err;

    memset(&cropts, 0, sizeof(cropts));
    CMDFLD(cropts).bucket = bucket;
    CMDFLD(cropts).user = username;
    CMDFLD(cropts).passwd = password;
    CMDFLD(cropts).host = host;
    CMDFLD(cropts).io = iops;

    if (SDKD_Conncache_Path) {
        lcb_cached_config_st cached;
        memset(&cached, 0, sizeof(cached));
        memcpy(&cached.createopt, &cropts, sizeof(cropts));
        cached.cachefile = SDKD_Conncache_Path;
        err = lcb_create_compat(LCB_CACHED_CONFIG, &cached, &obj, NULL);
        if (err != LCB_SUCCESS) {
            fprintf(stderr, "Couldn't create cached handle: %d\n", err);
            return NULL;
        }
    } else {
        if (LCB_SUCCESS != (err = lcb_create(&obj, &cropts)) ) {
            fprintf(stderr, "Couldn't create handle: EC: %d\n", err);
            return NULL;
        }
    }

    return obj;
}

#ifdef __cplusplus
}
#endif

#endif /* LCB_VERSION */

#endif /* LCB_10_COMPAT_H_ */
