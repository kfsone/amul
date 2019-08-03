#ifndef TYPEDEFS_H
#define TYPEDEFS_H 1

#include <cstddef>

/* $Id: typedefs.hpp,v 1.5 1997/05/05 15:17:38 oliver Exp $
 *  Definitions for 'native types' so that things get typecast
 *  properly
 */

using string = const char;      // static string
using vocid_t = int32_t;        // id of a word from the vocab table
using msgno_t = int32_t;        // umsg number
using counter_t = int32_t;      // anything for counting
using arg_t = int16_t;          // compiler lang-table argument
using basic_obj = int32_t;      // bobj number
using container_t = int32_t;    // id of a container
using flag_t = uint32_t;        // bit-flag field
using ext_flag_t = uint64_t;    // extended bit-flag field
using playerid_t = uint32_t;    // id of a player
using playermask_t = uint16_t;  // a mask of online players

#endif /* TYPEDEFS_H */
