#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define PAIV_JSON_IMPLEMENTATION
#include "paiv_json.h"

typedef size_t sz;
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
typedef float r32;
typedef double r64;
typedef const char cs;


static void
fatal_error(cs* message=nullptr) {
    if (message != nullptr) {
        fprintf(stderr, "! %s\n", message);
    }
    abort();
}


static void
fatal_perror(cs* context=nullptr) {
    perror(context);
    abort();
}

static void
assert(u8 condition) {
    if (condition == 0) {
        fatal_error();
    }
}


static void
write_file(cs* filename, cs* data) {
    FILE* fp = fopen(filename, "w");
    if (fp == nullptr) { fatal_perror(filename); }
    fwrite(data, strlen(data), 1, fp);
    fclose(fp);
}


template<class Worker>
static void
test_reader(cs* filename, cs* data, Worker worker) {
    // write_file(filename, data);
    // FILE* fp = fopen(filename, "r");
    FILE* fp = fmemopen((char*)data, strlen(data), "r");
    if (fp == nullptr) { fatal_perror(filename); }
    
    JSON json;
    JsonError err = json_reader_init(&json, fp);
    assert(err == JsonError_ok);

    worker(&json);

    fclose(fp);
}


static void
test1_hello() {
    cs* data = R"(
    "hello, world"
    )";
    test_reader("test1.json", data, [] (JSON* json) {
        char buf[100];
        sz nbuf = sizeof(buf);
        JsonError err = json_reader_read_string(json, &nbuf, buf);
        assert(err == JsonError_ok);
        assert(strcmp(buf, "hello, world") == 0);
    });
}


static void
test2_objects() {
    cs* data = R"(
    {"answer": 4.2e1, "float": 24214.5525e-2 }
    )";
    test_reader("test2.json", data, [] (JSON* json) {
        JSON object;
        JsonValueType type;
        JsonError err = json_reader_open_object(json, &object);
        assert(err == JsonError_ok);
        char buf[100];
        for (;;) {
            sz nbuf = sizeof(buf);
            err = json_reader_read_object(&object, &nbuf, buf, &type);
            if (err == JsonError_not_found) { break; }
            assert(err == JsonError_ok);
            if (strcmp(buf, "answer") == 0) {
                i32 x;
                err = json_reader_read_numberi(&object, &x);
                assert(err == JsonError_ok);
                assert(x == 42);
            }
            else if(strcmp(buf, "float") == 0) {
                r64 x;
                err = json_reader_read_numberd(&object, &x);
                assert(err == JsonError_ok);
                assert(x == 242.145525);
            }
            else {
                assert(false);
            }
        }
    });
}


static void
test3_arrays() {
    cs* data = R"(
    [[7, -11], [3.5, -1.5]]
    )";
    test_reader("test3.json", data, [] (JSON* json) {
        r32 expect_s[] = { 7, -11, 3.5, -1.5 };
        r32* pexpect = expect_s;
        JSON container;
        JsonValueType type;
        JsonError err = json_reader_open_array(json, &container);
        assert(err == JsonError_ok);
        for (;;) {
            err = json_reader_read_array(&container, &type);
            if (err == JsonError_not_found) { break; }
            assert(err == JsonError_ok);

            JSON item;
            JsonError err = json_reader_open_array(&container, &item);
            for (;;) {
                err = json_reader_read_array(&item, &type);
                if (err == JsonError_not_found) { break; }
                assert(err == JsonError_ok);

                r32 x;
                err = json_reader_read_numberf(&item, &x);
                assert(err == JsonError_ok);
                assert(x == *pexpect++);
            }
        }
    });
}


static void
test4_nulls() {
    cs* data = R"(
    [null, null, null, {"answer":null}]
    )";
    test_reader("test4.json", data, [] (JSON* json) {
        JSON container, object;
        JsonValueType type;
        JsonError err = json_reader_open_array(json, &container);
        assert(err == JsonError_ok);

        err = json_reader_read_array(&container, &type);
        assert(err == JsonError_ok);
        i32 x;
        err = json_reader_read_numberi(&container, &x);
        assert(err == JsonError_null);

        err = json_reader_read_array(&container, &type);
        assert(err == JsonError_ok);
        char sbuf[10];
        sz nbuf = sizeof(sbuf);
        err = json_reader_read_string(&container, &nbuf, sbuf);
        assert(err == JsonError_null);

        err = json_reader_read_array(&container, &type);
        assert(err == JsonError_ok);
        err = json_reader_read_null(&container);
        assert(err == JsonError_ok);

        err = json_reader_read_array(&container, &type);
        assert(err == JsonError_ok);
        {
            JsonError err = json_reader_open_object(json, &object);
            assert(err == JsonError_ok);
            char key[100];
            sz nkey = sizeof(key);
            err = json_reader_read_object(&object, &nkey, key, &type);
            assert(err == JsonError_ok);
            assert(strcmp(key, "answer") == 0);
            i32 x;
            err = json_reader_read_numberi(&object, &x);
            assert(err == JsonError_null);
        }
    });
}


