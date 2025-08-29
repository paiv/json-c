/* paiv_json - v0.92 - https://github.com/paiv/json-c

    This is a single file holding api and implementation both.
    In one of your units, include the implementation:

    #define PAIV_JSON_IMPLEMENTATION
    #include "paiv_json.h"


    LICENSE
    Refer to the end of the file for license information.

*/

#ifndef _PAIV_JSON_INCLUDE_JSON_H
#define _PAIV_JSON_INCLUDE_JSON_H


#ifndef PVJDEF
#ifdef PAIV_JSON_STATIC
#define PVJDEF static
#else
#define PVJDEF extern
#endif
#endif


#ifndef PAIV_JSON_NUMBER_BACKEND_TYPE
#define PAIV_JSON_NUMBER_BACKEND_TYPE long long int
#endif


#include <math.h>
#include <stdio.h>


#ifdef __cplusplus
extern "C" {
#endif


typedef enum {
    JsonError_ok,
    JsonError_invalid,
    JsonError_eof,
    JsonError_not_found,
    JsonError_null,
    JsonError_type_mismatch,
    JsonError_bufsize,
    JsonError_unicode,
    JsonError_write,
} JsonError;


typedef enum {
    JsonValueType_object,
    JsonValueType_array,
    JsonValueType_number,
    JsonValueType_string,
    JsonValueType_true,
    JsonValueType_false,
    JsonValueType_null,
} JsonValueType;


typedef struct {
    FILE* _file;
    int _element_count;
    int _parser_token;
    char _parser_char;
} JSON;


PVJDEF JsonError json_reader_init(JSON* context, FILE* file);
PVJDEF JsonError json_reader_open_object(JSON* context, JSON* object);
PVJDEF JsonError json_reader_read_object(JSON* context, size_t* key_size, char* key, JsonValueType* value);
PVJDEF JsonError json_reader_open_array(JSON* context, JSON* array);
PVJDEF JsonError json_reader_read_array(JSON* context, JsonValueType* value);
PVJDEF JsonError json_reader_read_numberi(JSON* context, int* value);
PVJDEF JsonError json_reader_read_numberl(JSON* context, long* value);
PVJDEF JsonError json_reader_read_numberll(JSON* context, long long* value);
PVJDEF JsonError json_reader_read_numberf(JSON* context, float* value);
PVJDEF JsonError json_reader_read_numberd(JSON* context, double* value);
PVJDEF JsonError json_reader_read_numberld(JSON* context, long double* value);
PVJDEF JsonError json_reader_read_string(JSON* context, size_t* buf_size, char* buf);
PVJDEF JsonError json_reader_resume_string(JSON* context, size_t* buf_size, char* buf);
PVJDEF JsonError json_reader_read_bool(JSON* context, int* value);
PVJDEF JsonError json_reader_read_null(JSON* context);
PVJDEF JsonError json_reader_consume_value(JSON* context);
PVJDEF JsonError json_reader_peek_value(JSON* context, JsonValueType* value);

PVJDEF JsonError json_writer_init(JSON* context, FILE* file);
PVJDEF JsonError json_writer_open_object(JSON* context, JSON* object);
PVJDEF JsonError json_writer_close_object(JSON* object);
PVJDEF JsonError json_writer_write_object_key_separator(JSON* object);
PVJDEF JsonError json_writer_write_object_value_separator(JSON* object);
PVJDEF JsonError json_writer_open_array(JSON* context, JSON* array);
PVJDEF JsonError json_writer_close_array(JSON* array);
PVJDEF JsonError json_writer_write_array_value_separator(JSON* array);
PVJDEF JsonError json_writer_write_numberi(JSON* context, int value);
PVJDEF JsonError json_writer_write_numberl(JSON* context, long value);
PVJDEF JsonError json_writer_write_numberll(JSON* context, long long value);
PVJDEF JsonError json_writer_write_numberf(JSON* context, float value);
PVJDEF JsonError json_writer_write_numberd(JSON* context, double value);
PVJDEF JsonError json_writer_write_numberld(JSON* context, long double value);
PVJDEF JsonError json_writer_write_string(JSON* context, const char* value);
PVJDEF JsonError json_writer_write_bool(JSON* context, int value);
PVJDEF JsonError json_writer_write_null(JSON* context);


#ifdef __cplusplus
}
#endif


#endif /* _PAIV_JSON_INCLUDE_JSON_H */


#ifdef PAIV_JSON_IMPLEMENTATION


typedef enum {
    _TokenType_invalid,
    _TokenType_bool_false,
    _TokenType_bool_true,
    _TokenType_array_open,
    _TokenType_array_close,
    _TokenType_object_open,
    _TokenType_object_close,
    _TokenType_key_separator,
    _TokenType_value_separator,
    _TokenType_string_open,
    _TokenType_string_close,
    _TokenType_number,
    _TokenType_null_value,
} _TokenType;


