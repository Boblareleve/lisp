#include "lisp.h"

#define Rc_get(obj) ((Rc_container*)(obj)-1)
#define Rc_dec(rc) (assert((rc)->ref_count >= 0), rc->ref_count == 0 ? free(rc) : rc->ref_count--)

#define Strv_Rc_dec(strv) Rc_dec(Rc_get(strv.arr))
#define Rc_String_to_Strv(rc_s) (assert((rc_s)->ref_count >= 0), (rc_s)->ref_count++, Strv_make((rc_s)->arr, (rc_s)->size))


static inline Variable Variable_make(Rc_List_array *value_parent, List value, Rc_String *name)
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
}

static inline bool Rc_inc_List(List li)
{
    if (li.tag == tag_list || li.tag == tag_string)
    {
        if (!li.list) return true;

        Rc_get(li.list)->ref_count++;
        return true;
    }
    return true;
}
static inline bool Rc_dec_List(List li)
{
    if (li.tag == tag_list || li.tag == tag_string)
    {
        if (!li.list) return true;

        Rc_container *rc = Rc_get(li.list);
        if (rc->ref_count == 0)
            free(rc->arr);
        else
            Rc_get(li.list)->ref_count--;
        return true;
    }
    return true;
}



static inline void *Rc_container_make(size_t element_count, size_t element_size)
{
    Rc_String *copy = malloc(sizeof(Rc_String) + element_count * element_size);
    copy->ref_count = 0;
    copy->size = element_count;
    return copy->arr;
}
static inline void *Rc_container_copy(Rc_container *src, size_t element_size)
{
    return memcpy(Rc_container_make(src->size, element_size), src->arr, element_size * src->size);
}

static inline bool Rcs_equal(const Rc_String *str1, const Strv str2)
{
    return str1->size == str2.size 
        && memcmp(str1->arr, str2.arr, str1->size)
    ;
}
#define Rcs_equal_lit(str1, str2) Rcs_equal(Rc_get(str1), Strv_lit(str2))

