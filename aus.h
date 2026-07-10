#ifndef ACTUALLY_USABLE_STRING_H
#define ACTUALLY_USABLE_STRING_H

/**
 * @author github.com/R1ssanen
 * @brief Dynamic string + view implementation
 * @date 10.07.2026
 * @file aus.h
 */

#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#ifndef AUS_INLINE
#define AUS_INLINE static inline
#endif

#ifndef AUS_SSO_LIMIT
#define AUS_SSO_LIMIT ( (size_t)16 )
#endif

#ifdef AUS_CUSTOM_ALLOC
extern void *aus_mem_alloc( size_t size );
extern void *aus_mem_realloc( void *ptr, size_t size );
extern void aus_mem_free( void *ptr );
#else
#define aus_mem_alloc( count_ ) malloc(( count_ ))
#define aus_mem_realloc( ptr_, count_ ) realloc( ( ptr_ ), ( count_ ) )
#define aus_mem_free( ptr_ ) free(( ptr_ ))
#endif

typedef struct aus_error aus_error;
struct aus_error
{
    const char *msg;
    bool err;
};

typedef struct aus_big_internals_ aus_big_internals_;
struct aus_big_internals_
{
    char *data;
    size_t capacity;
};

typedef struct au_string au_string;
struct au_string
{
    union {
        char small[AUS_SSO_LIMIT];
        aus_big_internals_ big;
    } opt;
    size_t size;
    bool is_small;
};

void aus_free( au_string *str );
void aus_clear( au_string *str );

AUS_INLINE size_t aus_size( const au_string *str )
{
    return str->size;
}

AUS_INLINE const char *aus_data( const au_string *str )
{
    return ( str->is_small ) ? str->opt.small : str->opt.big.data;
}

typedef struct au_string_view au_string_view;
struct au_string_view
{
    const char *str;
    const size_t size;
};

/**
 * @brief construction
 */

aus_error aus_with_capacity( size_t size, au_string *out );

aus_error aus_from_parts( const char *src, size_t size, au_string *out );

AUS_INLINE aus_error aus_from_cstr( const char *src, au_string *out )
{
    return aus_from_parts( src, strlen( src ), out );
}

AUS_INLINE aus_error aus_from_view( au_string_view src, au_string *out )
{
    return aus_from_parts( src.str, src.size, out );
}

AUS_INLINE aus_error aus_clone( const au_string *src, au_string *out )
{
    return aus_from_parts( aus_data( src ), src->size, out );
}

AUS_INLINE au_string_view ausv_from_parts( const char *src, size_t size )
{
    return (au_string_view){ .str = src, .size = size };
}

AUS_INLINE au_string_view ausv_from_cstr( const char *src )
{
    return ausv_from_parts( src, strlen( src ) );
}

AUS_INLINE au_string_view ausv_from_string( const au_string *src )
{
    return ausv_from_parts( aus_data( src ), src->size );
}

/**
 * @brief utility
 */

aus_error aus_concat_view( au_string *dst, au_string_view src );

AUS_INLINE aus_error aus_concat( au_string *dst, const au_string *src )
{
    return aus_concat_view( dst, ausv_from_string( src ) );
}

AUS_INLINE aus_error aus_concat_cstr( au_string *dst, const char *cstr )
{
    return aus_concat_view( dst, ausv_from_cstr( cstr ) );
}

aus_error aus_shrink_to_fit( au_string *str );

aus_error aus_push( au_string *dst, char c );

/**
 * @brief C11 convenience
 */

#if ( __STDC__ ) && ( __STDC_VERSION__ >= 201112L )

#define aus_from( x )                                                                                                  \
    _Generic( ( x ),                                                                                                   \
        char *: aus_from_cstr,                                                                                         \
        const char *: aus_from_cstr,                                                                                   \
        au_string_view: aus_from_view,                                                                                 \
        au_string *: aus_clone )(( x ))

#define aus_join( x, y )                                                                                               \
    _Generic( ( y ),                                                                                                   \
        char *: aus_concat_cstr,                                                                                       \
        const char *: aus_concat_cstr,                                                                                 \
        au_string_view: aus_concat_view,                                                                               \
        au_string *: aus_concat )( ( x ), ( y ) )

#endif

#endif