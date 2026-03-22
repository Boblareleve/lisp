#include "lisp.h"

// DA_TYPEDEF_ARRAY(List);
bool test_eval(List root)
{
    Lisp_context ctx = Lisp_context_init(root);
    if (root.size == 0)
    {
        Lisp_context_free(&ctx);
        return true;
    }
    
    if (root.list[0].tag == tag_symbole 
     && List_equal_lit(root.list[0], "ERROR")
    ) { // expect error
        for (int i = 1; i < root.size; i++)
        {
            List tmp = {0};
            if (!eval(&ctx, root.list[i], &tmp))
            {
                Lisp_context_free(&ctx);
                return true;
            }
        }
        fprintf(stderr, "no error while expecting one\t");
        goto fail;
    }

    
    List expect = {0};
    GOTRY(eval(&ctx, root.list[0], &expect), fprintf(stderr, "eval error while eval expected: "STRV_FMT"\t", STRV_UNPACK(error.view)));
    
    // last expected to be equal to "expect"
    List tmp = (List){0};
    for (int i = 1; i < root.size; i++)
    {
        tmp = (List){0};
        GOTRY(eval(&ctx, root.list[i], &tmp), fprintf(stderr, "unexpected error while eval: "STRV_FMT"\t", STRV_UNPACK(error.view)));
    }
    GOTRY(List_equal(expect, tmp),
        fprintf(stderr, "unexpected result got: '");
        List_print(tmp);
        fprintf(stderr, "'  expecting: '");
        List_print(expect);
        fprintf(stderr, "'\t");
    );
    
    Lisp_context_free(&ctx);
    return true;
fail:
    Lisp_context_free(&ctx);
    error.size = 0;
    return false;
}



bool test(const Strv str)
{
    if (str.size == 0)
        return true;   
    List root = {0};
    
    TRY(lists(str, &root), fprintf(stderr, "parse error: "STRV_FMT"\t", STRV_UNPACK(error.view)); error.size = 0;);
    
    
    // run
    TRY(test_eval(root), error.size = 0);

    error.size = 0;
    return true;
}


int main(int argc, char **argv)
{
    for (int i = 1; i < argc; i++)
    {
        Strb raw = {0};
        if (Strb_cat_file(&raw, argv[i]))
        {
            fprintf(stderr, "[TEST] file '%s' not found\n", argv[i]);
            continue ;
        }
        
        fprintf(stderr, "TEST %-*s\t", 48, argv[i]);
        if (!test(raw.view))
            fprintf(stderr, "\tFAILURE\n");
        else
            fprintf(stderr, "\tSUCCESS\n");
        

        Strb_free(raw);
    }
    return 0;
}

