#include "lisp.h"


FILE *fd = NULL;

bool test_eval(List root)
{
    
    set_Lisp_context(Lisp_context_init(root));

    if (root.size == 0)
    {
        Lisp_context_free();
        return true;
    }
    
    if (root.list[0].tag == tag_symbole 
     && List_equal_lit(root.list[0], "ERROR")
    ) { // expect error
        for (int i = 1; i < root.size; i++)
        {
            VM_push(root.list[i]);
            if (!eval())
            {
                Lisp_context_free();
                return true;
            }
            VM_pop;
        }
        fprintf(fd, "no error while expecting one\t");
        goto fail;
    }

    
    VM_push(root.list[0]);
    GOTRY(eval(), fprintf(fd, "eval error while eval expected: "STRV_FMT"\t", STRV_UNPACK(g_ctx->error.view)));

    // last expected to be equal to "expect"
    VM_push(NIL_LIST);
    for (int i = 1; i < root.size; i++)
    {
        VM_top1 = root.list[i];
        GOTRY(eval(), fprintf(fd, "unexpected error while eval: "STRV_FMT"\t", STRV_UNPACK(g_ctx->error.view)));
    }
    GOTRY(g_ctx->vm_stack.size == 2,  fprintf(fd, "return into main with too many element on the stack got %d", g_ctx->vm_stack.size));
    GOTRY(List_equal(VM_top1, VM_top2),
        fprintf(fd, "unexpected result got: '");
        List_print(VM_top1);
        fprintf(fd, "'  expecting: '");
        List_print(VM_top2);
        fprintf(fd, "'\t");
    );

    Lisp_context_free();
    return true;
fail:
    Lisp_context_free();
    return false;
}


bool _test(const Strv str)
{
    if (str.size == 0)
        return true;   
    List root = {0};

    Lisp_context tmp_ctx = {0};
    set_Lisp_context(&tmp_ctx);
    
    // parse
    TRY(lists(str, &root), fprintf(fd, "parse error: "STRV_FMT"\t", STRV_UNPACK(g_ctx->error.view)); reset_error(););

    set_void_ptr_free(&g_ctx->gc);
    g_ctx = NULL;
    Strb_free(tmp_ctx.error);
    
    
    // run
    TRY(test_eval(root));

    return true;
}


int __main(int argc, char **argv)
{
    init_primitive_map();
    test_get_Primitive();

    fd = stdout;
    for (int i = 1; i < argc; i++)
    {
        Strb raw = {0};
        if (Strb_cat_file(&raw, argv[i]))
        {
            fprintf(fd, "[TEST %d] file '%s' not found\n", i, argv[i]);
            Strb_free(raw);
            continue ;
        }
        
        fprintf(fd, "[TEST %d] %-*s\t", i, 48, argv[i]);
        fflush(fd);
        if (!_test(raw.view))
            fprintf(fd, "\tFAILURE\n");
        else
            fprintf(fd, "\tSUCCESS\n");
        
        Strb_free(raw);
    }
    
    free(g_ctx);
    return 0;
}


