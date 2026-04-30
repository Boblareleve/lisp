

#include <stdlib.h>
#include "Str.h"
#include <assert.h>

typedef struct List
{
    char tag;

    unsigned short size;

    char *str;
} List;

void *List_duplicate(const void *src, size_t count)
{
    void *mem = malloc(count);
    assert(mem);
    return memcpy(mem, src, count);
}

List foo(Strv str)
{
    char buffer[512];
    buffer[0] = 0;
    Strv res = Strv_make(
        (str.size < (int)sizeof(buffer)) ? buffer : malloc(str.size), // fallback to malloc if too large
        0
    );
    assert(res.arr);

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
        .tag = 3,
        .size = res.size,
        .str = List_duplicate(res.arr, res.size)
    };
    if (str.size >= (int)sizeof(buffer)) free(res.arr);
    return result;
}
