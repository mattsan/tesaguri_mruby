#include <cstdio>

#include <mruby.h>
#include <mruby/compile.h>
#include <mruby/proc.h>
#include <mruby/string.h>

mrb_value my_class_print_name(mrb_state* mrb, mrb_value self)
{
    mrb_int   n;
    mrb_value s;
    mrb_get_args(mrb, "iS", &n, &s);

    mrb_value result = mrb_str_dup(mrb, s); // 文字列の複製
    for(int i = 1; i < n; ++i)
    {
        mrb_str_concat(mrb, result, s); // 文字列の連結（末尾へ追加）
    }

    return result;
}

int exec(int argc, char* argv[])
{
    mrb_state* mrb = mrb_open();

    if(mrb == 0)
    {
        printf("cannot open mruby\n");
        return -2;
    }

    RClass* my_class = mrb_define_class(mrb, "MyClass", mrb->object_class);
    mrb_define_method(mrb, my_class, "print_name", my_class_print_name, ARGS_REQ(2));

    for(int i = 0; i < argc; ++i)
    {
        FILE* fp = std::fopen(argv[i], "r");
        if(fp == 0)
        {
            printf("cannot open file \"%s\"\n", argv[i]);
            continue;
        }

        mrb_parser_state* parser       = mrb_parse_file(mrb, fp);
        int               byte_code    = mrb_generate_code(mrb, parser->tree);
        mrb_value         return_value = mrb_run( mrb,
                                                  mrb_proc_new(mrb, mrb->irep[byte_code]),
                                                  mrb_top_self(mrb)
                                                );
        if(mrb->exc) // if exception occurred ...
        {
            mrb_p(mrb, return_value);
            mrb->exc = 0;
        }

        std::fclose(fp);
    }

    mrb_close(mrb);

    return 0;
}

int main(int argc, char* argv[])
{
    if(argc == 1)
    {
        printf("no ruby file\n");
        return -1;
    }
    else
    {
        return exec(argc - 1, argv + 1);
    }
}