static void
_json_parser_init(JSON* state) {
    state->_parser_token = _TokenType_invalid;
}


static JsonError
_json_parser_read_token(JSON* state, FILE* fp, _TokenType* token) {
    for (;;) {
        int c = fgetc(fp);
        switch (c) {
            case 0x09:
            case 0x0A:
            case 0x0D:
            case 0x20:
                break;
            case EOF:
                return JsonError_eof;
            default: {
                state->_parser_char = c;
                _TokenType t = _TokenType_invalid;
                switch (c) {
                    case '{':
                        t = _TokenType_object_open;
                        break;
                    case '}':
                        t = _TokenType_object_close;
                        break;
                    case '[':
                        t = _TokenType_array_open;
                        break;
                    case ']':
                        t = _TokenType_array_close;
                        break;
                    case ':':
                        t = _TokenType_key_separator;
                        break;
                    case ',':
                        t = _TokenType_value_separator;
                        break;
                    case '-':
                    case '0' ... '9':
                        t = _TokenType_number;
                        break;
                    case '"':
                        t = _TokenType_string_open;
                        break;
                    case 't':
                        if (
                            fgetc(fp) == 'r' &&
                            fgetc(fp) == 'u' &&
                            fgetc(fp) == 'e'
                            ) {
                            t = _TokenType_bool_true;
                        }
                        break;
                    case 'f':
                        if (
                            fgetc(fp) == 'a' &&
                            fgetc(fp) == 'l' &&
                            fgetc(fp) == 's' &&
                            fgetc(fp) == 'e'
                            ) {
                            t = _TokenType_bool_false;
                        }
                        break;
                    case 'n':
                        if (
                            fgetc(fp) == 'u' &&
                            fgetc(fp) == 'l' &&
                            fgetc(fp) == 'l'
                            ) {
                            t = _TokenType_null_value;
                        }
                        break;
                    default:
                        t = _TokenType_invalid;
                        break;
                }
                state->_parser_token = t;
                *token = t;
                return t == _TokenType_invalid ? JsonError_invalid : JsonError_ok;
            }
        }
    }
}


static JsonError
_json_parser_peek_token(JSON* state, FILE* fp, _TokenType* token) {
    for (;;) {
        int c = fgetc(fp);
        switch (c) {
            case 0x09:
            case 0x0A:
            case 0x0D:
            case 0x20:
                break;
            case EOF:
                return JsonError_eof;
            default: {
                ungetc(c, fp);
                _TokenType t = _TokenType_invalid;
                switch (c) {
                    case '{':
                        t = _TokenType_object_open;
                        break;
                    case '}':
                        t = _TokenType_object_close;
                        break;
                    case '[':
                        t = _TokenType_array_open;
                        break;
                    case ']':
                        t = _TokenType_array_close;
                        break;
                    case ':':
                        t = _TokenType_key_separator;
                        break;
                    case ',':
                        t = _TokenType_value_separator;
                        break;
                    case '-':
                    case '0' ... '9':
                        t = _TokenType_number;
                        break;
                    case '"':
                        t = _TokenType_string_open;
                        break;
                    case 't':
                        t = _TokenType_bool_true;
                        break;
                    case 'f':
                        t = _TokenType_bool_false;
                        break;
                    case 'n':
                        t = _TokenType_null_value;
                        break;
                    default:
                        break;
                }
                *token = t;
                return t == _TokenType_invalid ? JsonError_invalid : JsonError_ok;
            }
        }
    }
}


