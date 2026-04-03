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
            List tmp = {0};
            if (!eval(root.list[i], &tmp))
            {
                Lisp_context_free();
                return true;
            }
        }
        fprintf(fd, "no error while expecting one\t");
        goto fail;
    }

    
    List expect = {0};
    GOTRY(eval(root.list[0], &expect), fprintf(fd, "eval error while eval expected: "STRV_FMT"\t", STRV_UNPACK(error.view)));

    // last expected to be equal to "expect"
    List tmp = (List){0};
    for (int i = 1; i < root.size; i++)
    {
        tmp = (List){0};
        GOTRY(eval(root.list[i], &tmp), fprintf(fd, "unexpected error while eval: "STRV_FMT"\t", STRV_UNPACK(error.view)));
    }
    GOTRY(List_equal(expect, tmp),
        fprintf(fd, "unexpected result got: '");
        List_print(tmp);
        fprintf(fd, "'  expecting: '");
        List_print(expect);
        fprintf(fd, "'\t");
    );

    Lisp_context_free();
    return true;
fail:
    Lisp_context_free();
    error.size = 0;
    return false;
}


bool test(const Strv str)
{
    if (str.size == 0)
        return true;   
    List root = {0};
    
    TRY(lists(str, &root), fprintf(fd, "parse error: "STRV_FMT"\t", STRV_UNPACK(error.view)); error.size = 0;);
    
    // run
    TRY(test_eval(root), error.size = 0);

    error.size = 0;
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
            continue ;
        }
        
        fprintf(fd, "TEST %-*s\t", 48, argv[i]);
        if (!test(raw.view))
            fprintf(fd, "\tFAILURE\n");
        else
            fprintf(fd, "\tSUCCESS\n");
        
        Strb_free(raw);
    }
    Strb_free(error);

    
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