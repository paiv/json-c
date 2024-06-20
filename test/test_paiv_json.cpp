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
test1() {
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
test2() {
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
test3() {
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
test4() {
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
test5() {
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


int main(int argc, const char* argv[]) {
    test1();
    test2();
    test3();
    test4();
    test5();

    return 0;
}

