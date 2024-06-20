#include <stdlib.h>
#include <string.h>
#include "paiv_json.h"


static void
_assert_json_ok(JsonError err, int line) {
    if (err != JsonError_ok) {
        fprintf(stderr, "%s:%d: json error %d\n", __FILE__, line, err);
        abort();
    }
}

#define assert_json_ok(err) _assert_json_ok(err, __LINE__)


int main(int argc, const char* argv[]) {
    const char* filename = "menu.json";
    if (argc > 1) {
        filename = argv[1];
    }

    FILE* fp = fopen(filename, "r");
    if (fp == NULL) {
        perror(filename);
        return 1;
    }

    JSON json, content, menu, items;
    int err = json_reader_init(&json, fp);
    assert_json_ok(err);

    err = json_reader_open_object(&json, &content);
    assert_json_ok(err);

    char key[20], value[100];
    size_t key_size, value_size;
    JsonValueType type;
    for (;;) {
        key_size = sizeof(key);
        err = json_reader_read_object(&content, &key_size, key, &type);
        if (err == JsonError_not_found) { break; }
        assert_json_ok(err);

        if (strcmp(key, "menu") == 0) {
            err = json_reader_open_object(&content, &menu);
            assert_json_ok(err);

            for (;;) {
                key_size = sizeof(key);
                err = json_reader_read_object(&menu, &key_size, key, &type);
                if (err == JsonError_not_found) { break; }
                assert_json_ok(err);

                if (strcmp(key, "header") == 0) {
                    value_size = sizeof(value);
                    err = json_reader_read_string(&menu, &value_size, value);
                    assert_json_ok(err);
                    printf("menu header: %s\n", value);
                }
                else if (strcmp(key, "items") == 0) {
                    printf("items:\n");
                    err = json_reader_open_array(&menu, &items);
                    assert_json_ok(err);
                    for (;;) {
                        err = json_reader_read_array(&items, &type);
                        if (err == JsonError_not_found) { break; }
                        assert_json_ok(err);
                        
                        JSON item;
                        err = json_reader_open_object(&items, &item);
                        if (err == JsonError_null) {
                            printf("- --\n");
                            continue;
                        }
                        assert_json_ok(err);
                        printf("-");
                        for (;;) {
                            key_size = sizeof(key);
                            err = json_reader_read_object(&item, &key_size, key, &type);
                            if (err == JsonError_not_found) { break; }
                            assert_json_ok(err);
                            printf(" %s=", key);
                            value_size = sizeof(value);
                            err = json_reader_read_string(&item, &value_size, value);
                            if (err == JsonError_null) {
                                printf("null");
                                continue;
                            }
                            assert_json_ok(err);
                            printf("%s", value);
                        }
                        printf("\n");
                    }
                }
                else {
                    fprintf(stderr, "unhandled key: %s\n", key);
                    err = json_reader_consume_value(&menu);
                    assert_json_ok(err);
                }
            }
        }
        else {
            fprintf(stderr, "unhandled key: %s\n", key);
            err = json_reader_consume_value(&content);
            assert_json_ok(err);
        }
    }

    fclose(fp);
    return 0;
}

