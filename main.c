#include "lisp.h"

DA_TYPEDEF_ARRAY(List);
bool test_eval(const da_List lis)
{
    Lisp_context ctx = {0};
    if (da_first(&lis).tag == tag_symbole 
     && Strv_equal_lit(da_first(&lis).str, "ERROR")
    ) { // expect error
        for (int i = 1; i < lis.size; i++)
        {
            List tmp = {0};
            if (!eval(&ctx, lis.arr[i], &tmp))
            {
                // printf("(err)\t"); // : "STRV_FMT")\t", STRV_UNPACK(Strv_slice(error.view, -16, -1)));
                return true;
            }
        }
        printf("no error while expecting one\t");        
        return false;
    }
    List expect = {0};
    TRY(eval(&ctx, da_first(&lis), &expect), 
        printf("eval error while eval expected: "STRV_FMT"\t", STRV_UNPACK(error.view));
        error.size = 0;
    );
    // last expected to be equal to "expect"
    List tmp = (List){0};
    for (int i = 1; i < lis.size; i++)
    {
        tmp = (List){0};
        TRY(eval(&ctx, lis.arr[i], &tmp), 
            printf("unexpected error while eval: "STRV_FMT"\t", STRV_UNPACK(error.view));
            error.size = 0;
        );
    }
    TRY(List_equal(expect, tmp),
        printf("unexpected result got: ");
        print(tmp);
        printf("  expecting: ");
        print(expect);
        printf("\t");
        error.size = 0;
    );
    // print(tmp);
    // printf("\t");
    return true;
}
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
        TRY(list(&it, &da_top(&lis)),
            printf("parse error: "STRV_FMT"\t", STRV_UNPACK(error.view));
            error.size = 0;
        );
    }
    // da_for (List, it, &lis)
    // {
    //     print(*it);
    //     printf("\n");
    // }
    // printf("-------------\n");
    const int samples = 1;
    for (int _ = 0; _ < samples; _++)
        TRY(test_eval(lis));
    da_free(&lis);
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
        
        printf("TEST %s\t", argv[i]);
        if (!test(raw.view))
            printf("\tFAILURE\n");
        else
            printf("SUCCESS\n");
        

        Strb_free(raw);
    }
    return 0;
}

/* 
DA_TYPEDEF_ARRAY(List);
bool test_parse(Strv str)
{
    printf("\n");
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
        da_push_zero(&lis);
        TRY(list(&it, &da_top(&lis)),
            printf("parse error: "STRV_FMT"\t", STRV_UNPACK(error.view));
            error.size = 0;
        );
        printf("parse sucess:\n");
        print(da_top(&lis));
        printf("\n");
    }

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
        
        printf("TEST %s\t", argv[i]);
        if (!test_parse(raw.view))
            ; //printf("\tFAILURE\n");
        // else
            // printf("SUCCESS\n");
        

        Strb_free(raw);
    }
    return 0;
}
 */