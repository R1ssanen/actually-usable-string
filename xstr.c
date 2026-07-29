#include "xstr.h"

/**
 * @author github.com/R1ssanen
 * @brief Dynamic string + view implementation
 * @date 27.07.2026
 * @file xstr.c
 */

#include <assert.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SSO_LIMIT ( (size_t)16 )
#define MAX( a, b ) ( ( a ) < ( b ) ? ( b ) : ( a ) )

#define INVALID                                                                                                        \
    ( xstr )                                                                                                           \
    {                                                                                                                  \
        .state = XSTR_INVALID                                                                                          \
    }

/**
 * @brief implementation
 */

void xstr_free( xstr *str )
{
    assert( str != NULL );
    assert( str->state != XSTR_INVALID );

    if ( str->state == XSTR_BIG )
    {
        xstr_mem_free( str->opt.big.data );
    }

    memset( str, 0, sizeof( xstr ) );
}

/// @note does not initialize memory
xstr xstr_with_capacity( size_t capacity, const char **error )
{
    assert( capacity > 0 );
    xstr str = INVALID;

    if ( capacity >= SSO_LIMIT )
    {
        char *data = xstr_mem_alloc( capacity + 1 );
        if ( !data )
        {
            if ( error )
            {
                *error = "Could not allocate memory for creating xstr.";
            }
            return str;
        }

        internal_big_ *big = &str.opt.big;
        big->data = data;
        big->capacity = capacity;
        str.state = XSTR_BIG;
    }
    else
    {
        str.state = XSTR_SMALL;
    }

    str.size = 0;
    return str;
}

xstr xstr_from_parts( const char *src, size_t size, const char **error )
{
    assert( src != NULL );
    assert( size > 0 );

    xstr str = xstr_with_capacity( size, error );
    if ( !xstr_invalid( &str ) )
    {
        char *data = xstr_data( &str );
        memcpy( data, src, size );
        data[size] = '\0';
        str.size = size;
    }

    return str;
}

bool xstr_ensure_capacity( xstr *str, size_t new_capacity, const char **error )
{
    if ( str->state == XSTR_SMALL )
    {
        if ( new_capacity >= SSO_LIMIT ) // switch to big mode
        {
            xstr new = xstr_with_capacity( new_capacity, error );
            if ( xstr_invalid( &new ) )
            {
                return false;
            }

            internal_big_ *big = &new.opt.big;
            memcpy( big->data, str->opt.small, str->size );

            new.size = str->size;
            *str = new;
        }
    }
    else
    {
        internal_big_ *big = &str->opt.big;
        if ( big->capacity <= new_capacity )
        {
            new_capacity = MAX( new_capacity, big->capacity * 2 );
            char *resized = xstr_mem_realloc( big->data, new_capacity + 1 );
            if ( !resized )
            {
                if ( error )
                {
                    *error = "Could not allocate memory for growing xstr.";
                }
                return false;
            }

            big->data = resized;
            big->capacity = new_capacity;
        }
    }

    return true;
}

bool xstr_join_view( xstr *dst, xview src, const char **error )
{
    assert( dst != NULL );
    assert( src.str != NULL );
    assert( src.size > 0 );

    const size_t total_size = dst->size + src.size;
    if ( !xstr_ensure_capacity( dst, total_size, error ) )
    {
        return false;
    }

    char *data = xstr_data( dst );
    memcpy( data + dst->size, src.str, src.size );
    data[total_size] = '\0';
    dst->size = total_size;

    return true;
}

bool xstr_shrink_to_fit( xstr *str, const char **error )
{
    assert( str != NULL );

    if ( str->state == XSTR_BIG )
    {
        internal_big_ *big = &str->opt.big;
        char *resized = xstr_mem_realloc( big->data, str->size + 1 );
        if ( !resized )
        {
            if ( error )
            {
                *error = "Could not reallocate to shrink xstr.";
            }
            return false;
        }

        big->data = resized;
        big->capacity = str->size;
    }

    return true;
}

bool xstr_push( xstr *dst, char c, const char **error )
{
    assert( dst != NULL );

    const size_t new_size = dst->size + 1;
    if ( !xstr_ensure_capacity( dst, new_size, error ) )
    {
        return false;
    }

    char *data = xstr_data( dst );
    data[dst->size] = c;
    data[new_size] = '\0';
    dst->size = new_size;

    return true;
}

bool xstr_pop_index( xstr *str, size_t index, char *out, const char **error )
{
    assert( str != NULL );

    if ( index >= str->size )
    {
        if ( error )
        {
            *error = "Pop index out of bounds.";
        }
        return false;
    }

    char *data = xstr_data( str );
    *out = data[index];
    memmove( data + index, data + index + 1, ( str->size-- ) - index );

    return true;
}

bool xstr_pop_front( xstr *str, char *out )
{
    return xstr_pop_index( str, 0, out, NULL );
}

bool xstr_pop_back( xstr *str, char *out )
{
    assert( str != NULL );

    if ( xstr_empty( str ) )
    {
        return false;
    }

    char *data = xstr_data( str );
    *out = data[str->size - 1];
    data[( str->size-- ) - 1] = '\0';

    return true;
}

bool xstr_append_formatted( xstr *str, const char **error, const char *fmt, ... )
{
    va_list args;
    va_start( args, fmt );

    const size_t new_size = str->size + (size_t)vsnprintf( NULL, 0, fmt, args );
    if ( !xstr_ensure_capacity( str, new_size, error ) )
    {
        va_end( args );
        return false;
    }

    char *data = xstr_data( str );
    vsprintf( data + str->size, fmt, args );
    va_end( args );

    data[new_size] = '\0';
    str->size = new_size;

    return true;
}

bool xview_matches_view( xview a, xview b )
{
    if ( a.size != b.size )
    {
        return false;
    }

    for ( size_t i = 0; i < a.size; ++i )
    {
        if ( a.str[i] != b.str[i] )
        {
            return false;
        }
    }

    return true;
}

const char *xview_find_first( xview view, char c )
{
    for ( const char *p = view.str; p < ( view.str + view.size ); ++p )
    {
        if ( *p == c )
        {
            return p;
        }
    }

    return NULL;
}

xview xview_trim_front( xview view, const char *trim )
{
    xview trimset = xview_from_cstr( trim );
    xview copy = view;

    for ( size_t i = 0; i < view.size; ++i )
    {
        if ( !xview_find_first( trimset, view.str[i] ) ) // not in trim set
        {
            break;
        }

        copy.str++;
        copy.size--;
    }

    return copy;
}

xview xview_trim_back( xview view, const char *trim )
{
    xview trimset = xview_from_cstr( trim );

    for ( ; view.size > 0; --view.size )
    {
        if ( !xview_find_first( trimset, view.str[view.size - 1] ) ) // not in trim set
        {
            break;
        }
    }

    return view;
}
