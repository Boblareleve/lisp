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


bool test(const Strv str)
{
    if (str.size == 0)
        return true;   
    List root = {0};

    Lisp_context tmp_ctx = {0};
    set_Lisp_context(&tmp_ctx);
    
    // parse
    TRY(lists(str, &root), fprintf(fd, "parse error: "STRV_FMT"\t", STRV_UNPACK(g_ctx->error.view)); reset_error(););

    g_ctx = NULL;
    Strb_free(tmp_ctx.error);
    
    
    // run
    TRY(test_eval(root));

    return true;
}


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
            fprintf(fd, "[TEST] file '%s' not found\n", argv[i]);
            Strb_free(raw);
            continue ;
        }
        
        fprintf(fd, "TEST %-*s\t", 48, argv[i]);
        fflush(fd);
        if (!test(raw.view))
            fprintf(fd, "\tFAILURE\n");
        else
            fprintf(fd, "\tSUCCESS\n");
        
        Strb_free(raw);
    }
    
    free(g_ctx);
    return 0;
}



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