static JsonError
_json_parser_read_string(JSON* _, FILE* fp, size_t* buf_size, char* buf) {
    size_t capacity = *buf_size;
    size_t count = 0;
    int state = 0;
    for (;;) {
        int c = fgetc(fp);
        if (c == EOF) {
            return JsonError_eof;
        }
        switch (state) {
            case 0:
                switch (c) {
                    case '"':
                        if (count + 1 <= capacity) {
                            *buf++ = '\0';
                            *buf_size = count;
                            return JsonError_ok;
                        }
                        else {
                            ungetc(c, fp);
                            *buf_size = count;
                            return JsonError_bufsize;
                        }
                        break;
                    case '\\':
                        state = 1;
                        break;
                    case 0x00 ... 0x1F:
                        return JsonError_invalid;
                    case 0x20:
                    case 0x21:
                    case 0x23 ... 0x5B:
                    case 0x5D ... 0xFF:
                        if (count + 1 < capacity) {
                            *buf++ = c;
                            count++;
                        }
                        else {
                            ungetc(c, fp);
                            *buf_size = count;
                            return JsonError_bufsize;
                        }
                        break;
                    default:
                        return JsonError_invalid;
                }
                break;
            case 1:
                if (c == 'u') {
                    state = 2;
                }
                else {
                    switch (c) {
                        case '"':
                        case '\\':
                        case '/':
                            break;
                        case 'b':
                            c = '\b';
                            break;
                        case 'f':
                            c = '\f';
                            break;
                        case 'n':
                            c = '\n';
                            break;
                        case 'r':
                            c = '\r';
                            break;
                        case 't':
                            c = '\t';
                            break;
                        default:
                            return JsonError_invalid;
                    }
                    if (count + 1 < capacity) {
                        *buf++ = c;
                        count++;
                        state = 0;
                    }
                    else {
                        fseek(fp, -2, SEEK_CUR);
                        *buf_size = count;
                        return JsonError_bufsize;
                    }
                }
                break;
            case 2: {
                int value = 0;
                size_t i;
                for (i = 0; i < 4; ++i) {
                    if (i != 0) {
                        c = fgetc(fp);
                    }
                    if (c == EOF) {
                        return JsonError_eof;
                    }
                    int x;
                    switch (c) {
                        case '0': x = 0; break;
                        case '1': x = 1; break;
                        case '2': x = 2; break;
                        case '3': x = 3; break;
                        case '4': x = 4; break;
                        case '5': x = 5; break;
                        case '6': x = 6; break;
                        case '7': x = 7; break;
                        case '8': x = 8; break;
                        case '9': x = 9; break;
                        case 'A': x = 10; break;
                        case 'B': x = 11; break;
                        case 'C': x = 12; break;
                        case 'D': x = 13; break;
                        case 'E': x = 14; break;
                        case 'F': x = 15; break;
                        case 'a': x = 10; break;
                        case 'b': x = 11; break;
                        case 'c': x = 12; break;
                        case 'd': x = 13; break;
                        case 'e': x = 14; break;
                        case 'f': x = 15; break;
                        default:
                            return JsonError_invalid;
                    }
                    value = (value << 4) | x;
                }
                if (value < 0 || value > 0xFF) {
                    return JsonError_unicode;
                }
                c = value & 0xFF;
                if (count + 1 < capacity) {
                    *buf++ = c;
                    count++;
                    state = 0;
                }
                else {
                    fseek(fp, -6, SEEK_CUR);
                    *buf_size = count;
                    return JsonError_bufsize;
                }
                }
                break;
        }
    }
}


static JsonError
_json_parser_consume_string(JSON* _, FILE* fp) {
    int state = 0;
    for (;;) {
        int c = fgetc(fp);
        if (c == EOF) {
            return JsonError_eof;
        }
        switch (state) {
            case 0:
                switch (c) {
                    case '"':
                        return JsonError_ok;
                    case '\\':
                        state = 1;
                        break;
                    case 0x00 ... 0x1F:
                        return JsonError_invalid;
                    case 0x20:
                    case 0x21:
                    case 0x23 ... 0x5B:
                    case 0x5D ... 0xFF:
                        break;
                    default:
                        return JsonError_invalid;
                }
                break;
            case 1:
                if (c == 'u') {
                    state = 2;
                }
                else {
                    switch (c) {
                        case '"':
                        case '\\':
                        case '/':
                            break;
                        case 'b':
                            c = '\b';
                            break;
                        case 'f':
                            c = '\f';
                            break;
                        case 'n':
                            c = '\n';
                            break;
                        case 'r':
                            c = '\r';
                            break;
                        case 't':
                            c = '\t';
                            break;
                        default:
                            return JsonError_invalid;
                    }
                    state = 0;
                }
                break;
            case 2: {
                size_t i;
                for (i = 0; i < 4; ++i) {
                    if (i != 0) {
                        c = fgetc(fp);
                    }
                    if (c == EOF) {
                        return JsonError_eof;
                    }
                    switch (c) {
                        case '0' ... '9':
                        case 'A' ... 'F':
                        case 'a' ... 'f':
                            break;
                        default:
                            return JsonError_invalid;
                    }
                }
                state = 0;
                break;
            }
        }
    }
}


