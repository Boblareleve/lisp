
#include "lisp.h"


bool have_function_arguments_shape(const List li)
{
    TRY(li.tag == tag_list);
    for (int i = 0; i < li.size; i++)
        TRY(li.list[i].tag == tag_symbole // symbole that will be link to a type or a is a local variable
         || li.list[i].tag == tag_type);  // or a type
    return true;
}

bool is_of_type(const List li, const List type)
{
    assert(type.tag == tag_type);

    // wildcard
    if (type.type_tag == ttag_any_type) 
        return true;
    
    TRY(li.quote_count == type.quote_count);
    
    // union
    if (type.type_tag == ttag_union_type)
    {
        for (int i = 0; i < type.size; i++)
            if (is_of_type(li, type.list[i]))
                return true;        
        
        return false;
    }

    // lists
    {
        // any list just a list of any shape: (A C B ...)
        if (type.type_tag == ttag_any_list_type)
            return li.tag == tag_list;
        
        // array of n time a type: (T T T)
        if (type.type_tag == ttag_array_type)
        {
            TRY(li.tag == tag_list && li.size == type.size);
            
            for (int i = 0; i < type.size; i++)
                if (is_of_type(li.list[i], (List){ .tag = tag_type, .type_tag = type.type_list_tag }))
                    return true;
            
            return false;
        }
    
        // strict sequence of n type (T1 T2 T3)
        if (type.type_tag == ttag_tuple_type)
        {
            TRY(li.tag == tag_list && li.size == type.size);
            
            for (int i = 0; i < li.size; i++)
                TRY(is_of_type(li.list[i], type.list[i]));
            
            return true;
        }
    }

    TRY(li.tag == type.type_tag);

    return true;
}

// bool type_equal(const List a, const List b)
// {
//     TRY(a.tag == tag_type && b.tag == tag_type);
//     TRY(a.type_tag == b.type_tag);
//     if (a.type_tag == tag_list)
//     {
//         if (a.size == TYPE_UNDEFINED_LIST_SIZE && b.size == TYPE_UNDEFINED_LIST_SIZE)
//             return true;
//         TRY(a.size == b.size);
//         for (int i = 0; i < a.size; i++)
//             TRY(type_equal(a.list[i], b.list[i]));
//     }
//     return true;
// }

bool type_equal(const List t1, const List t2)
{
    TODO("type equal");
    // TRY(a.tag == tag_type && b.tag == tag_type);
    // if (a.type_tag == ttag_any_type || b.type_tag == ttag_any_type)
    //     return true;
    // TRY(a.type_tag == b.type_tag);
    // if (a.type_tag == tag_list)
    // {
    //     if (a.size == TYPE_UNDEFINED_LIST_SIZE || b.size == TYPE_UNDEFINED_LIST_SIZE)
    //         return true;
    //     TRY(a.size == b.size);
    //     for (int i = 0; i < a.size; i++)
    //         TRY(type_equal(a.list[i], b.list[i]));
    // }
    return true;
}

void add_primitive_type(const char *name, List type)
{
    assert(type.tag == 0 || type.tag == tag_type);
    type.tag = tag_type;
    
    size_t name_len = strlen(name);
    set_insert(&g_ctx->variables, ((Variable){
        .name = (List){
            .tag = tag_symbole,
            .size = name_len,
            .str = List_duplicate(name, name_len)
        },
        .value = type,
        .type = TYPE_TYPE
    }));
}
