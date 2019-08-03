/* $Id: includes.hpp,v 1.8 1999/06/08 15:36:45 oliver Exp $
 * Standard/global includes
 */

#ifndef INCS_H
#define INCS_H 1

/* Most importantly, what has 'autoconf' taught us? */
#ifdef HAVE_CONFIG_H
#include "config.h" /* Generated by autoconf */
#else
#error "config.h is required - panic!"
#endif

/* Standard, system includes */

#include <cstdint>

#include "portable.hpp"

#endif /* INCS_H */
