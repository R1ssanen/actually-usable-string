#include "aus.h"

/**
 * @author github.com/R1ssanen
 * @brief Dynamic string + view implementation
 * @date 11.07.2026
 * @file aus.c
 */

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#define AUS_MAX( a, b ) ( ( a ) < ( b ) ? ( b ) : ( a ) )

#define AUS_OK                                                                                                         \
    ( aus_error )                                                                                                      \
    {                                                                                                                  \
        .msg = NULL, .err = false                                                                                      \
    }

#define AUS_ERR( msg_ )                                                                                                \
    ( aus_error )                                                                                                      \
    {                                                                                                                  \
        .msg = ( msg_ ), .err = true                                                                                   \
    }

// casts const away
AUS_INLINE char *get_data( aus_str *str )
{
    return (char *)aus_data( str );
}

/**
 * @brief implementation
 */

void aus_free( aus_str *str )
{
    assert( str != NULL );

    if ( !str->is_small )
    {
        aus_mem_free( str->opt.big.data );
    }

    memset( str, 0, sizeof( aus_str ) );
}

void aus_clear( aus_str *str )
{
    assert( str != NULL );

    str->size = 0;
    get_data( str )[0] = '\0';
}

/// @note does not initialize memory
aus_error aus_with_capacity( size_t size, aus_str *out )
{
    assert( size > 0 );

    if ( size >= AUS_SSO_LIMIT )
    {
        char *data = aus_mem_alloc( size + 1 );
        if ( !data )
        {
            return AUS_ERR( "Could not allocate memory for creating aus_str." );
        }

        aus_big_internals_ *big = &out->opt.big;
        big->data = data;
        big->data[0] = '\0';
        big->capacity = size;
        out->is_small = false;
    }
    else
    {
        out->opt.small[0] = '\0';
        out->is_small = true;
    }

    out->size = size;
    return AUS_OK;
}

aus_error aus_from_parts( const char *src, size_t size, aus_str *out )
{
    assert( src != NULL );
    assert( size > 0 );

    aus_error e = aus_with_capacity( size, out );
    if ( e.err )
    {
        return e;
    }

    char *data = get_data( out );
    memcpy( data, src, size );
    data[size] = '\0';

    return AUS_OK;
}

static aus_error grow_to_accommodate( aus_str *str, size_t new_size )
{
    if ( str->is_small )
    {
        if ( new_size >= AUS_SSO_LIMIT ) // switch to big mode
        {
            aus_str new;
            aus_error e = aus_with_capacity( new_size, &new );
            if ( e.err )
            {
                return e;
            }

            aus_big_internals_ *big = &new.opt.big;
            memcpy( big->data, str->opt.small, str->size );

            new.size = str->size;
            *str = new;
        }
    }
    else
    {
        aus_big_internals_ *big = &str->opt.big;
        if ( big->capacity <= new_size )
        {
            const size_t new_cap = AUS_MAX( new_size, big->capacity * 2 );
            char *resized = aus_mem_realloc( big->data, new_cap + 1 );
            if ( !resized )
            {
                return AUS_ERR( "Could not allocate memory for growing aus_str." );
            }

            big->data = resized;
            big->capacity = new_cap;
        }
    }

    return AUS_OK;
}

aus_error aus_join_view( aus_str *dst, aus_str_view src )
{
    assert( dst != NULL );
    assert( src.str != NULL );
    assert( src.size > 0 );

    const size_t total_size = dst->size + src.size;
    aus_error e = grow_to_accommodate( dst, total_size );
    if ( e.err )
    {
        return e;
    }

    char *data = get_data( dst );
    memcpy( data + dst->size, src.str, src.size );
    data[total_size] = '\0';
    dst->size = total_size;

    return AUS_OK;
}

aus_error aus_shrink_to_fit( aus_str *str )
{
    assert( str != NULL );

    if ( !str->is_small )
    {
        aus_big_internals_ *big = &str->opt.big;
        char *resized = aus_mem_realloc( big->data, str->size + 1 );
        if ( !resized )
        {
            return AUS_ERR( "Could not reallocate to shrink aus_str." );
        }

        big->data = resized;
        big->capacity = str->size;
    }

    return AUS_OK;
}

aus_error aus_push( aus_str *dst, char c )
{
    assert( dst != NULL );

    const size_t new_size = dst->size + 1;
    aus_error e = grow_to_accommodate( dst, new_size );
    if ( e.err )
    {
        return e;
    }

    char *data = get_data( dst );
    data[dst->size] = c;
    data[new_size] = '\0';
    dst->size = new_size;

    return AUS_OK;
}

aus_error aus_pop_index( aus_str *str, size_t index, char *out )
{
    assert( str != NULL );

    if ( index >= str->size )
    {
        return AUS_ERR( "Pop index out of bounds." );
    }
    else
    {
        char *data = get_data( str );
        *out = data[index];
        memmove( data + index, data + index + 1, ( str->size-- ) - index );
        return AUS_OK;
    }
}

bool aus_pop_front( aus_str *str, char *out )
{
    return !aus_pop_index( str, 0, out ).err;
}

bool aus_pop_back( aus_str *str, char *out )
{
    assert( str != NULL );

    if ( aus_is_empty( str ) )
    {
        return false;
    }
    else
    {
        char *data = get_data( str );
        *out = data[str->size - 1];
        data[( str->size-- ) - 1] = '\0';
        return true;
    }
}

bool ausv_matches_view( aus_str_view a, aus_str_view b )
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

const char *ausv_find_first( aus_str_view view, char c )
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

aus_str_view ausv_trim_front( aus_str_view view, const char *trim )
{
    aus_str_view trimset = ausv_from_cstr( trim );
    aus_str_view copy = view;

    for ( size_t i = 0; i < view.size; ++i )
    {
        if ( ausv_find_first( trimset, view.str[i] ) == NULL ) // not in trim set
        {
            break;
        }
        else
        {
            copy.str++;
            copy.size--;
        }
    }

    return copy;
}

aus_str_view ausv_trim_back( aus_str_view view, const char *trim )
{
    aus_str_view trimset = ausv_from_cstr( trim );

    for ( ; view.size > 0; --view.size )
    {
        if ( ausv_find_first( trimset, view.str[view.size - 1] ) == NULL ) // not in trim set
        {
            break;
        }
    }

    return view;
}
