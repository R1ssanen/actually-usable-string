#include "aus.h"

/**
 * @author github.com/R1ssanen
 * @brief Dynamic string + view implementation
 * @date 10.07.2026
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
AUS_INLINE char *get_data( au_string *str )
{
    return (char *)aus_data( str );
}

/**
 * @brief implementation
 */

void aus_free( au_string *str )
{
    assert( str != NULL );

    if ( !str->is_small )
    {
        aus_mem_free( str->opt.big.data );
    }

    memset( str, 0, sizeof( au_string ) );
}

void aus_clear( au_string *str )
{
    assert( str != NULL );

    str->size = 0;
    get_data( str )[0] = '\0';
}

/// @note does not initialize memory
aus_error aus_with_capacity( size_t size, au_string *out )
{
    assert( size > 0 );

    if ( size >= AUS_SSO_LIMIT )
    {
        char *data = aus_mem_alloc( size + 1 );
        if ( !data )
        {
            return AUS_ERR( "Could not allocate memory for creating au_string." );
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

aus_error aus_from_parts( const char *src, size_t size, au_string *out )
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

AUS_INLINE aus_error grow_to_accommodate( au_string *str, size_t new_size )
{
    if ( str->is_small )
    {
        if ( new_size >= AUS_SSO_LIMIT ) // switch to big mode
        {
            au_string new;
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
                return AUS_ERR( "Could not allocate memory for growing au_string." );
            }

            big->data = resized;
            big->capacity = new_cap;
        }
    }

    return AUS_OK;
}

aus_error aus_concat_view( au_string *dst, au_string_view src )
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

aus_error aus_shrink_to_fit( au_string *str )
{
    assert( str != NULL );

    if ( !str->is_small )
    {
        aus_big_internals_ *big = &str->opt.big;
        char *resized = aus_mem_realloc( big->data, str->size + 1 );
        if ( !resized )
        {
            return AUS_ERR( "Could not reallocate to shrink au_string." );
        }

        big->data = resized;
        big->capacity = str->size;
    }

    return AUS_OK;
}

aus_error aus_push( au_string *dst, char c )
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