static JsonError
_json_parser_read_number(JSON* context, FILE* fp, PAIV_JSON_NUMBER_BACKEND_TYPE* value, PAIV_JSON_NUMBER_BACKEND_TYPE* exponent) {
    PAIV_JSON_NUMBER_BACKEND_TYPE x = 0;
    PAIV_JSON_NUMBER_BACKEND_TYPE y = 0;
    int sign = 1;
    int msign = 1;
    int decs = 0;
    int state = 0;
    char c;
    if (context->_parser_token == _TokenType_number) {
        c = context->_parser_char;
        context->_parser_token = _TokenType_invalid;
    }
    else {
        c = fgetc(fp);
    }
    for (;; c = fgetc(fp)) {
        switch (state) {
            case 0:
                switch (c) {
                    case '-':
                        sign = -1;
                        state = 1;
                        break;
                    case '0':
                        state = 3;
                        break;
                    case '1' ... '9':
                        switch (c) {
                            case '1': x = 1; break;
                            case '2': x = 2; break;
                            case '3': x = 3; break;
                            case '4': x = 4; break;
                            case '5': x = 5; break;
                            case '6': x = 6; break;
                            case '7': x = 7; break;
                            case '8': x = 8; break;
                            case '9': x = 9; break;
                        }
                        state = 2;
                        break;
                    case EOF:
                        return JsonError_eof;
                    default:
                        return JsonError_invalid;
                }
                break;
            case 1:
                switch (c) {
                    case '0':
                        state = 3;
                        break;
                    case '1' ... '9':
                        switch (c) {
                            case '1': x = 1; break;
                            case '2': x = 2; break;
                            case '3': x = 3; break;
                            case '4': x = 4; break;
                            case '5': x = 5; break;
                            case '6': x = 6; break;
                            case '7': x = 7; break;
                            case '8': x = 8; break;
                            case '9': x = 9; break;
                        }
                        state = 2;
                        break;
                    case EOF:
                        return JsonError_eof;
                    default:
                        return JsonError_invalid;
                }
                break;
            case 2:
                switch (c) {
                    case '0' ... '9':
                        switch (c) {
                            case '0': x = x * 10 + 0; break;
                            case '1': x = x * 10 + 1; break;
                            case '2': x = x * 10 + 2; break;
                            case '3': x = x * 10 + 3; break;
                            case '4': x = x * 10 + 4; break;
                            case '5': x = x * 10 + 5; break;
                            case '6': x = x * 10 + 6; break;
                            case '7': x = x * 10 + 7; break;
                            case '8': x = x * 10 + 8; break;
                            case '9': x = x * 10 + 9; break;
                        }
                        break;
                    case '.':
                        state = 5;
                        break;
                    case 'e':
                    case 'E':
                        state = 7;
                        break;
                    case EOF:
                        *value = x * sign;
                        *exponent = 0;
                        return JsonError_ok;
                    default:
                        ungetc(c, fp);
                        *value = x * sign;
                        *exponent = 0;
                        return JsonError_ok;
                }
                break;
            case 3:
                switch (c) {
                    case '.':
                        state = 5;
                        break;
                    case 'E':
                    case 'e':
                        state = 7;
                        break;
                    case EOF:
                        *value = x * sign;
                        *exponent = 0;
                        return JsonError_ok;
                    default:
                        ungetc(c, fp);
                        *value = x * sign;
                        *exponent = 0;
                        return JsonError_ok;
                }
                break;
            case 5:
                switch (c) {
                    case '0' ... '9':
                        decs++;
                        switch (c) {
                            case '0': x = x * 10 + 0; break;
                            case '1': x = x * 10 + 1; break;
                            case '2': x = x * 10 + 2; break;
                            case '3': x = x * 10 + 3; break;
                            case '4': x = x * 10 + 4; break;
                            case '5': x = x * 10 + 5; break;
                            case '6': x = x * 10 + 6; break;
                            case '7': x = x * 10 + 7; break;
                            case '8': x = x * 10 + 8; break;
                            case '9': x = x * 10 + 9; break;
                        }
                        state = 6;
                        break;
                    case EOF:
                        return JsonError_eof;
                    default:
                        return JsonError_invalid;
                }
                break;
            case 6:
                switch (c) {
                    case '0' ... '9':
                        decs++;
                        switch (c) {
                            case '0': x = x * 10 + 0; break;
                            case '1': x = x * 10 + 1; break;
                            case '2': x = x * 10 + 2; break;
                            case '3': x = x * 10 + 3; break;
                            case '4': x = x * 10 + 4; break;
                            case '5': x = x * 10 + 5; break;
                            case '6': x = x * 10 + 6; break;
                            case '7': x = x * 10 + 7; break;
                            case '8': x = x * 10 + 8; break;
                            case '9': x = x * 10 + 9; break;
                        }
                        break;
                    case 'E':
                    case 'e':
                        state = 7;
                        break;
                    case EOF:
                        *value = x * sign;
                        *exponent = -decs;
                        return JsonError_ok;
                    default:
                        ungetc(c, fp);
                        *value = x * sign;
                        *exponent = -decs;
                        return JsonError_ok;
                }
                break;
            case 7:
                switch (c) {
                    case '-':
                        msign = -1;
                        state = 8;
                        break;
                    case '+':
                        msign = 1;
                        state = 8;
                        break;
                    case '0' ... '9':
                        switch (c) {
                            case '0': y = 0; break;
                            case '1': y = 1; break;
                            case '2': y = 2; break;
                            case '3': y = 3; break;
                            case '4': y = 4; break;
                            case '5': y = 5; break;
                            case '6': y = 6; break;
                            case '7': y = 7; break;
                            case '8': y = 8; break;
                            case '9': y = 9; break;
                        }
                        state = 8;
                        break;
                    case EOF:
                        return JsonError_eof;
                    default:
                        return JsonError_invalid;
                }
                break;
            case 8:
                switch (c) {
                    case '0': y = y * 10 + 0; break;
                    case '1': y = y * 10 + 1; break;
                    case '2': y = y * 10 + 2; break;
                    case '3': y = y * 10 + 3; break;
                    case '4': y = y * 10 + 4; break;
                    case '5': y = y * 10 + 5; break;
                    case '6': y = y * 10 + 6; break;
                    case '7': y = y * 10 + 7; break;
                    case '8': y = y * 10 + 8; break;
                    case '9': y = y * 10 + 9; break;
                    case EOF:
                        *value = x * sign;
                        *exponent = y * msign - decs;
                        return JsonError_ok;
                    default:
                        ungetc(c, fp);
                        *value = x * sign;
                        *exponent = y * msign - decs;
                        return JsonError_ok;
                }
                break;
        }
    }
}


