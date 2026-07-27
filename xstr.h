#ifndef XSTR_H
#define XSTR_H

/**
 * @author github.com/R1ssanen
 * @brief Dynamic string + view implementation
 * @date 27.07.2026
 * @file xstr.h
 */

#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#ifndef XSTR_INLINE
#define XSTR_INLINE static inline
#endif

#ifdef XSTR_CUSTOM_ALLOC
extern void *xstr_mem_alloc( size_t size );
extern void *xstr_mem_realloc( void *ptr, size_t size );
extern void xstr_mem_free( void *ptr );
#else
#define xstr_mem_alloc( count_ ) malloc(( count_ ))
#define xstr_mem_realloc( ptr_, count_ ) realloc( ( ptr_ ), ( count_ ) )
#define xstr_mem_free( ptr_ ) free(( ptr_ ))
#endif

typedef enum xstr_state
{
    XSTR_INVALID,
    XSTR_SMALL,
    XSTR_BIG,
} xstr_state;

typedef struct internal_big_ internal_big_;
struct internal_big_
{
    char *data;
    size_t capacity;
};

typedef struct xstr xstr;
struct xstr
{
    union {
        char small[16];
        internal_big_ big;
    } opt;
    size_t size;
    xstr_state state;
};

typedef struct xview xview;
struct xview
{
    const char *str;
    size_t size;
};

/**
 * @brief basic
 */

void xstr_free( xstr *str );

XSTR_INLINE char *xstr_data( xstr *str )
{
    return ( str->state == XSTR_SMALL ) ? str->opt.small : str->opt.big.data;
}

XSTR_INLINE const char *xstr_readonly( const xstr *str )
{
    return ( str->state == XSTR_SMALL ) ? str->opt.small : str->opt.big.data;
}

XSTR_INLINE void xstr_clear( xstr *str )
{
    str->size = 0;
    xstr_data( str )[0] = '\0';
}

XSTR_INLINE bool xstr_invalid( const xstr *str )
{
    return ( str->state == XSTR_INVALID );
}

XSTR_INLINE size_t xstr_size( const xstr *str )
{
    return str->size;
}

XSTR_INLINE char *xstr_end( xstr *str )
{
    return xstr_data( str ) + str->size;
}

XSTR_INLINE bool xstr_empty( const xstr *str )
{
    return ( str->size == 0 );
}

/**
 * @brief construction
 */

bool xstr_ensure_capacity( xstr *str, size_t new_capacity, const char **error );

/// @note does not initialize memory, or null-terminate
xstr xstr_with_capacity( size_t capacity, const char **error );

xstr xstr_from_parts( const char *src, size_t size, const char **error );

XSTR_INLINE xstr xstr_from_cstr( const char *src, const char **error )
{
    return xstr_from_parts( src, strlen( src ), error );
}

XSTR_INLINE xstr xstr_from_str( const xstr *src, const char **error )
{
    return xstr_from_parts( xstr_data( (xstr *)src ), src->size, error );
}

XSTR_INLINE xstr xstr_from_view( xview src, const char **error )
{
    return xstr_from_parts( src.str, src.size, error );
}

XSTR_INLINE xview xview_from_parts( const char *src, size_t size )
{
    return (xview){ .str = src, .size = size };
}

XSTR_INLINE xview xview_from_cstr( const char *src )
{
    return xview_from_parts( src, strlen( src ) );
}

XSTR_INLINE xview xview_from_str( const xstr *src )
{
    return xview_from_parts( xstr_data( (xstr *)src ), src->size );
}

/**
 * @brief element manipulation
 */

bool xstr_join_view( xstr *dst, xview src, const char **error );

XSTR_INLINE bool xstr_join_str( xstr *dst, const xstr *src, const char **error )
{
    return xstr_join_view( dst, xview_from_str( src ), error );
}

XSTR_INLINE bool xstr_join_cstr( xstr *dst, const char *cstr, const char **error )
{
    return xstr_join_view( dst, xview_from_cstr( cstr ), error );
}

bool xstr_shrink_to_fit( xstr *str, const char **error );

bool xstr_push( xstr *dst, char c, const char **error );

bool xstr_pop_index( xstr *str, size_t index, char *out, const char **error );

bool xstr_pop_front( xstr *str, char *out );

bool xstr_pop_back( xstr *str, char *out );

bool xstr_append_formatted( xstr *str, const char **error, const char *fmt, ... );

/**
 * @brief utility
 */

bool xview_matches_view( xview a, xview b );

XSTR_INLINE bool xview_matches_cstr( xview a, const char *b )
{
    return xview_matches_view( a, xview_from_cstr( b ) );
}

XSTR_INLINE bool xview_matches_str( xview a, const xstr *b )
{
    return xview_matches_view( a, xview_from_str( b ) );
}

XSTR_INLINE bool xstr_matches_view( const xstr *a, xview b )
{
    return xview_matches_view( xview_from_str( a ), b );
}

XSTR_INLINE bool xstr_matches_cstr( const xstr *a, const char *b )
{
    return xview_matches_view( xview_from_str( a ), xview_from_cstr( b ) );
}

XSTR_INLINE bool xstr_matches_str( const xstr *a, const xstr *b )
{
    return xview_matches_view( xview_from_str( a ), xview_from_str( b ) );
}

const char *xview_find_first( xview view, char c );

XSTR_INLINE const char *xstr_find_first( const xstr *str, char c )
{
    return xview_find_first( xview_from_str( str ), c );
}

xview xview_trim_front( xview view, const char *trim );

xview xview_trim_back( xview view, const char *trim );

XSTR_INLINE xview xview_trim( xview view, const char *trim )
{
    return xview_trim_front( xview_trim_back( view, trim ), trim );
}

XSTR_INLINE xview xstr_trim_front( const xstr *str, const char *trim )
{
    return xview_trim_front( xview_from_str( str ), trim );
}

XSTR_INLINE xview xstr_trim_back( const xstr *str, const char *trim )
{
    return xview_trim_back( xview_from_str( str ), trim );
}

XSTR_INLINE xview xstr_trim( const xstr *str, const char *trim )
{
    return xview_trim_front( xview_trim_back( xview_from_str( str ), trim ), trim );
}

#endif