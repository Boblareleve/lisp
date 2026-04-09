#include "lisp.h"

Ar arena = {0};

#define GOTRY_consume(str) GOTRY(consume(str), error_log("unexpected EOF"))
bool consume(Strv *str)
{
    Strv_inc(str);
    return Strv_first(*str) != '\0';
}
void skip_space(Strv *str)
{
    while (isspace(Strv_first(*str)))
        Strv_inc(str);
}
void skip_comment(Strv *str)
{
    skip_space(str);

    while (Strv_first(*str) == ';')
    {
        Strv_c_substr(str, '\n');
        skip_space(str);
    }
}


int is_end(int c)     { return c == '(' || c == ')' || isspace(c); }
int not_is_end(int c) { return !is_end(c); }



bool is_unary_sign(char c)
{
    return c == '+' || c == '-';
}

// (()(()))
// return char after closing
// (Strv){0} on error
Strv get_pair(Strv str)
{
    Strv res = {0};
    GOTRY(Strv_first(str) == '(');

    GOTRY_consume(&str);
    int depth = 1;
    while (depth > 0 && str.size > 0)
    {
        skip_comment(&str);
        if (Strv_first(str) == '(')
            depth++;
        else if (Strv_first(str) == ')')
            depth--;
        else if (Strv_first(str) == '"')
        { // skip string
            do {
                // skip escaped
                if (Strv_first(str) == '\\')
                    GOTRY_consume(&str);
                // advance
                GOTRY_consume(&str);
            } while (Strv_first(str) != '"');
        }
        
        Strv_inc(&str);
    }
    GOTRY(depth == 0);
    
    res = str;
fail:
    return res;
}

bool skip_parent(Strv *str)
{
    *str = get_pair(*str);
    return str->arr != NULL;
}

bool skip_atom(Strv *str)
{
    while (str->size > 0 && !is_end(Strv_first(*str)))
        Strv_inc(str);
    return true;
}



List escaping(Strv str)
{
    char buffer[512];
    buffer[0] = 0;
    Strv res = Strv_make(
        (str.size < (int)sizeof(buffer)) ? buffer : malloc(str.size), // fallback to malloc if too large
        0
    );

    for (int i = 0; i < str.size; i++)
    {
        if (str.arr[i] == '\\')
        {
            i++;
            assert(i < str.size);
            switch (str.arr[i])
            {
            case '\\': res.arr[res.size++] = '\\'; break;
            case 'n':  res.arr[res.size++] = '\n'; break;
            case 't':  res.arr[res.size++] = '\t'; break;
            case 'r':  res.arr[res.size++] = '\r'; break;
            case 'v':  res.arr[res.size++] = '\v'; break;
            case 'a':  res.arr[res.size++] = '\a'; break;
            case 'b':  res.arr[res.size++] = '\b'; break;
            case 'f':  res.arr[res.size++] = '\f'; break;
            default:   res.arr[res.size++] = '?';  break; // unkown
            }
            continue;
        }
        res.arr[res.size++] = str.arr[i];
    }

    List result = {
        .tag = tag_string,
        .size = res.size,
        .str = List_duplicate(res.arr, res.size)
    };
    if (str.size >= (int)sizeof(buffer)) free(res.arr);
    return result;
}


bool list(Strv *str, List *li)
{
    TRY(li, error_log("no output list to parse"));
    TRY(Strv_first(*str) != ')', error_log("closing parent at root"));
    TRY(str->size > 0, error_log("empty input"));
    
    skip_comment(str);
    
    if (Strv_first(*str) == '(')
    {
        li->tag = tag_list;
        
        TRY(consume(str), error_log("EOF"));
        skip_comment(str);
        TRY(str->size > 0, error_log("EOF"));
        
        if (Strv_first(*str) == ')')
        {
            *li = NIL_LIST;
            Strv_inc(str); // can't be the end of file
            return true;
        }

        Ar_save_point save = Ar_save(&arena);
        
        int capacity = 1;
        li->list = Ar_calloc(&arena, capacity * sizeof(List));
        li->size = 0;
        do {
            if (capacity < li->size + 1)
            {
                li->list = Ar_crealloc(&arena, li->list, capacity * sizeof(List), (capacity + 4) * sizeof(List));
                capacity += 4;
            }
            
            assert(li->list);
            TRY(list(str, &li->list[li->size]));
            li->size++;
            skip_comment(str);
        } while (str->size > 0 && Strv_first(*str) != ')');
        Strv_inc(str);

        li->list = List_duplicate(li->list, li->size * sizeof(List));
        Ar_restore(&arena, save);

        return true;
    }
    if (isdigit(Strv_first(*str)) || (is_unary_sign(Strv_first(*str)) && isdigit(Strv_char_at(*str, 1))))
    {
        char *it = str->arr;
        char *end = &str->arr[str->size];

        *li = (List){
            .tag = tag_integer,
            .integer = strtoll(str->arr, &it, 10)
        };
        if (*it == '.')
            *li = (List){
                .tag = tag_real,
                .real = strtod(str->arr, &it)
            };
        
        if (it != str->arr)
        {
            *str = Strv_range(it, end); 
            return true;
        }
    }
    if (Strv_first(*str) == '"')
    {
        TRY(consume(str));
        char *begin = str->arr;
        while (Strv_first(*str) != '"')
        {
            TRY(consume(str));
            if (Strv_first(*str) == '\\')
            {
                TRY(consume(str));
                TRY(consume(str));
            }
        }

        *li = escaping(Strv_range(begin, str->arr));
        Strv_inc(str);
        return true;
    }
    if (Strv_first(*str) == '\'')
    {
        // count ref depth
        uint32_t count = 1;

        do TRY(consume(str)); while (Strv_first(*str) == '\'');
        
        TRY(list(str, li));
        li->quote_count = count;
        
        return true;
    }
    
    // any symbole (yes can be any character)
    Strv symbole = Strv_fun_substr(str, not_is_end);


    // special case true
    if (Strv_equal_lit(symbole, "t"))
    {
        *li = (List){ .tag = tag_true };
        return true;
    }
    
    
    *li = (List){
        .tag = tag_symbole,
        .str = List_duplicate(symbole.arr, symbole.size),
        .size = symbole.size
    };
    return true;
}

bool lists(Strv str, List *li)
{
    Ar_save_point save = Ar_save(&arena);

    skip_comment(&str);

    li->tag = tag_list;
    
    int capacity = 1;
    li->list = Ar_calloc(&arena, capacity * sizeof(List));
    li->size = 0;
    while (str.size > 0)
    {
        if (capacity < li->size + 1)
        {
            li->list = Ar_crealloc(&arena, li->list, capacity * sizeof(List), (capacity + 4) * sizeof(List));
            capacity += 4;
        }
        
        TRY(list(&str, &li->list[li->size]));
        li->size++;

        skip_comment(&str);
    }

    li->list = List_duplicate(li->list, li->size * sizeof(List));

    Ar_restore(&arena, save);
    return true;
}
