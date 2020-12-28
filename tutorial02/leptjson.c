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
    assert(match != NULL);
    size_t sz = strlen(match);
    size_t i = 0;
    EXPECT(c, match[i++]);
    while (i < sz)
    {
        if (c->json[i - 1] != match[i])
        {
            return LEPT_PARSE_INVALID_VALUE;
        }
        ++i;
    }
    c->json += sz - 1;
    v->type = match_type;
    return LEPT_PARSE_OK;
}

static int lept_parse_true(lept_context* c, lept_value* v) {
    return lept_parse_literal(c, v, "true", LEPT_TRUE);
}

static int lept_parse_false(lept_context* c, lept_value* v) {
    return lept_parse_literal(c, v, "false", LEPT_FALSE);
}

static int lept_parse_null(lept_context* c, lept_value* v) {
    return lept_parse_literal(c, v, "null", LEPT_NULL);
}

#define ISDIGIT(ch)         ((ch) >= '0' && (ch) <= '9')
#define ISDIGIT1TO9(ch)     ((ch) >= '1' && (ch) <= '9')

static int lept_parse_number(lept_context* c, lept_value* v) {
    char* end;
    /* \TODO validate number */
    const char* json = c->json;
    size_t i = 0;
    // check first character
    if (ISDIGIT(json[i]) || json[i] == '-')
    {
        // check '-'
        if (json[i] == '-') ++i;
        // check 0 and 1-9
        if (json[i] == '0') {
            ++i;
        }
        else {
            ++i;
            while (ISDIGIT(json[i]))
            {
                ++i;
            }
        }
        if (json[i] == '.')
        {
            ++i;
            size_t nums_after_dot = 0;
            while (ISDIGIT(json[i]))
            {
                ++i;
                ++nums_after_dot;
            }
            if (nums_after_dot == 0)
            {
                return LEPT_PARSE_INVALID_VALUE;
            }
        }
        if (json[i] == 'e' || json[i] == 'E')
        {
            ++i;
            if (json[i] == '+' || json[i] == '-')
            {
                ++i;
            }
            size_t nums_after_exp = 0;
            while (ISDIGIT(json[i]))
            {
                ++i;
                ++nums_after_exp;
            }
            if (nums_after_exp == 0)
            {
                return LEPT_PARSE_INVALID_VALUE;
            }

        }
        if (json[i] != '\0')
        {
            return LEPT_PARSE_ROOT_NOT_SINGULAR;
        }
    }
    else {
        return LEPT_PARSE_INVALID_VALUE;
    }

    v->n = strtod(c->json, &end);
    if (c->json == end)
        return LEPT_PARSE_INVALID_VALUE;
    if (isinf(v->n)) {
        return LEPT_PARSE_NUMBER_TOO_BIG;
    }
    c->json = end;
    v->type = LEPT_NUMBER;
    return LEPT_PARSE_OK;
}

static int lept_parse_value(lept_context* c, lept_value* v) {
    switch (*c->json) {
        case 't':  return lept_parse_true(c, v);
        case 'f':  return lept_parse_false(c, v);
        case 'n':  return lept_parse_null(c, v);
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
