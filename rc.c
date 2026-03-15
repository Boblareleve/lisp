

// while >= 0 object valid -> rc == 0 <=> one reference for (Rc_container){0} convinence
#define RC_CONTAINER_HEADER size_t ref_count // struct { size_t ref_count; size_t size; }

typedef struct Rc_container
{
    RC_CONTAINER_HEADER;
    uint8_t *arr[0];
} Rc_container;

typedef struct Rc_List_array
{
    RC_CONTAINER_HEADER;
    List arr[0];
} Rc_List_array;
typedef struct Rc_String
{
    RC_CONTAINER_HEADER;
    char arr[0];
} Rc_String;

#define _Rc_get(obj) (((Rc_container*)(obj))-1)
// #define Rc_dec(rc) (assert((rc)->ref_count >= 0), rc->ref_count == 0 ? free(rc) : rc->ref_count--)
// #define Strv_Rc_dec(strv) Rc_dec(Rc_get(strv.arr))
// #define Rc_String_to_Strv(rc_s) (assert((rc_s)->ref_count >= 0), (rc_s)->ref_count++, Strv_make((rc_s)->arr, (rc_s)->size))


Rc_List_array *Rc_get_list(const List *li)
{
    if (li->tag != tag_list) return NULL;

    return (Rc_List_array*)_Rc_get(&li->list[-(int)li->offset]);
}
Rc_String *Rc_get_str(const List *li)
{
    if (li->tag != tag_string) return NULL;

    return (Rc_String*)_Rc_get(&li->str[-(int)li->offset]);
}


/* static inline Variable Variable_make(Rc_List_array *value_parent, List value, Rc_String *name)
{
    assert(value_parent->ref_count != -1);
    assert(name->ref_count != -1);
    if (value_parent)
    {
        assert(value_parent->ref_count >= 0);
        value_parent->ref_count++;
    }

    return (Variable){
        .value = value,
        .name = Rc_String_to_Strv(name)
    };
} */


static inline bool Rc_inc_List(List li)
{
    if (li.tag == tag_list)
    {
        if (!li.list) return true;
        
        Rc_get_list(li.list)->ref_count++;
        return true;
    }
    if (li.tag == tag_string)
    {
        if (!li.str) return true;
        
        Rc_get_str(li.str)->ref_count++;
        return true;
    }
    return true;
}
static inline bool Rc_dec_List(List li)
{
    if (li.tag == tag_list)
    {
        if (!li.list) return true;

        Rc_List_array *rc = Rc_get_list(li.list);
        if (rc->ref_count == 0)
        {
            rc->ref_count = -1;
            free(rc->arr);
        }
        else
            rc->ref_count--;
        return true;
    }
    if (li.tag == tag_string)
    {
        if (!li.list) return true;

        Rc_String *rc = Rc_get_str(li.str);
        if (rc->ref_count == 0)
        {
            rc->ref_count = -1;
            free(rc->arr);
        }
        else
            rc->ref_count--;
        return true;
    }
    return true;
}



static inline void *Rc_container_make(size_t element_count, size_t element_size)
{
    Rc_String *new = malloc(sizeof(Rc_String) + element_count * element_size);
    new->ref_count = 0;
    // new->size = element_count;
    return new->arr;
}

static inline bool List_str_equal(const List li, const char *str, size_t size)
{
    assert(li.tag == tag_string);
    return li.size == size
        && memcmp(li.str, str, size)
    ;
}


/* static inline void *Rc_container_copy(Rc_container *src, size_t element_size)
{
    return memcpy(Rc_container_make(src->size, element_size), src->arr, element_size * src->size);
} */

/* static inline bool Rcs_equal(const Rc_String *str1, const Strv str2)
{
    return str1->size == str2.size 
        && memcmp(str1->arr, str2.arr, str1->size)
    ;
} */
// #define Rcs_equal_lit(str1, str2) Rcs_equal(Rc_get(str1), Strv_lit(str2))

