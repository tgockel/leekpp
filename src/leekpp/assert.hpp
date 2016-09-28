#pragma once

/** \addtogroup Configuration
 *  \{
**/

/** \def LEEK_ASSERT
 *  Used checking certain expressions and potentially doing something about it. By default, this function will throw an
 *  exception of type \c std::ex, but you can use it to do whatever you want by defining \c LEEK_ASSERT yourself.
 *  
 *  \param expr The expression that was checked. This should be convertable to bool.
 *  \param ex The exception type which would have been thrown if you would expect an assertion to throw an exception.
 *   The tokens here are limited to what you would find in the \c stdexcept header, but without the \c std:: prefix (for
 *   example, simply \c out_of_range and \c invalid_argument).
 *  \param message An informative message which would be perfectly valid to pass to a function like \c printf (if you
 *   know the proper preprocessor incantations to do so). Examples of the contents of this variable are
 *   <tt>("Invalid value %uz: must be between %zu and %zu", in_arg, lowest, highest)</tt>,
 *   <tt>("You look like you could use a %s", beer_name.c_str())</tt>, and
 *   <tt>("The C preprocessor is %f%% a pain", 100.0)</tt>.
 *   While this might seem really painful and insane to you, feel good knowing that probably everyone else (including
 *   the library author) agrees with you on that fact. Feel free to ignore this parameter if it pleases you.
 *  
 *  \see LEEK_ASSERT_THROW_STD
**/

/** \def LEEK_ASSERT_THROW_STD
 *  If set to \c 1, an assertion failure in \c LEEK_ASSERT macro should cause an exception to be thrown.
**/
#ifndef LEEK_ASSERT_THROW_STD
#   define LEEK_ASSERT_THROW_STD 1
#endif

/** \def LEEK_ASSERT_THROW_STD_MESSAGE_BUFFER_SIZE
 *  The maximum size of the error message to use when throwing an error. This is only applicable if
 *  \c LEEK_ASSERT_THROW_STD is on.
**/
#ifndef LEEK_ASSERT_THROW_STD_MESSAGE_BUFFER_SIZE
#   define LEEK_ASSERT_THROW_STD_MESSAGE_BUFFER_SIZE 256
#endif

#if !defined LEEK_ASSERT

#if LEEK_ASSERT_THROW_STD

#include <cstdio>
#include <stdexcept>
#include <string>

namespace leekpp
{
namespace detail
{

template <typename TException, typename... TArgs>
inline TException make_exception(const char* format, TArgs&&... args)
{
    char buffer[LEEK_ASSERT_THROW_STD_MESSAGE_BUFFER_SIZE];
    std::snprintf(buffer, sizeof buffer, format, args...);
    return TException(buffer);
}

}
}

#define LEEK_ASSERT(expr, ex, message)                                                                                 \
    while (!(expr))                                                                                                    \
        throw ::leekpp::detail::make_exception< ::std::ex> message

#else

#define LEEK_ASSERT(expr, ex, message) static_cast<void>(expr)

#endif

#endif

/** \} **/
