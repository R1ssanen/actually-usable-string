#ifndef ACTUALLY_USABLE_STRING_H
#define ACTUALLY_USABLE_STRING_H

/**
 * @author github.com/R1ssanen
 * @brief Dynamic string + view implementation
 * @date 11.07.2026
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

typedef struct aus_str aus_str;
struct aus_str
{
    union {
        char small[AUS_SSO_LIMIT];
        aus_big_internals_ big;
    } opt;
    size_t size;
    bool is_small;
};

typedef struct aus_str_view aus_str_view;
struct aus_str_view
{
    const char *str;
    size_t size;
};

/**
 * @brief basic
 */

void aus_free( aus_str *str );
void aus_clear( aus_str *str );

AUS_INLINE size_t aus_size( const aus_str *str )
{
    return str->size;
}

AUS_INLINE const char *aus_data( const aus_str *str )
{
    return ( str->is_small ) ? str->opt.small : str->opt.big.data;
}

AUS_INLINE const char *aus_end( const aus_str *str )
{
    return aus_data( str ) + str->size;
}

AUS_INLINE bool aus_is_empty( const aus_str *str )
{
    return ( str->size == 0 );
}

/**
 * @brief construction
 */

/// @note does not initialize memory, or null-terminate
aus_error aus_with_capacity( size_t size, aus_str *out );

aus_error aus_from_parts( const char *src, size_t size, aus_str *out );

AUS_INLINE aus_error aus_from_cstr( const char *src, aus_str *out )
{
    return aus_from_parts( src, strlen( src ), out );
}

AUS_INLINE aus_error aus_from_str( const aus_str *src, aus_str *out )
{
    return aus_from_parts( aus_data( src ), src->size, out );
}

AUS_INLINE aus_error aus_from_view( aus_str_view src, aus_str *out )
{
    return aus_from_parts( src.str, src.size, out );
}

AUS_INLINE aus_str_view ausv_from_parts( const char *src, size_t size )
{
    return (aus_str_view){ .str = src, .size = size };
}

AUS_INLINE aus_str_view ausv_from_cstr( const char *src )
{
    return ausv_from_parts( src, strlen( src ) );
}

AUS_INLINE aus_str_view ausv_from_str( const aus_str *src )
{
    return ausv_from_parts( aus_data( src ), src->size );
}

/**
 * @brief element manipulation
 */

aus_error aus_join_view( aus_str *dst, aus_str_view src );

AUS_INLINE aus_error aus_join_str( aus_str *dst, const aus_str *src )
{
    return aus_join_view( dst, ausv_from_str( src ) );
}

AUS_INLINE aus_error aus_join_cstr( aus_str *dst, const char *cstr )
{
    return aus_join_view( dst, ausv_from_cstr( cstr ) );
}

aus_error aus_shrink_to_fit( aus_str *str );

aus_error aus_push( aus_str *dst, char c );

aus_error aus_pop_index( aus_str *str, size_t index, char *out );

bool aus_pop_front( aus_str *str, char *out );

bool aus_pop_back( aus_str *str, char *out );

/**
 * @brief utility
 */

bool ausv_matches_view( aus_str_view a, aus_str_view b );

AUS_INLINE bool ausv_matches_cstr( aus_str_view a, const char *b )
{
    return ausv_matches_view( a, ausv_from_cstr( b ) );
}

AUS_INLINE bool ausv_matches_str( aus_str_view a, const aus_str *b )
{
    return ausv_matches_view( a, ausv_from_str( b ) );
}

AUS_INLINE bool aus_matches_view( const aus_str *a, aus_str_view b )
{
    return ausv_matches_view( ausv_from_str( a ), b );
}

AUS_INLINE bool aus_matches_cstr( const aus_str *a, const char *b )
{
    return ausv_matches_view( ausv_from_str( a ), ausv_from_cstr( b ) );
}

AUS_INLINE bool aus_matches_str( const aus_str *a, const aus_str *b )
{
    return ausv_matches_view( ausv_from_str( a ), ausv_from_str( b ) );
}

const char *ausv_find_first( aus_str_view view, char c );

AUS_INLINE const char *aus_find_first( const aus_str *str, char c )
{
    return ausv_find_first( ausv_from_str( str ), c );
}

aus_str_view ausv_trim_front( aus_str_view view, const char *trim );

aus_str_view ausv_trim_back( aus_str_view view, const char *trim );

AUS_INLINE aus_str_view ausv_trim( aus_str_view view, const char *trim )
{
    return ausv_trim_front( ausv_trim_back( view, trim ), trim );
}

AUS_INLINE aus_str_view aus_trim_front( const aus_str *str, const char *trim )
{
    return ausv_trim_front( ausv_from_str( str ), trim );
}

AUS_INLINE aus_str_view aus_trim_back( const aus_str *str, const char *trim )
{
    return ausv_trim_back( ausv_from_str( str ), trim );
}

AUS_INLINE aus_str_view aus_trim( const aus_str *str, const char *trim )
{
    return ausv_trim_front( ausv_trim_back( ausv_from_str( str ), trim ), trim );
}

/**
 * @brief C11 convenience
 */

#if ( __STDC__ ) && ( __STDC_VERSION__ >= 201112L )

#define aus_from( x )                                                                                                  \
    _Generic( ( x ),                                                                                                   \
        char *: aus_from_cstr,                                                                                         \
        const char *: aus_from_cstr,                                                                                   \
        aus_str_view: aus_from_view,                                                                                   \
        aus_str *: aus_from_str )(( x ))

#define aus_join( x, y )                                                                                               \
    _Generic( ( y ),                                                                                                   \
        char *: aus_join_cstr,                                                                                         \
        const char *: aus_join_cstr,                                                                                   \
        aus_str_view: aus_join_view,                                                                                   \
        aus_str *: aus_join_str )( ( x ), ( y ) )

#define aus_matches( x, y )                                                                                            \
    _Generic( ( y ),                                                                                                   \
        char *: aus_matches_cstr,                                                                                      \
        const char *: aus_matches_cstr,                                                                                \
        aus_str_view: aus_matches_view,                                                                                \
        aus_str *: aus_matches_str )( ( x ), ( y ) )

#define ausv_matches( x, y )                                                                                           \
    _Generic( ( y ),                                                                                                   \
        char *: ausv_matches_cstr,                                                                                     \
        const char *: ausv_matches_cstr,                                                                               \
        aus_str_view: ausv_matches_view,                                                                               \
        aus_str *: ausv_matches_str )( ( x ), ( y ) )

#endif

#endif