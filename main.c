#include "lisp.h"

DA_TYPEDEF_ARRAY(List);
bool test_eval(const da_List lis)
{
    Lisp_context ctx = Lisp_context_init();
    if (da_first(&lis).tag == tag_symbole 
     && List_str_equal(da_first(&lis), "ERROR")
    ) { // expect error
        for (int i = 1; i < lis.size; i++)
        {
            List tmp = {0};
            if (!eval(&ctx, lis.arr[i], &tmp))
            {
                Lisp_context_free(&ctx);
                return true;
            }
        }
        printf("no error while expecting one\t");
        goto fail;
    }
    List expect = {0};
    GOTRY(eval(&ctx, da_first(&lis), &expect), 
        printf("eval error while eval expected: "STRV_FMT"\t", STRV_UNPACK(error.view));
    );
    // last expected to be equal to "expect"
    List tmp = (List){0};
    for (int i = 1; i < lis.size; i++)
    {
        tmp = (List){0};
        GOTRY(eval(&ctx, lis.arr[i], &tmp), 
            printf("unexpected error while eval: "STRV_FMT"\t", STRV_UNPACK(error.view));
        );
    }
    GOTRY(List_equal(expect, tmp),
        printf("unexpected result got: ");
        List_print(tmp);
        printf("  expecting: ");
        List_print(expect);
        printf("\t");
    );

    Lisp_context_free(&ctx);
    return true;
fail:
    Lisp_context_free(&ctx);
    error.size = 0;
    return false;
}


Ar arena = {0};

bool test(const Strv str)
{
    if (str.size == 0)
        return true;   
    da_List lis = {0};
    Strv it = *(Strv*)&str;
    while (it.size > 0)
    {
        skip_space(&it);
        if (Strv_first(it) == ';')
        {
            skip_comment(&it);
            continue;
        }
        if (it.size <= 0) break;

        da_push_zero(&lis);
        GOTRY(list(&arena, &it, &da_top(&lis)),
            printf("parse error: "STRV_FMT"\t", STRV_UNPACK(error.view));
        );
    }
    
    // copy

    da_List cpy = {0};
    bool do_copy = false;
    if (do_copy)
    {
        da_for (List, it, &lis)
            da_push(&cpy, List_copy(&arena, *it));
    }
    
    
    // run
    const int samples = 1;
    for (int _ = 0; _ < samples; _++)
        GOTRY(test_eval(lis));

    
    if (do_copy)
    {
        bool some_changes = false;
        for (int i = 0; i < cpy.size; i++)
            if (!List_equal(cpy.arr[i], lis.arr[i])) 
            {
                printf("code have change\t");
                some_changes = true;
            }
        if (!some_changes) printf("no code changes\t");
    }
    
    da_for (List, it, &lis)
        List_free(it);
    da_free(&lis);

    if (do_copy)
    {
        da_for (List, it, &cpy)
            List_free(it);
        da_free(&cpy);
    }

    error.size = 0;
    return true;
fail:
    error.size = 0;
    da_free(&lis);
    return false;
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
        
        printf("TEST %-*s\t", 48, argv[i]);
        if (!test(raw.view))
            printf("\tFAILURE\n");
        else
            printf("SUCCESS\n");
        

        Strb_free(raw);
    }
    return 0;
}