PVJDEF JsonError
json_reader_init(JSON* state, FILE* file) {
    state->_file = file;
    state->_element_count = 0;
    _json_parser_init(state);
    return JsonError_ok;
}


PVJDEF JsonError
json_reader_open_object(JSON* state, JSON* object) {
    _TokenType token;
    JsonError err = _json_parser_read_token(state, state->_file, &token);
    if (err != JsonError_ok) {
        return err;
    }
    switch (token) {
        case _TokenType_object_open:
            break;
        case _TokenType_null_value:
            return JsonError_null;
        default:
            return JsonError_type_mismatch;
    }
    err = json_reader_init(object, state->_file);
    return err;
}


PVJDEF JsonError
json_reader_read_object(JSON* state, size_t* key_size, char* key, JsonValueType* value) {
    _TokenType token;
    if (state->_element_count != 0) {
        JsonError err = _json_parser_read_token(state, state->_file, &token);
        if (err != JsonError_ok) { return err; }
        switch (token) {
            case _TokenType_object_close:
                return JsonError_not_found;
            case _TokenType_value_separator:
                break;
            default:
                return JsonError_invalid;
        }
    }
    JsonError err = _json_parser_read_token(state, state->_file, &token);
    if (err != JsonError_ok) { return err; }
    switch (token) {
        case _TokenType_string_open:
            break;
        case _TokenType_object_close:
            if (state->_element_count != 0) {
                return JsonError_invalid;
            }
            return JsonError_not_found;
        default:
            return JsonError_invalid;
    }
    err = _json_parser_read_string(state, state->_file, key_size, key);
    if (err != JsonError_ok) {
        return err;
    }
    err = _json_parser_read_token(state, state->_file, &token);
    if (err != JsonError_ok) {
        return err;
    }
    switch (token) {
        case _TokenType_key_separator:
            break;
        default:
            return JsonError_invalid;
    }
    state->_element_count++;
    if (value) {
        err = json_reader_peek_value(state, value);
        return err;
    }
    return JsonError_ok;
}


PVJDEF JsonError
json_reader_open_array(JSON* state, JSON* array) {
    _TokenType token;
    JsonError err = _json_parser_read_token(state, state->_file, &token);
    if (err != JsonError_ok) {
        return err;
    }
    switch (token) {
        case _TokenType_array_open:
            break;
        case _TokenType_null_value:
            return JsonError_null;
        default:
            return JsonError_type_mismatch;
    }
    err = json_reader_init(array, state->_file);
    return err;
}


PVJDEF JsonError
json_reader_read_array(JSON* state, JsonValueType* value) {
    _TokenType token;
    if (state->_element_count != 0) {
        JsonError err = _json_parser_read_token(state, state->_file, &token);
        if (err != JsonError_ok) { return err; }
        switch (token) {
            case _TokenType_array_close:
                return JsonError_not_found;
            case _TokenType_value_separator:
                break;
            default:
                return JsonError_invalid;
        }
    }
    JsonError err = _json_parser_peek_token(state, state->_file, &token);
    if (err != JsonError_ok) { return err; }
    switch (token) {
        case _TokenType_bool_false:
            if (value) { *value = JsonValueType_false; }
            break;
        case _TokenType_bool_true:
            if (value) { *value = JsonValueType_true; }
            break;
        case _TokenType_array_open:
            if (value) { *value = JsonValueType_array; }
            break;
        case _TokenType_object_open:
            if (value) { *value = JsonValueType_object; }
            break;
        case _TokenType_string_open:
            if (value) { *value = JsonValueType_string; }
            break;
        case _TokenType_number:
            if (value) { *value = JsonValueType_number; }
            break;
        case _TokenType_null_value:
            if (value) { *value = JsonValueType_null; }
            break;
        case _TokenType_array_close:
            if (state->_element_count != 0) {
                return JsonError_invalid;
            }
            _json_parser_read_token(state, state->_file, &token);
            return JsonError_not_found;
        case _TokenType_object_close:
        case _TokenType_key_separator:
        case _TokenType_value_separator:
        case _TokenType_string_close:
        case _TokenType_invalid:
            return JsonError_invalid;
    }
    state->_element_count++;
    return JsonError_ok;
}


