#include "aos_util.h"
#include "aos_log.h"

static const char *g_s_wday[] = {
    "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
};

static const char *g_s_mon[] = {
    "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

static const char g_s_gmt_format[] = "%s, %.2d %s %.4d %.2d:%.2d:%.2d GMT";

int aos_parse_xml_body(aos_list_t *bc, mxml_node_t **root)
{
    aos_buf_t *b;
    size_t len;

    *root = NULL;
    len = (size_t)aos_buf_list_len(bc);

    {
        int nsize = 0;
        char *buffer = (char*)malloc(sizeof(char)*(len+1));
        memset(buffer, 0, len + 1);
        aos_list_for_each_entry(aos_buf_t, b, bc, node) {
            memcpy(buffer + nsize, (char *)b->pos, aos_buf_size(b));
            nsize += aos_buf_size(b);
        }
        *root = mxmlLoadString(NULL, buffer, MXML_OPAQUE_CALLBACK);
        free(buffer);
        if (NULL == *root) {
           return AOSE_INTERNAL_ERROR; 
        }
    }

    return AOSE_OK;
}

int aos_convert_to_gmt_time(char* date, const char* format, apr_time_exp_t *tm)
{
    int size = apr_snprintf(date, AOS_MAX_GMT_TIME_LEN, format, 
        g_s_wday[tm->tm_wday], tm->tm_mday, g_s_mon[tm->tm_mon], 1900 + tm->tm_year, tm->tm_hour, tm->tm_min, tm->tm_sec);
    if (size >= 0 && size < AOS_MAX_GMT_TIME_LEN) {
        return AOSE_OK;
    } else {
        return AOSE_INTERNAL_ERROR;
    }
}

int aos_get_gmt_str_time(char datestr[AOS_MAX_GMT_TIME_LEN])
{
    int s;
    apr_time_t now;
    char buf[128];
    apr_time_exp_t result;

    now = apr_time_now();
    if ((s = apr_time_exp_gmt(&result, now)) != APR_SUCCESS) {
        aos_error_log("apr_time_exp_gmt fialure, code:%d %s.", s, apr_strerror(s, buf, sizeof(buf)));
        return AOSE_INTERNAL_ERROR;
    }
    
    if ((s = aos_convert_to_gmt_time(datestr, g_s_gmt_format, &result))
        != AOSE_OK) {
        aos_error_log("aos_convert_to_GMT failure, code:%d.", s);
    }

    return s;
}

int aos_url_encode(char *dest, const char *src, int maxSrcSize)
{
    static const char *hex = "0123456789ABCDEF";

    int len = 0;
    unsigned char c;

    while (*src) {
        if (++len > maxSrcSize) {
            *dest = 0;
            return AOSE_INVALID_ARGUMENT;
        }
        c = *src;
        if (isalnum(c) || (c == '-') || (c == '_') || (c == '.') || (c == '~')) {
            *dest++ = c;
        } else if (*src == ' ') {
            *dest++ = '%';
            *dest++ = '2';
            *dest++ = '0';
        } else {
            *dest++ = '%';
            *dest++ = hex[c >> 4];
            *dest++ = hex[c & 15];
        }
        src++;
    }

    *dest = 0;

    return AOSE_OK;
}

int aos_query_params_to_string(aos_pool_t *p, aos_table_t *query_params, aos_string_t *querystr)
{
    int rs;
    int pos;
    int len;
    char sep = '?';
    char ebuf[AOS_MAX_QUERY_ARG_LEN*3+1];
    char abuf[AOS_MAX_QUERY_ARG_LEN*6+128];
    int max_len;
    const aos_array_header_t *tarr;
    const aos_table_entry_t *telts;
    aos_buf_t *querybuf;

    if (apr_is_empty_table(query_params)) {
        return AOSE_OK;
    }

    max_len = sizeof(abuf)-1;
    querybuf = aos_create_buf(p, 256);
    aos_str_null(querystr);

    tarr = aos_table_elts(query_params);
    telts = (aos_table_entry_t*)tarr->elts;
    
    for (pos = 0; pos < tarr->nelts; ++pos) {
        if ((rs = aos_url_encode(ebuf, telts[pos].key, AOS_MAX_QUERY_ARG_LEN)) != AOSE_OK) {
            aos_error_log("query params args too big, key:%s.", telts[pos].key);
            return AOSE_INVALID_ARGUMENT;
        }
        len = apr_snprintf(abuf, max_len, "%c%s", sep, ebuf);
        if (telts[pos].val != NULL && *telts[pos].val != '\0') {
            if ((rs = aos_url_encode(ebuf, telts[pos].val, AOS_MAX_QUERY_ARG_LEN)) != AOSE_OK) {
                aos_error_log("query params args too big, value:%s.", telts[pos].val);
                return AOSE_INVALID_ARGUMENT;
            }
            len += apr_snprintf(abuf+len, max_len-len, "=%s", ebuf);
            if (len >= AOS_MAX_QUERY_ARG_LEN) {
                aos_error_log("query params args too big, %s.", abuf);
                return AOSE_INVALID_ARGUMENT;
            }
        }
        aos_buf_append_string(p, querybuf, abuf, len);
        sep = '&';
    }

    // result
    querystr->data = (char *)querybuf->pos;
    querystr->len = aos_buf_size(querybuf);
    
    return AOSE_OK;
}

void aos_gnome_sort(const char **headers, int size)
{
    const char *tmp;
    int i = 0, last_highest = 0;

    while (i < size) {
        if ((i == 0) || apr_strnatcasecmp(headers[i-1], headers[i]) < 0) {
            i = ++last_highest;
        } else {
            tmp = headers[i];
            headers[i] = headers[i - 1];
            headers[--i] = tmp;
        }
    }
}

const char* aos_http_method_to_string(http_method_e method)
{
    switch (method) {
        case HTTP_GET:
            return "GET";
        case HTTP_HEAD:
            return "HEAD";
        case HTTP_PUT:
            return "PUT";
        case HTTP_POST:
            return "POST";
        case HTTP_DELETE:
            return "DELETE";
        default:
            return "UNKNOWN";
    }
}

int aos_base64_encode(const unsigned char *in, int inLen, char *out)
{
    static const char *ENC = 
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    char *original_out = out;

    while (inLen) {
        // first 6 bits of char 1
        *out++ = ENC[*in >> 2];
        if (!--inLen) {
            // last 2 bits of char 1, 4 bits of 0
            *out++ = ENC[(*in & 0x3) << 4];
            *out++ = '=';
            *out++ = '=';
            break;
        }
        // last 2 bits of char 1, first 4 bits of char 2
        *out++ = ENC[((*in & 0x3) << 4) | (*(in + 1) >> 4)];
        in++;
        if (!--inLen) {
            // last 4 bits of char 2, 2 bits of 0
            *out++ = ENC[(*in & 0xF) << 2];
            *out++ = '=';
            break;
        }
        // last 4 bits of char 2, first 2 bits of char 3
        *out++ = ENC[((*in & 0xF) << 2) | (*(in + 1) >> 6)];
        in++;
        // last 6 bits of char 3
        *out++ = ENC[*in & 0x3F];
        in++, inLen--;
    }

    return (out - original_out);
}

// HMAC-SHA-1:
//
// K - is key padded with zeros to 512 bits
// m - is message
// OPAD - 0x5c5c5c...
// IPAD - 0x363636...
//
// HMAC(K,m) = SHA1((K ^ OPAD) . SHA1((K ^ IPAD) . m))
void HMAC_SHA1(unsigned char hmac[20], const unsigned char *key, int key_len,
               const unsigned char *message, int message_len)
{
    unsigned char kopad[64], kipad[64];
    int i;
    unsigned char digest[APR_SHA1_DIGESTSIZE];
    apr_sha1_ctx_t context;
    
    if (key_len > 64) {
        key_len = 64;
    }

    for (i = 0; i < key_len; i++) {
        kopad[i] = key[i] ^ 0x5c;
        kipad[i] = key[i] ^ 0x36;
    }

    for ( ; i < 64; i++) {
        kopad[i] = 0 ^ 0x5c;
        kipad[i] = 0 ^ 0x36;
    }

    apr_sha1_init(&context);
    apr_sha1_update(&context, (const char *)kipad, 64);
    apr_sha1_update(&context, (const char *)message, (unsigned int)message_len);
    apr_sha1_final(digest, &context);

    apr_sha1_init(&context);
    apr_sha1_update(&context, (const char *)kopad, 64);
    apr_sha1_update(&context, (const char *)digest, 20);
    apr_sha1_final(hmac, &context);
}

unsigned char* aos_md5(aos_pool_t* pool, const char *in, apr_size_t in_len) {
    unsigned char* out;
    apr_md5_ctx_t context;

    //APR_MD5_DIGESTSIZE: The MD5 digest size, value is 16
    out = aos_palloc(pool, APR_MD5_DIGESTSIZE + 1);
    if (!out) {
        return NULL;
    }

    if (0 != apr_md5_init(&context)) {
        return NULL;
    }

    if (0 != apr_md5_update(&context, in, in_len)) {
        return NULL;
    }

    if (0 != apr_md5_final(out, &context)) {
        return NULL;
    }
    out[APR_MD5_DIGESTSIZE] = '\0';
    return out;
};

int aos_url_decode(const char *in, char *out)
{
    static const char tbl[256] = {
        -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
         0, 1, 2, 3, 4, 5, 6, 7,  8, 9,-1,-1,-1,-1,-1,-1,
        -1,10,11,12,13,14,15,-1, -1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
        -1,10,11,12,13,14,15,-1, -1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1
    };
    char c, v1, v2;

    if(in != NULL) {
        while((c=*in++) != '\0') {
            if(c == '%') {
                if(!(v1=*in++) || (v1=tbl[(unsigned char)v1])<0 || 
                   !(v2=*in++) || (v2=tbl[(unsigned char)v2])<0) {
                    *out = '\0';
                    return -1;
                }
                c = (v1<<4)|v2;
            } else if (c == '+') {
                c = ' ';
            }
            *out++ = c;
        }
    }
    *out = '\0';
    return 0;
}

int aos_urlsafe_base64_encode(const char *in, int inLen, char *out)
{
    int pad = 0;
    int i;
    int len;
    len = aos_base64_encode((const unsigned char *)in, inLen, out);
    //'+' -> '-'
    //'/' -> '_'
    //'=' -> ''
    for (i = 0; i < len; i++) {
        if (out[i] == '+') {
            out[i] = '-';
        } else if (out[i] == '/') {
            out[i] = '_';
        } else if (out[i] == '=') {
            out[i] = '\0'; 
            pad++; 
        }
    }
    return len - pad;
}


/*
 * Convert a string to a long long integer.
 *
 * Ignores `locale' stuff.  Assumes that the upper and lower case
 * alphabets and digits are each contiguous.
 */
long long aos_strtoll(const char *nptr, char **endptr, int base)
{
    const char *s;
    /* LONGLONG */
    long long int acc, cutoff;
    int c;
    int neg, any, cutlim;

    /* endptr may be NULL */

#ifdef __GNUC__
    /* This outrageous construct just to shut up a GCC warning. */
    (void) &acc; (void) &cutoff;
#endif

    /*
     * Skip white space and pick up leading +/- sign if any.
     * If base is 0, allow 0x for hex and 0 for octal, else
     * assume decimal; if base is already 16, allow 0x.
     */
    s = nptr;
    do {
        c = (unsigned char) *s++;
    } while (isspace(c));
    if (c == '-') {
        neg = 1;
        c = *s++;
    } else {
        neg = 0;
        if (c == '+')
            c = *s++;
    }
    if ((base == 0 || base == 16) &&
        c == '0' && (*s == 'x' || *s == 'X')) {
        c = s[1];
        s += 2;
        base = 16;
    }
    if (base == 0)
        base = c == '0' ? 8 : 10;

    /*
     * Compute the cutoff value between legal numbers and illegal
     * numbers.  That is the largest legal value, divided by the
     * base.  An input number that is greater than this value, if
     * followed by a legal input character, is too big.  One that
     * is equal to this value may be valid or not; the limit
     * between valid and invalid numbers is then based on the last
     * digit.  For instance, if the range for long longs is
     * [-9223372036854775808..9223372036854775807] and the input base
     * is 10, cutoff will be set to 922337203685477580 and cutlim to
     * either 7 (neg==0) or 8 (neg==1), meaning that if we have
     * accumulated a value > 922337203685477580, or equal but the
     * next digit is > 7 (or 8), the number is too big, and we will
     * return a range error.
     *
     * Set any if any `digits' consumed; make it negative to indicate
     * overflow.
     */
    cutoff = neg ? LLONG_MIN : LLONG_MAX;
    cutlim = (int)(cutoff % base);
    cutoff /= base;
    if (neg) {
        if (cutlim > 0) {
            cutlim -= base;
            cutoff += 1;
        }
        cutlim = -cutlim;
    }
    for (acc = 0, any = 0;; c = (unsigned char) *s++) {
        if (isdigit(c))
            c -= '0';
        else if (isalpha(c))
            c -= isupper(c) ? 'A' - 10 : 'a' - 10;
        else
            break;
        if (c >= base)
            break;
        if (any < 0)
            continue;
        if (neg) {
            if (acc < cutoff || (acc == cutoff && c > cutlim)) {
                any = -1;
                acc = LLONG_MIN;
                errno = ERANGE;
            } else {
                any = 1;
                acc *= base;
                acc -= c;
            }
        } else {
            if (acc > cutoff || (acc == cutoff && c > cutlim)) {
                any = -1;
                acc = LLONG_MAX;
                errno = ERANGE;
            } else {
                any = 1;
                acc *= base;
                acc += c;
            }
        }
    }
    if (endptr != 0)
        /* LINTED interface specification */
        *endptr = (char *)(any ? s - 1 : nptr);
    return (acc);
}

int64_t aos_atoi64(const char *nptr)
{
    return aos_strtoull(nptr, NULL, 10);
}

unsigned long long aos_strtoull(const char *nptr, char **endptr, int base)
{
    const char *s;
    unsigned long long acc, cutoff;
    int c;
    int neg, any, cutlim;

    /*
     * See strtoq for comments as to the logic used.
     */
    s = nptr;
    do {
        c = (unsigned char) *s++;
    } while (isspace(c));
    if (c == '-') {
        neg = 1;
        c = *s++;
    } else { 
        neg = 0;
        if (c == '+')
            c = *s++;
    }
    if ((base == 0 || base == 16) &&
        c == '0' && (*s == 'x' || *s == 'X')) {
        c = s[1];
        s += 2;
        base = 16;
    }
    if (base == 0)
        base = c == '0' ? 8 : 10;

    cutoff = ULLONG_MAX / (unsigned long long)base;
    cutlim = ULLONG_MAX % (unsigned long long)base;
    for (acc = 0, any = 0;; c = (unsigned char) *s++) {
        if (isdigit(c))
            c -= '0';
        else if (isalpha(c))
            c -= isupper(c) ? 'A' - 10 : 'a' - 10;
        else
            break;
        if (c >= base)
            break;
        if (any < 0)
            continue;
        if (acc > cutoff || (acc == cutoff && c > cutlim)) {
            any = -1;
            acc = ULLONG_MAX;
            errno = ERANGE;
        } else {
            any = 1;
            acc *= (unsigned long long)base;
            acc += c;
        }
    }
    if (neg && any > 0)
#ifdef WIN32
#pragma warning(disable : 4146)
#endif
        acc = -acc;
#ifdef WIN32
#pragma warning(default : 4146)
#endif
    if (endptr != 0)
        *endptr = (char *) (any ? s - 1 : nptr);
    return (acc);
}

uint64_t aos_atoui64(const char *nptr) 
{
    return aos_strtoull(nptr, NULL, 10);
}