bool test(const Strv str)
{
    static_error = (Strb){0};
    if (str.size == 0)
        return true;
    
    // parse
    List root = {0}; {

        g_ctx = NULL;
        if (!lists(str, &root))
        {
            if (Strv_start_with_cstr(str, "ERROR"))
            {
                Strb_free(static_error);
                return true;
            }
            fprintf(fd, "parse error: "STRV_FMT"\t", STRV_UNPACK(static_error.view));
            
            Strb_free(static_error);
            return false;
        }
        
        Strb_free(static_error);
    }
    
    
    set_Lisp_context(Lisp_context_init(root));
    
    if (root.list[0].tag == tag_symbole) //, fprintf(fd, "expected a symbole to indicate test type"));
    {
        if (List_equal_lit(root.list[0], "PARSE")) // do not run
        {
            Lisp_context_free();
            return true;
        }
        if (List_equal_lit(root.list[0], "ERROR")) // expect an error to occure
        {
            for (int i = 1; i < root.size; i++)
            {
                VM_push(root.list[i]);
                if (!eval())
                {
                    Lisp_context_free();
                    return true;
                }
                VM_pop;
            }
            fprintf(fd, "no error while expecting one\t");
            Lisp_context_free();
            return false;
        }
        if (List_equal_lit(root.list[0], "EQUAL")) // expect to finish with the same value as the next one
        {
            VM_push(root.list[1]);
            TRY(eval(), fprintf(fd, STRV_FMT, STRV_UNPACK(g_ctx->error.view)); Lisp_context_free());
            List first = VM_top1;
            VM_pop;

            VM_push(NIL_LIST);
            for (int i = 2; i < root.size; i++)
            {
                VM_top1 = root.list[i];
                TRY(eval(), fprintf(fd, STRV_FMT, STRV_UNPACK(g_ctx->error.view)); Lisp_context_free());
            }
            List last = VM_top1;
            VM_pop;

            if (!List_equal(first, last))
            {
                fprintf(fd, "expected: ");
                List_print(first);
                fprintf(fd, " got ");
                List_print(last);
                fprintf(fd, "\"");
                Lisp_context_free();
                return false;
            }
            Lisp_context_free();
            return true;
        }
        if (List_equal_lit(root.list[0], "TRUE")) // finish with true value
        {
            VM_push(NIL_LIST);
            for (int i = 1; i < root.size; i++)
            {
                VM_top1 = root.list[i];
                TRY(eval(), fprintf(fd, STRV_FMT, STRV_UNPACK(g_ctx->error.view)); Lisp_context_free());
            }
            if (VM_top1.tag != tag_true)
            {
                fprintf(fd, "expected true got: ");
                List_print(VM_top1);
                Lisp_context_free();
                return false;
            }
            VM_pop;
            Lisp_context_free();
            return true;
        }
    }
    
    VM_push(NIL_LIST);
    for (int i = 0; i < root.size; i++)
    {
        VM_top1 = root.list[i];
        TRY(eval(), fprintf(fd, STRV_FMT, STRV_UNPACK(g_ctx->error.view)); Lisp_context_free());
    }
    VM_pop;
    Lisp_context_free();
    return true;
}

// int tests(int argc, char **argv)
int main(int argc, char **argv)
{
    init_primitive_map();
    test_get_Primitive();

    fd = stdout;
    for (int i = 1; i < argc; i++)
    {
        Strb raw = {0};
        if (Strb_cat_file(&raw, argv[i]))
        {
            fprintf(fd, "[TEST %d] file '%s' not found\n", i, argv[i]);
            Strb_free(raw);
            continue ;
        }
        
        fprintf(fd, "[TEST %d] %-*s\t", i, 64, argv[i]);
        fflush(fd);
        
        int samples_count = 1;
        clock_t time;
        bool res = true;
        TIME(time)
        {
            for (int i = 0; res && i < samples_count; i++)
                res = test(raw.view);
        }
        if (!res)
            fprintf(fd, "\tFAILURE");
        else
            fprintf(fd, "SUCCESS");

        time /= samples_count;
        if (time >= CLOCKS_PER_SEC/1000)
            fprintf(fd, "  %.3lfms\n", (double)time * (1000.0 / CLOCKS_PER_SEC));
        else
            fprintf(fd, "  %liµs\n", time * (clock_t)(1000 * 1000 / CLOCKS_PER_SEC));
        
        Strb_free(raw);
    }
    
    return 0;
}


// int main(int argc, char **argv)
// {
//     if (argc <= 1) return 1;
//     switch (argv[1])
//     {
//     case 't': return tests(argc-1, argv+1);
//     case 'c': return run(argc-1, argv+1);
//     default: return 1;
//     }
//     return 0;
// }


/* #include <dlfcn.h>

bool a()
{
    List dl_sqlite3 = load_dl(_cstr_to_List("libsqlite3.so.0"));
    if (IS_NIL(dl_sqlite3))
    {
        fprintf(fd, "%.*s\n", error.size, error.arr);
        error.size = 0;
        return false;
    }

    List_print(dl_sqlite3);
    printf("\n");
    
    
    List sqlv = get_fun_dl(dl_sqlite3, _cstr_to_List("sqlite3_libversion"), NIL_LIST);
    if (IS_NIL(sqlv))
    {
        fprintf(fd, "%.*s\n", error.size, error.arr);
        return false;
    }
    assert(sqlv.fun._1);
    printf("sqlite3 version: %s\n", (char*)sqlv.fun._8());
    
    unload_dl(dl_sqlite3);
    fprintf(fd, "%.*s\n", error.size, error.arr);
    error.size = 0;
    return true;
} */