static JsonError
_json_reader_read_number(JSON* state, PAIV_JSON_NUMBER_BACKEND_TYPE* value, PAIV_JSON_NUMBER_BACKEND_TYPE* exponent) {
    _TokenType token;
    JsonError err = _json_parser_read_token(state, state->_file, &token);
    if (err != JsonError_ok) {
        return err;
    }
    switch (token) {
        case _TokenType_number:
            break;
        case _TokenType_null_value:
            return JsonError_null;
        default:
            return JsonError_type_mismatch;
    }
    err = _json_parser_read_number(state, state->_file, value, exponent);
    return err;
}


PVJDEF JsonError
json_reader_read_numberi(JSON* state, int* value) {
    PAIV_JSON_NUMBER_BACKEND_TYPE signi, power;
    JsonError err = _json_reader_read_number(state, &signi, &power);
    if (err != JsonError_ok) { return err; }
    if (power == 0) {
        *value = signi;
    }
    else {
        *value = signi * pow(10, power);
    }
    return JsonError_ok;
}


PVJDEF JsonError
json_reader_read_numberl(JSON* state, long* value) {
    PAIV_JSON_NUMBER_BACKEND_TYPE signi, power;
    JsonError err = _json_reader_read_number(state, &signi, &power);
    if (err != JsonError_ok) { return err; }
    if (power == 0) {
        *value = signi;
    }
    else {
        *value = signi * pow(10, power);
    }
    return JsonError_ok;
}


PVJDEF JsonError
json_reader_read_numberll(JSON* state, long long* value) {
    PAIV_JSON_NUMBER_BACKEND_TYPE signi, power;
    JsonError err = _json_reader_read_number(state, &signi, &power);
    if (err != JsonError_ok) { return err; }
    if (power == 0) {
        *value = signi;
    }
    else {
        *value = signi * powl(10, power);
    }
    return JsonError_ok;
}

PVJDEF JsonError
json_reader_read_numberf(JSON* state, float* value) {
    PAIV_JSON_NUMBER_BACKEND_TYPE signi, power;
    JsonError err = _json_reader_read_number(state, &signi, &power);
    if (err != JsonError_ok) { return err; }
    *value = signi * powf(10, power);
    return JsonError_ok;
}


PVJDEF JsonError
json_reader_read_numberd(JSON* state, double* value) {
    PAIV_JSON_NUMBER_BACKEND_TYPE signi, power;
    JsonError err = _json_reader_read_number(state, &signi, &power);
    if (err != JsonError_ok) { return err; }
    *value = signi * pow(10, power);
    return JsonError_ok;
}


PVJDEF JsonError
json_reader_read_numberld(JSON* state, long double* value) {
    PAIV_JSON_NUMBER_BACKEND_TYPE signi, power;
    JsonError err = _json_reader_read_number(state, &signi, &power);
    if (err != JsonError_ok) { return err; }
    *value = signi * powl(10, power);
    return JsonError_ok;
}


PVJDEF JsonError
json_reader_read_string(JSON* state, size_t* buf_size, char* buf) {
    _TokenType token;
    JsonError err = _json_parser_read_token(state, state->_file, &token);
    if (err != JsonError_ok) {
        return err;
    }
    switch (token) {
        case _TokenType_string_open:
            break;
        case _TokenType_null_value:
            return JsonError_null;
        default:
            return JsonError_type_mismatch;
    }
    err = _json_parser_read_string(state, state->_file, buf_size, buf);
    return err;
}


PVJDEF JsonError
json_reader_resume_string(JSON* state, size_t* buf_size, char* buf) {
    JsonError err = _json_parser_read_string(state, state->_file, buf_size, buf);
    return err;
}


PVJDEF JsonError
json_reader_read_bool(JSON* state, int* value) {
    _TokenType token;
    JsonError err = _json_parser_read_token(state, state->_file, &token);
    if (err != JsonError_ok) {
        return err;
    }
    switch (token) {
        case _TokenType_bool_false:
            *value = 0;
            break;
        case _TokenType_bool_true:
            *value = 1;
            break;
        case _TokenType_null_value:
            return JsonError_null;
        case _TokenType_array_open:
        case _TokenType_object_open:
        case _TokenType_string_open:
        case _TokenType_number:
            return JsonError_type_mismatch;
        default:
            return JsonError_invalid;
    }
    return JsonError_ok;
}


