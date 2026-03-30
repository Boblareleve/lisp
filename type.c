
#include "lisp.h"


bool is_of_type(const List li, const List type)
{
    assert(type.tag == tag_type);
    if (type.type_tag == ttag_any_type)
        return true;
    TRY(li.tag == type.type_tag);
    TRY(li.quote_count == type.quote_count);

    if (li.tag == tag_list)
    {

        if (type.size == TYPE_UNDEFINED_LIST_SIZE)
            return true;
        TRY(li.size == type.size);
        
        for (int i = 0; i < li.size; i++)
            TRY(is_of_type(li.list[i], type.list[i]));
        return true;
    }

    return true;
}

// void add_simple_type(Lisp_context *ctx, const char *name, List_tag tag)
// {
//     set_Variable_insert(&ctx->types, (Variable){
//         _cstr_to_List(name),
//         (List){
//             .tag = tag_type,
//             .type_tag = tag
//         }
//     });
// }
void add_simple_type(Lisp_context *ctx, const char *name, List type)
{
    size_t name_len = strlen(name);
    set_Variable_insert(&ctx->types, (Variable){ 
        .name = (List){ .tag = tag_symbole, .size = name_len, .str = List_duplicate(ctx, name, name_len) }, 
        .value = type, 
        .type = TYPE_TYPE
    });
}
