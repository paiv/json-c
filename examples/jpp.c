#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include "paiv_json.h"


static const char _usage[] =
    "usage: jpp [-i INDENT] [-b BUFSIZE] <file>\n"
    ;


static int
_check_json_ok(JsonError err, int line) {
    if (err != JsonError_ok) {
        fprintf(stderr, "%s:%d: json error %d\n", __FILE__, line, err);
        return 1;
    }
    return 0;
}

#define guard_ok(err) { if (_check_json_ok(err, __LINE__) != 0) { return err; }}


typedef struct {
    FILE* file_out;
    int filename_count;
    const char* filenames[100];
    size_t bufsize;
    char* buf;
    int nesting;
    int indent_size;
} Context;


static const int _DefaultIndent = INT_MIN;
static JsonError jfilter_value(JSON* jin, JSON* jout, Context* context);


static void
print_indent(Context* context) {
    if (context->indent_size == _DefaultIndent) {
        fputc(' ', context->file_out);
    }
    else if (context->indent_size > 0) {
        fprintf(context->file_out, "\n%*s", context->nesting * context->indent_size, "");
    }
}


static JsonError
jfilter_object(JSON* jin, JSON* jout, Context* context) {
    JSON content, so;
    JsonError err = json_reader_open_object(jin, &content);
    guard_ok(err);

    err = json_writer_open_object(jout, &so);
    context->nesting += 1;
    int vcount = 0;

    for (;; ++vcount) {
        JsonValueType type;
        size_t key_size = context->bufsize;
        err = json_reader_read_object(&content, &key_size, context->buf, &type);
        if (err == JsonError_not_found) { break; }
        guard_ok(err);
        err = json_writer_write_object_value_separator(&so);
        guard_ok(err);
        if (vcount != 0 || context->indent_size != _DefaultIndent) {
            print_indent(context);
        }
        err = json_writer_write_string(&so, context->buf);
        guard_ok(err);
        err = json_writer_write_object_key_separator(&so);
        guard_ok(err);
        if (context->indent_size != 0) {
            fputc(' ', context->file_out);
        }
        err = jfilter_value(&content, &so, context);
        guard_ok(err);
    }

    context->nesting -= 1;
    if (vcount != 0 && context->indent_size != _DefaultIndent) {
        print_indent(context);
    }
    err = json_writer_close_object(&so);
    return err;
}


static JsonError
jfilter_array(JSON* jin, JSON* jout, Context* context) {
    JSON content, so;
    JsonError err = json_reader_open_array(jin, &content);
    guard_ok(err);

    err = json_writer_open_array(jout, &so);
    context->nesting += 1;
    int vcount = 0;

    for (;; ++vcount) {
        JsonValueType type;
        err = json_reader_read_array(&content, &type);
        if (err == JsonError_not_found) { break; }
        guard_ok(err);
        err = json_writer_write_array_value_separator(&so);
        guard_ok(err);
        if (vcount != 0 || context->indent_size != _DefaultIndent) {
            print_indent(context);
        }
        err = jfilter_value(&content, &so, context);
        guard_ok(err);
    }

    context->nesting -= 1;
    if (vcount != 0 && context->indent_size != _DefaultIndent) {
        print_indent(context);
    }
    err = json_writer_close_array(&so);
    return err;
}


static JsonError
jfilter_value(JSON* jin, JSON* jout, Context* context) {
    JsonValueType type;
    JsonError err = json_reader_peek_value(jin, &type);
    guard_ok(err);
    switch (type) {
        case JsonValueType_object:
            err = jfilter_object(jin, jout, context);
            break;
        case JsonValueType_array:
            err = jfilter_array(jin, jout, context);
            break;
        case JsonValueType_number: {
            double value;
            err = json_reader_read_numberd(jin, &value);
            guard_ok(err);
            err = json_writer_write_numberd(jout, value);
            guard_ok(err);
            break;
        }
        case JsonValueType_string: {
            size_t value_size = context->bufsize;
            err = json_reader_read_string(jin, &value_size, context->buf);
            guard_ok(err);
            err = json_writer_write_string(jout, context->buf);
            guard_ok(err);
            break;
        }
        case JsonValueType_false:
        case JsonValueType_true: {
            int value;
            err = json_reader_read_bool(jin, &value);
            guard_ok(err);
            err = json_writer_write_bool(jout, value);
            guard_ok(err);
            break;
        }
        case JsonValueType_null:
            err = json_reader_read_null(jin);
            guard_ok(err);
            err = json_writer_write_null(jout);
            guard_ok(err);
            break;
        default:
            fprintf(stderr, "! unhandled value type %d\n", type);
            abort();
    }
    return JsonError_ok;
}