PVJDEF JsonError
json_reader_read_null(JSON* state) {
    _TokenType token;
    JsonError err = _json_parser_read_token(state, state->_file, &token);
    if (err != JsonError_ok) {
        return err;
    }
    switch (token) {
        case _TokenType_null_value:
            break;
        case _TokenType_bool_false:
        case _TokenType_bool_true:
        case _TokenType_array_open:
        case _TokenType_object_open:
        case _TokenType_string_open:
        case _TokenType_number:
            return JsonError_type_mismatch;
        default:
            return JsonError_invalid;
    }
    return JsonError_ok;
}


static JsonError _json_reader_consume_array(JSON* state);
static JsonError _json_reader_consume_object(JSON* state);


PVJDEF JsonError
json_reader_consume_value(JSON* state) {
    _TokenType token;
    JsonError err = _json_parser_peek_token(state, state->_file, &token);
    if (err != JsonError_ok) {
        return err;
    }
    switch (token) {
        case _TokenType_null_value:
        case _TokenType_bool_false:
        case _TokenType_bool_true:
            err = _json_parser_read_token(state, state->_file, &token);
            return err;
        case _TokenType_string_open: {
            err = _json_parser_read_token(state, state->_file, &token);
            if (err != JsonError_ok) { return err; }
            err = _json_parser_consume_string(state, state->_file);
            return err;
            }
        case _TokenType_number: {
            long long value, exponent;
            err = _json_parser_read_number(state, state->_file, &value, &exponent);
            return err;
            }
        case _TokenType_array_open:
            err = _json_reader_consume_array(state);
            return err;
        case _TokenType_object_open:
            err = _json_reader_consume_object(state);
            return err;
        default:
            return JsonError_invalid;
    }
}


static JsonError
_json_reader_consume_array(JSON* state) {
    JSON array;
    JsonError err = json_reader_open_array(state, &array);
    if (err != JsonError_ok) { return err; }
    for (;;) {
        err = json_reader_read_array(&array, NULL);
        if (err == JsonError_not_found) { break; }
        if (err != JsonError_ok) { return err; }
        err = json_reader_consume_value(&array);
        if (err != JsonError_ok) { return err; }
    }
    return JsonError_ok;
}


static JsonError
_json_reader_consume_object_key(JSON* state) {
    _TokenType token;
    if (state->_element_count != 0) {
        JsonError err = _json_parser_read_token(state, state->_file, &token);
        if (err != JsonError_ok) { return err; }
        switch (token) {
            case _TokenType_object_close:
                return JsonError_not_found;
            case _TokenType_value_separator:
                break;
            default:
                return JsonError_invalid;
        }
    }
    JsonError err = _json_parser_read_token(state, state->_file, &token);
    if (err != JsonError_ok) { return err; }
    switch (token) {
        case _TokenType_string_open:
            break;
        case _TokenType_object_close:
            if (state->_element_count != 0) {
                return JsonError_invalid;
            }
            return JsonError_not_found;
        default:
            return JsonError_invalid;
    }
    err = _json_parser_consume_string(state, state->_file);
    if (err != JsonError_ok) {
        return err;
    }
    err = _json_parser_read_token(state, state->_file, &token);
    if (err != JsonError_ok) {
        return err;
    }
    switch (token) {
        case _TokenType_key_separator:
            break;
        default:
            return JsonError_invalid;
    }
    state->_element_count++;
    return JsonError_ok;
}


static JsonError
_json_reader_consume_object(JSON* state) {
    JSON object;
    JsonError err = json_reader_open_object(state, &object);
    if (err != JsonError_ok) { return err; }
    for (;;) {
        err = _json_reader_consume_object_key(&object);
        if (err == JsonError_not_found) { break; }
        if (err != JsonError_ok) { return err; }
        err = json_reader_consume_value(&object);
        if (err != JsonError_ok) { return err; }
    }
    return JsonError_ok;
}


PVJDEF JsonError
json_reader_peek_value(JSON* state, JsonValueType* value) {
    _TokenType token;
    JsonError err = _json_parser_peek_token(state, state->_file, &token);
    if (err != JsonError_ok) { return err; }
    switch (token) {
        case _TokenType_bool_false:
            *value = JsonValueType_false;
            break;
        case _TokenType_bool_true:
            *value = JsonValueType_true;
            break;
        case _TokenType_array_open:
            *value = JsonValueType_array;
            break;
        case _TokenType_object_open:
            *value = JsonValueType_object;
            break;
        case _TokenType_string_open:
            *value = JsonValueType_string;
            break;
        case _TokenType_number:
            *value = JsonValueType_number;
            break;
        case _TokenType_null_value:
            *value = JsonValueType_null;
            break;
        default:
            return JsonError_invalid;
    }
    return JsonError_ok;
}


PVJDEF JsonError
json_writer_init(JSON* state, FILE* file) {
    state->_file = file;
    state->_element_count = 0;
    return JsonError_ok;
}