static void
test5_booleans() {
    cs* data = R"(
    [false, true, null, 1]
    )";
    test_reader("test5.json", data, [] (JSON* json) {
        JSON container, object;
        JsonValueType type;
        JsonError err = json_reader_open_array(json, &container);
        assert(err == JsonError_ok);

        err = json_reader_read_array(&container, &type);
        assert(err == JsonError_ok);
        i32 x = true;
        err = json_reader_read_bool(&container, &x);
        assert(err == JsonError_ok);
        assert(x == false);

        err = json_reader_read_array(&container, &type);
        assert(err == JsonError_ok);
        i32 y = 17;
        err = json_reader_read_bool(&container, &y);
        assert(err == JsonError_ok);
        assert(y == 1);

        err = json_reader_read_array(&container, &type);
        assert(err == JsonError_ok);
        i32 z;
        err = json_reader_read_bool(&container, &z);
        assert(err == JsonError_null);

        err = json_reader_read_array(&container, &type);
        assert(err == JsonError_ok);
        i32 w;
        err = json_reader_read_bool(&container, &z);
        assert(err == JsonError_type_mismatch);
    });
}


static void
test6_chunked_read() {
    cs* data = R"(
    "long \u006c\u006F\u006e\u0067 long string"
    )";
    test_reader("test6.json", data, [] (JSON* json) {
        size_t chunk = 4;
        char buf[100];
        size_t bufsize = chunk;
        JsonError err = json_reader_read_string(json, &bufsize, buf);
        assert(err == JsonError_bufsize);
        size_t offset = bufsize;
        for (;;) {
            assert(offset + chunk <= sizeof(buf));
            bufsize = chunk;
            err = json_reader_resume_string(json, &bufsize, &buf[offset]);
            if (err == JsonError_ok) { break; }
            assert(err == JsonError_bufsize);
            offset += bufsize;
        }
        assert(strcmp(buf, "long long long string") == 0);
    });
}


static void
test7_numbers_i32() {
    cs* data = R"(
    [0, 1, -1, -2147483648, 2147483647, 4294967295]
    )";
    test_reader("test7.json", data, [] (JSON* json) {
        i32 expect_s[] = {0, 1, -1, -2147483648, 2147483647, -1};
        i32* pexpect = expect_s;
        JSON array;
        JsonValueType type;
        JsonError err = json_reader_open_array(json, &array);
        assert(err == JsonError_ok);
        for (;;) {
            err = json_reader_read_array(&array, &type);
            if (err == JsonError_not_found) { break; }
            assert(err == JsonError_ok);
            int value;
            err = json_reader_read_numberi(&array, &value);
            assert(err == JsonError_ok);
            assert(value == *pexpect++);
        }
    });
}


static void
test8_numbers_i64() {
    cs* data = R"(
    [0, 1, -1, -2147483648, 2147483647, 4294967295,
        -1234567890123456789, -9223372036854775808,
        1234567890123456789, 9223372036854775807]
    )";
    test_reader("test8.json", data, [] (JSON* json) {
        i64 expect_s[] = {0, 1, -1, -2147483648, 2147483647, 4294967295,
            -1234567890123456789, INT64_MIN,
            1234567890123456789, 9223372036854775807
        };
        i64* pexpect = expect_s;
        JSON array;
        JsonValueType type;
        JsonError err = json_reader_open_array(json, &array);
        assert(err == JsonError_ok);
        for (;;) {
            err = json_reader_read_array(&array, &type);
            if (err == JsonError_not_found) { break; }
            assert(err == JsonError_ok);
            i64 value;
            err = json_reader_read_numberll(&array, &value);
            assert(err == JsonError_ok);
            assert(value == *pexpect++);
        }
    });
}


static void
test9_numbers_double() {
    cs* data = R"(
    [0, 1, -1, -2147483648, 2147483647, 4294967295,
        -0.0, 1.2345, -1.2345,
       2.225073858507201e-308, 2.2250738585072014e-308, 1.7976931348623157e308]
    )";
    test_reader("test9.json", data, [] (JSON* json) {
        r64 expect_s[] = {0, 1, -1, -2147483648, 2147483647, 4294967295,
            0, 1.2345, -1.2345,
            2.225073858507201e-308, 2.2250738585072014e-308, 1.7976931348623157e308};
        r64* pexpect = expect_s;
        JSON array;
        JsonValueType type;
        JsonError err = json_reader_open_array(json, &array);
        assert(err == JsonError_ok);
        for (;;) {
            err = json_reader_read_array(&array, &type);
            if (err == JsonError_not_found) { break; }
            assert(err == JsonError_ok);
            r64 value;
            err = json_reader_read_numberd(&array, &value);
            assert(err == JsonError_ok);
            // printf("expect: %.39g, actual: %.39g\n", *pexpect, value);
            assert(abs(value - *pexpect++) < 1e-12);
        }
    });
}


static void
test10_consume_values() {
    cs* data = R"(
    {"true":true, "false":false, "null":null, "num42":42,
        "num-17":-17, "str": "string\n\n \u0031 \u0033",
        "arr": [1,35,"7",{}, null], "obj":{"inner":{}}}
    )";
    test_reader("test10.json", data, [] (JSON* json) {
        JSON object;
        JsonValueType type;
        JsonError err = json_reader_open_object(json, &object);
        assert(err == JsonError_ok);
        char buf[100];
        for (;;) {
            sz nbuf = sizeof(buf);
            err = json_reader_read_object(&object, &nbuf, buf, &type);
            if (err == JsonError_not_found) { break; }
            assert(err == JsonError_ok);
            err = json_reader_consume_value(&object);
            assert(err == JsonError_ok);
        }
    });
}


int main(int argc, const char* argv[]) {
    test1_hello();
    test2_objects();
    test3_arrays();
    test4_nulls();
    test5_booleans();
    test6_chunked_read();
    test7_numbers_i32();
    test8_numbers_i64();
    test9_numbers_double();
    test10_consume_values();

    return 0;
}

