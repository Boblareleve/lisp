
#include "lisp.h"

typedef struct Primitive
{
    List name;
    primitive_t fun;
} Primitive;

#include "primitives.c"

uint64_t set_Primitive_hash(const Primitive prim, uint64_t seed)
{
    const List str = prim.name;
    uint32_t f2chars = 0;
    if (str.size == 0) return 0;
    
    if (str.size == 1)
        f2chars = str.str[0] << 3*8; // <- useless
    else
        f2chars = (str.str[0]          << 0*8)
                | (str.str[1]          << 1*8)
                | (str.str[str.size-2] << 2*8)
                | (str.str[str.size-1] << 3*8);
    
    return f2chars * seed;
}
int set_Primitive_equal(const Primitive a, const Primitive b)
{
    return List_str_equal(a.name, b.name);
}


#define SET_PRIM_IS_NULL(p) ((p).fun == NULL)
#define SET_PRIM_SET_NULL(p) ((p).fun = NULL)
SET_IMPLEMENT_HASH_SET(Primitive, SET_PRIM_IS_NULL, SET_PRIM_SET_NULL, 2, 0.7, 64);






static set_Primitive map = {0};
void init_primitive_map(void)
{
    static_for (i, keys)
        set_insert(&map, keys[i]);
}

primitive_t get_Primitive(const List op)
{
    return struct_unwrap(set_get(&map, (Primitive){ .name = op }), fun);
}

bool test_get_Primitive(void)
{
#ifdef DEBUG
    init_primitive_map();

    static_for (i, keys)
    {
        primitive_t p = get_Primitive(keys[i].name);
        // printf("-> p %p <> .fun %p\n", p, keys[i].fun);
        (void)p;
        assert(keys[i].fun == p); // map[keys[i].name]);
    }
#endif /* DEBUG */
    return true;
}

