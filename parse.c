#include "lisp.h"

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
    while (depth > 0)
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
        
        GOTRY_consume(&str);
    }

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



ssize_t list_count(Strv str)
{    
    GOTRY(Strv_first(str) == '(');
    GOTRY_consume(&str);
    skip_comment(&str);  // not needed ? 
    GOTRY(str.size > 0);


    ssize_t count = 0;
    while (Strv_first(str) != ')')
    {
        skip_comment(&str);
        GOTRY(str.size > 0);

        if (Strv_first(str) == '(')
        {
            GOTRY(skip_parent(&str));
            skip_comment(&str);
            GOTRY(str.size > 0);
            count++;
            continue;
        }
        if (Strv_first(str) == '"')  // string with escape character
        {
            do {
                GOTRY_consume(&str);
                if (Strv_first(str) == '\\')
                {
                    GOTRY_consume(&str);
                    GOTRY_consume(&str);
                }
            }
            while (str.size > 0 && Strv_first(str) != '"');
            GOTRY_consume(&str);
            skip_comment(&str);
            GOTRY(str.size > 0);
            count++;
            continue;
        }
        if (Strv_first(str) == '\'') // reference
        {
            do GOTRY_consume(&str); while (Strv_first(str) == '\'');
            continue;
            // skip_comment(&str);
            // GOTRY(str.size > 0, error_log("expected atom after reference (') got EOF"));
        }


        GOTRY(skip_atom(&str));
        skip_comment(&str);
        GOTRY(str.size > 0);
        count++;
    }

    return count;
fail:
    return -count;
}

void test_list_count(void)
{
    assert(list_count(Strv_lit("()")) == 0);
    assert(list_count(Strv_lit("(a)")) == 1);
    assert(list_count(Strv_lit("(a aa)")) == 2);
    assert(list_count(Strv_lit("(  dqfs  d dd)")) == 3);
    assert(list_count(Strv_lit("((dq dd dd) a)")) == 2);
    assert(list_count(Strv_lit("((dq \"(\" dd) a)")) == 2);
    assert(list_count(Strv_lit("(('dq '''a) a '(a bdd (aad d)))")) == 3);
    
}

List escaping(Strv str)
{
    char buffer[512];
    Strv res = Strv_make(
        (str.size < sizeof(buffer)) ? buffer : malloc(str.size), // fallback to malloc if too large
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
        .str = List_duplicate(NULL, res.arr, res.size)
    };
    if (str.size >= sizeof(buffer)) free(res.arr);
    return result;
}



bool list(Strv *str, List *li)
{
    TRY(li, error_log("no output list to parse"));
    TRY(Strv_first(*str) != ')', error_log("closing parent at root"));
    TRY(str->size > 0, error_log("empty input"));
    
    skip_space(str);
    skip_comment(str);

    if (Strv_first(*str) == '(')
    {
        li->tag = tag_list;
        ssize_t count = list_count(*str);
        TRY(count < 0, error_log("count error"));
        
        TRY(consume(str), error_log("EOF"));
        skip_comment(str);
        TRY(str->size > 0, error_log("EOF"));
        
        if (count == 0)
        {
            *li = NIL_LIST;
            Strv_inc(str); // can't be the end of file
            return true;
        }

        const Strv save = *str;
        
        li->list = List_alloc(NULL, count * sizeof(List));
        li->size = 0;
        do {
            li->list[li->size] = NIL_LIST;
            TRY(list(str, &li->list[li->size]));
            li->size++;
            skip_comment(str);
        } while (li->size < count && str->size > 0 && Strv_first(*str) != ')');
        TRY(li->size == count, error_log("invalid list element count, expected %d got %d with: %sv", count, li->size, &save));
        TRY(Strv_first(*str) == ')', error_log("unexpected EOF or underestimate list_count() counted %d but there is more", count));
        Strv_inc(str);
        return true;
    }
    if (isdigit(Strv_first(*str)) || (is_unary_sign(Strv_first(*str)) && isdigit(Strv_char_at(*str, 1))))
    {
        char *it = str->arr;
        char *end = &str->arr[str->size];

        *li = (List){
            .tag = tag_number,
            .number = strtod(str->arr, &it)
            // .number = strtoll(str->arr, &it, 10)
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
        .str = List_duplicate(NULL, symbole.arr, symbole.size),
        .size = symbole.size
    };
    
    
    return true;
}

bool lists(Strv str, List *li)
{
    skip_comment(&str);

    li->tag = tag_list;
    ssize_t count = list_count(str);
    TRY(count > 0, error_log("root count error"));
    li->list = List_alloc(NULL, count * sizeof(List));
    li->size = 0;

    while (li->size < count && str.size > 0);
    {
        li->list[li->size] = NIL_LIST;
        TRY(list(&str, &li->list[li->size]));
        li->size++;

        skip_comment(&str);
    }

    TRY(li->size == count, error_log("invalid list element count in root, expected %d got %d", count, li->size));
    return true;
}
