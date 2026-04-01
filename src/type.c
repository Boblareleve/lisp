
#include "lisp.h"


// bool is_composite_type(const List li)
// {
//     return li.tag == tag_type
//         && (
//             li.type_tag == 
//         )
// }

bool is_of_type(const List li, const List type)
{
    assert(type.tag == tag_type);
    if (type.type_tag == ttag_any_type) // "wildcard"
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

bool type_equal(const List a, const List b)
{
    TRY(a.tag == tag_type && b.tag == tag_type);
    TRY(a.type_tag == b.type_tag);
    if (a.type_tag == tag_list)
    {
        if (a.size == TYPE_UNDEFINED_LIST_SIZE && b.size == TYPE_UNDEFINED_LIST_SIZE)
            return true;
        TRY(a.size == b.size);
        for (int i = 0; i < a.size; i++)
            TRY(type_equal(a.list[i], b.list[i]));
    }
    return true;
}

bool type_compatible(const List a, const List b)
{
    TRY(a.tag == tag_type && b.tag == tag_type);
    if (a.type_tag == ttag_any_type || b.type_tag == ttag_any_type)
        return true;
    TRY(a.type_tag == b.type_tag);
    if (a.type_tag == tag_list)
    {
        if (a.size == TYPE_UNDEFINED_LIST_SIZE || b.size == TYPE_UNDEFINED_LIST_SIZE)
            return true;
        TRY(a.size == b.size);
        for (int i = 0; i < a.size; i++)
            TRY(type_compatible(a.list[i], b.list[i]));
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