/*

uint32_t primitive_hash(const List str, uint32_t seed)
{
    uint32_t f2chars = 0;
    if (str.size == 0) return 0;
    
    if (str.size == 1)
        f2chars = str.str[0] << 3*8; // <- useless
    else
        f2chars = (str.str[0]          << 0*8)
                | (str.str[1]          << 1*8)
                | (str.str[str.size-2] << 2*8)
                | (str.str[str.size-1] << 3*8);
    
    return f2chars * seed;
}
// parameters: s6436: c0 a71
static primitive_t map[71] = {0};

typedef uint32_t (*hash_t)(const List str, uint32_t seed);

uint32_t hash0(const List str, uint32_t seed)
{
    uint32_t f2chars = 0;
    if (str.size == 0) return 0;
    
    if (str.size == 1)
        f2chars = str.str[0] << 3*8;
    else
        f2chars = (str.str[0]          << 0*8)
                | (str.str[1]          << 1*8)
                | (str.str[str.size-2] << 2*8)
                | (str.str[str.size-1] << 3*8);
    
    return f2chars * seed;
}

uint32_t hash1(const Strv str, uint32_t seed)
{
    uint32_t f2chars = 0;
    if (str.size == 0) return 0;
    
    if (str.size == 1)
        f2chars = str.arr[0] << 3*8;
    else
        f2chars = (str.arr[0]          << 0*8)
                // | (str.arr[1]          << 1*8)
                // | (str.arr[str.size-2] << 2*8)
                | (str.arr[str.size-1] << 3*8)
    ;
    
    return f2chars * seed;
}

uint32_t hash2(const Strv str, uint32_t seed)
{
    uint32_t f2chars = 0;
    if (str.size == 0) return 0;
    
    if (str.size == 1)
        f2chars = str.arr[0] << 3*8;
    else
        f2chars = ((str.arr[0]          << 0*8)
                ^ (str.arr[1]          << 0*8))
                | ((str.arr[str.size-2] << 1*8)
                ^ (str.arr[str.size-1] << 1*8))
    ;
    
    return f2chars ^ seed;
}

uint32_t hash3(const Strv str, uint32_t seed)
{
    uint32_t f2chars = 0;
    if (str.size == 0) return 0;
    
    if (str.size == 1)
        f2chars = str.arr[0] << 3*8;
    else
        f2chars = (str.arr[0]          << 0*8)
                | (str.arr[1]          << 1*8)
                | (str.arr[str.size-2] << 2*8)
                | (str.arr[str.size-1] << 3*8);
    
    return f2chars ^ seed;
}

uint32_t hash4(const Strv str, uint32_t seed)
{
    uint32_t f2chars = 0;
    if (str.size == 0) return 0;
    
    if (str.size == 1)
        f2chars = str.arr[0] << 3*8;
    else
        f2chars = (str.arr[0]          << 0*8)
                | (str.arr[1]          << 1*8)
                | (str.arr[str.size-2] << 2*8)
                | (str.arr[str.size-1] << 3*8);
    
    return (f2chars ^ seed) + seed;
}

uint32_t hash5(const Strv str, uint32_t seed)
{
    uint32_t f2chars = 0;
    if (str.size == 0) return 0;
    
    if (str.size == 1)
        f2chars = str.arr[0] << 3*8;
    else
        f2chars = (str.arr[0]          << 0*8)
                | (str.arr[1]          << 1*8)
                | (str.arr[str.size-2] << 2*8)
                | (str.arr[str.size-1] << 3*8);
    
    return f2chars + seed;
}

uint32_t hash6(const Strv str, uint32_t seed)
{
    uint32_t f2chars = 0;
    if (str.size == 0) return 0;
    
    if (str.size == 1)
        f2chars = str.arr[0] << 3*8;
    else
        f2chars = ((str.arr[0]          << 0*8)
                + (str.arr[1]          << 0*8))
                | ((str.arr[str.size-2] << 1*8)
                + (str.arr[str.size-1] << 1*8));
    
    return f2chars ^ seed;
}

uint32_t hash7(const Strv str, uint32_t seed)
{
    uint32_t f2chars = 0;
    if (str.size == 0) return 0;
    
    if (str.size == 1)
        f2chars = str.arr[0] << 3*8;
    else
        f2chars = ((str.arr[0]          << 0*8)
                ^ (str.arr[1]           << 1))
                | ((str.arr[str.size-2] << 1*8)
                ^ (str.arr[str.size-1]  << (1*8+1)));
    
    return f2chars ^ seed;
}

uint32_t hash8(const Strv str, uint32_t seed)
{
    return Strv_hash(str, seed);
}


const hash_t hashs[] = {
    hash0,
    // hash1,
    // hash2,
    // hash3,
    // hash4,
    // hash5,
    // hash6,
    // hash7,
    // hash8
};



typedef struct
{
    uint32_t seed;
    size_t area;
    size_t count;
    size_t hash_index;
} Strategie;


int Strategie_cmp(const void *a, const void *b)
{
    const Strategie *pa = a;
    const Strategie *pb = b;
    if (pa->count != pb->count)
        return (pa->count - pb->count);
    return (pa->area - pb->area);
}


DA_TYPEDEF_ARRAY(Strategie);
DA_TYPEDEF_ARRAY(int);


int count_for_strategie(size_t seed, size_t hash_index, da_int *slots, bool print_buckets)
{
    for (int i = 0; i < (int)ARRAY_LEN(keys); i++)
    {
        uint32_t h = hashs[hash_index](keys[i].name, seed) % slots->size;
        if (print_buckets) printf("%.*s\t-> %d\n", keys[i].name.size, keys[i].name.arr, h);
        slots->arr[h]++;
    }
    // if (print_buckets) printf("\n");
    int count  = 0;
    // printf("s.size %d\n", slots->size);
    for (int i = 0; i < slots->size; i++)
    {
        if (print_buckets) printf("(%d)%d, ", i, slots->arr[i]);

        if (slots->arr[i] > 1)
        {
            count += slots->arr[i];
        }
        
    }
    if (print_buckets) printf("\n");
    memset(slots->arr, 0, slots->size * sizeof(slots->arr[0]));

    return count;
}

uint32_t search_seed(void)
{
    da_int slots = {0};
    da_push_nzeros(&slots, ARRAY_LEN(keys));
    
    da_Strategie counts = {0};
    da_push(&counts, (Strategie){ .count = INT32_MAX, .area = slots.size, .seed = 1 });
    

    static_for (h_fun, hashs)
    {
        for (int area = 0;  area < 80; area++)
        {
            (void)primes;
            // for (uint32_t seed = 1; seed < ARRAY_LEN(primes); seed++)
            for (uint32_t seed = 1; seed < 10000; seed++)
            {
                // int count  = count_for_strategie(primes[seed], h_fun, &slots, false);
                int count  = count_for_strategie(seed, h_fun, &slots, false);
                
                da_push(&counts, (Strategie){ 
                    .hash_index = h_fun,
                    .seed = seed,
                    .count = count,
                    .area = slots.size
                });
                
                da_qsort(&counts, Strategie_cmp);
                
                counts.size = MIN(30, counts.size);
            }
            da_push_zero(&slots);
        }
        slots.size = ARRAY_LEN(keys);
    }


    da_for (Strategie, it, &counts)
        printf("s%u: c%zu a%zu fun%ld\n", it->seed, it->count, it->area, it->hash_index);
    

    return counts.arr[0].seed;
}

int _main(void)
{
    test_get_Primitive();

    // s9472: c1 a37 fun1
    // s2231: c2 a55 fun0
    // if (0)
    // {
        // da_int slots = {0};
        // da_push_nzeros(&slots, 55);
        // printf("-> %d\n", count_for_strategie(2231, 0, &slots, true));
    // }
    // else
        // search_seed();
    return 0;
}

*/