static int
parse_args(int argc, const char* argv[], Context* context) {
    if (argc < 2) {
        printf(_usage);
        return 0;
    }

    context->filename_count = 0;
    context->indent_size = _DefaultIndent;

    int i = 1;
    const char* arg = argv[i];
    int state = 0;
    size_t fncap = sizeof(context->filenames)/sizeof(context->filenames[0]);

    for (; i < argc; ++i) {
        arg = argv[i];
        size_t arg_size = strlen(arg);
        switch (state) {
            case 0:
                if (arg_size == 0) { continue; }
                if (arg_size > 1 && arg[0] == '-') {
                    if (
                        strcmp(arg, "-h") == 0 ||
                        strcmp(arg, "-help") == 0 ||
                        strcmp(arg, "--help") == 0
                    ) {
                        printf(_usage);
                        exit(0);
                    }
                    else if (
                        strcmp(arg, "-i") == 0 ||
                        strcmp(arg, "--indent-size") == 0
                    ) {
                        state = 10;
                    }
                    else if (
                        strcmp(arg, "-b") == 0 ||
                        strcmp(arg, "--buffer-size") == 0 ||
                        strcmp(arg, "--bufsize") == 0
                    ) {
                        state = 20;
                    }
                    else {
                        fprintf(stderr, "unknown option: %s", arg);
                        fprintf(stderr, _usage);
                        return 1;
                    }
                }
                else {
                    if (context->filename_count + 1 < fncap) {
                        context->filenames[context->filename_count++] = arg;
                    }
                    else {
                        fprintf(stderr, "warning: skipping %s\n", arg);
                    }
                }
                break;
            case 10: {
                char* ps;
                long n = strtol(arg, &ps, 0);
                if (n < 0 || ps != &arg[arg_size]) {
                    fprintf(stderr, "! invalid indent size: %s\n", arg);
                    fprintf(stderr, _usage);
                    return 1;
                }
                context->indent_size = n;
                state = 0;
                break;
            }
            case 20: {
                char* ps;
                long n = strtol(arg, &ps, 0);
                if (n < 0 || ps != &arg[arg_size]) {
                    fprintf(stderr, "! invalid bufsize: %s\n", arg);
                    fprintf(stderr, _usage);
                    return 1;
                }
                context->bufsize = n;
                state = 0;
                break;
            }
        }
    }

    if (state != 0) {
        fprintf(stderr, _usage);
        return 1;
    }

    return 0;
}


int main(int argc, const char* argv[]) {
    Context context;
    context.bufsize = 1000;
    context.nesting = 0;
    context.file_out = stdout;

    int res = parse_args(argc, argv, &context);
    if (res != 0) { return res; }

    char buf[context.bufsize];
    context.buf = buf;

    int fi = 0;
    const char* filename;

    for (; fi < context.filename_count; ++fi) {
        filename = context.filenames[fi];
        FILE* fp;
        if (strcmp(filename, "-") == 0) {
            fp = stdin;
        }
        else {
            fp = fopen(filename, "r");
            if (fp == NULL) {
                perror(filename);
                return 1;
            }
        }

        JSON jreader, jwriter;
        JsonError err = json_reader_init(&jreader, fp);
        guard_ok(err);

        err = json_writer_init(&jwriter, context.file_out);
        guard_ok(err);

        err = jfilter_value(&jreader, &jwriter, &context);
        guard_ok(err);

        if (fp != stdin) {
            fclose(fp);
        }
        puts("");
    }
    return 0;
}
