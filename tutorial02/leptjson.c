#include "leptjson.h"
#include <assert.h>  /* assert() */
#include <stdlib.h>  /* NULL, strtod() */
#include <string.h>
#include <math.h>
#define EXPECT(c, ch)       do { assert(*c->json == (ch)); c->json++; } while(0)

typedef struct {
    const char* json;
}lept_context;

static void lept_parse_whitespace(lept_context* c) {
    const char *p = c->json;
    while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')
        p++;
    c->json = p;
}

static int lept_parse_literal(lept_context* c, lept_value* v, const char* match, const lept_type match_type) {
    // use '\0' to check whether reach the end
    size_t i;
    EXPECT(c, match[0]);
    for (i = 0; match[i+1] ; i++)
    {
        if (c->json[i] != match[i+1])
        {
            return LEPT_PARSE_INVALID_VALUE;
        }
    }

    c->json += i;
    v->type = match_type;
    return LEPT_PARSE_OK;
}

#define ISDIGIT(ch)         ((ch) >= '0' && (ch) <= '9')
#define ISDIGIT1TO9(ch)     ((ch) >= '1' && (ch) <= '9')

static int lept_parse_number(lept_context* c, lept_value* v) {
    /* validate number */
    const char* json = c->json;
    size_t i = 0;
    // check '-'
    if (json[i] == '-') ++i;
    // check integer part
    if (json[i] == '0') {
        ++i;
    }
    else {
        if (!ISDIGIT1TO9(json[i])) return LEPT_PARSE_INVALID_VALUE;
        ++i;
        while (ISDIGIT(json[i]))  ++i;
    }
    // check decimal part
    if (json[i] == '.')
    {
        ++i;
        if (!ISDIGIT(json[i])) return LEPT_PARSE_INVALID_VALUE;
        ++i;
        while (ISDIGIT(json[i])) ++i;
    }
    // check exponent part
    if (json[i] == 'e' || json[i] == 'E')
    {
        ++i;
        if (json[i] == '+' || json[i] == '-') ++i;
        if (!ISDIGIT(json[i])) return LEPT_PARSE_INVALID_VALUE;
        ++i;
        while (ISDIGIT(json[i])) ++i;
    }
    
    errno = 0;
    v->n = strtod(c->json, NULL);
    if (errno == ERANGE && isinf(v->n)) {
        return LEPT_PARSE_NUMBER_TOO_BIG;
    }

    c->json = json + i;
    v->type = LEPT_NUMBER;
    return LEPT_PARSE_OK;
}

static int lept_parse_value(lept_context* c, lept_value* v) {
    switch (*c->json) {
        case 't':  return lept_parse_literal(c, v, "true", LEPT_TRUE);
        case 'f':  return lept_parse_literal(c, v, "false", LEPT_FALSE);
        case 'n':  return lept_parse_literal(c, v, "null", LEPT_NULL);
        default:   return lept_parse_number(c, v);
        case '\0': return LEPT_PARSE_EXPECT_VALUE;
    }
}

int lept_parse(lept_value* v, const char* json) {
    lept_context c;
    int ret;
    assert(v != NULL);
    c.json = json;
    v->type = LEPT_NULL;
    lept_parse_whitespace(&c);
    if ((ret = lept_parse_value(&c, v)) == LEPT_PARSE_OK) {
        lept_parse_whitespace(&c);
        if (*c.json != '\0') {
            v->type = LEPT_NULL;
            ret = LEPT_PARSE_ROOT_NOT_SINGULAR;
        }
    }
    return ret;
}

lept_type lept_get_type(const lept_value* v) {
    assert(v != NULL);
    return v->type;
}

double lept_get_number(const lept_value* v) {
    assert(v != NULL && v->type == LEPT_NUMBER);
    return v->n;
}