PVJDEF JsonError
json_writer_open_object(JSON* state, JSON* object) {
    FILE* fp = state->_file;
    if (fputc('{', fp) == EOF) { return JsonError_write; }
    JsonError err = json_writer_init(object, fp);
    return err;
}


PVJDEF JsonError
json_writer_close_object(JSON* state) {
    FILE* fp = state->_file;
    if (fputc('}', fp) == EOF) { return JsonError_write; }
    return JsonError_ok;
}


PVJDEF JsonError
json_writer_write_object_key_separator(JSON* state) {
    FILE* fp = state->_file;
    if (fputc(':', fp) == EOF) { return JsonError_write; }
    return JsonError_ok;
}


PVJDEF JsonError
json_writer_write_object_value_separator(JSON* state) {
    if (state->_element_count++ != 0) {
        FILE* fp = state->_file;
        if (fputc(',', fp) == EOF) { return JsonError_write; }
    }
    return JsonError_ok;
}


PVJDEF JsonError
json_writer_open_array(JSON* state, JSON* array) {
    FILE* fp = state->_file;
    if (fputc('[', fp) == EOF) { return JsonError_write; }
    JsonError err = json_writer_init(array, fp);
    return err;
}


PVJDEF JsonError
json_writer_close_array(JSON* state) {
    FILE* fp = state->_file;
    if (fputc(']', fp) == EOF) { return JsonError_write; }
    return JsonError_ok;
}


PVJDEF JsonError
json_writer_write_array_value_separator(JSON* state) {
    if (state->_element_count++ != 0) {
        FILE* fp = state->_file;
        if (fputc(',', fp) == EOF) { return JsonError_write; }
    }
    return JsonError_ok;
}


PVJDEF JsonError
json_writer_write_numberi(JSON* state, int value) {
    int n = fprintf(state->_file, "%d", value);
    if (n < 0) { return JsonError_write; }
    return JsonError_ok;
}


PVJDEF JsonError
json_writer_write_numberl(JSON* state, long value) {
    int n = fprintf(state->_file, "%ld", value);
    if (n < 0) { return JsonError_write; }
    return JsonError_ok;
}


PVJDEF JsonError
json_writer_write_numberll(JSON* state, long long value) {
    int n = fprintf(state->_file, "%lld", value);
    if (n < 0) { return JsonError_write; }
    return JsonError_ok;
}


PVJDEF JsonError
json_writer_write_numberf(JSON* state, float value) {
    int n = fprintf(state->_file, "%.7g", value);
    if (n < 0) { return JsonError_write; }
    return JsonError_ok;
}


PVJDEF JsonError
json_writer_write_numberd(JSON* state, double value) {
    int n = fprintf(state->_file, "%.16g", value);
    if (n < 0) { return JsonError_write; }
    return JsonError_ok;
}


PVJDEF JsonError
json_writer_write_numberld(JSON* state, long double value) {
    int n = fprintf(state->_file, "%.34Lg", value);
    if (n < 0) { return JsonError_write; }
    return JsonError_ok;
}


PVJDEF JsonError
json_writer_write_string(JSON* state, const char* value) {
    FILE* fp = state->_file;
    const char* p = value;
    if (fputc('"', fp) == EOF) { return JsonError_write; }
    for (;; ++p) {
        char c = *p;
        if (c == '\0') { break; }
        switch (c) {
            case '\b':
                fputc('\\', fp);
                fputc('b', fp);
                break;
            case '\t':
                fputc('\\', fp);
                fputc('t', fp);
                break;
            case '\n':
                fputc('\\', fp);
                fputc('n', fp);
                break;
            case '\f':
                fputc('\\', fp);
                fputc('f', fp);
                break;
            case '\r':
                fputc('\\', fp);
                fputc('r', fp);
                break;
            case '"':
                fputc('\\', fp);
                fputc('"', fp);
                break;
            case '\\':
                fputc('\\', fp);
                fputc('\\', fp);
                break;
            default:
                fputc(c, fp);
                break;
        }
    }
    if (fputc('"', fp) == EOF) { return JsonError_write; }
    return JsonError_ok;
}


PVJDEF JsonError
json_writer_write_bool(JSON* state, int value) {
    size_t n;
    if (value == 0) {
        n = fwrite("false", 5, 1, state->_file);
    }
    else {
        n = fwrite("true", 4, 1, state->_file);
    }
    if (n != 1) { return JsonError_write; }
    return JsonError_ok;
}


PVJDEF JsonError
json_writer_write_null(JSON* state) {
    size_t n = fwrite("null", 4, 1, state->_file);
    if (n != 1) { return JsonError_write; }
    return JsonError_ok;
}


#endif /* PAIV_JSON_IMPLEMENTATION */


/*
MIT License Copyright (c) 2024 Pavel Ivashkov https://github.com/paiv

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

