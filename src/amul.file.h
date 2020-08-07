#ifndef AMUL_FILE_H
#define AMUL_FILE_H

#include "amulconfig.h"
#include <fcntl.h>
#include <string>
#include <string_view>
#include <sys/stat.h>
#include <utility>

#include "amul.typedefs.h"

#if defined(HAVE_UNISTD_H)
#include <unistd.h>
#elif defined(_MSC_VER)
#include <io.h>
#endif

#if defined(O_BINARY)
constexpr int READ_FLAGS = O_RDONLY | O_BINARY;
constexpr int WRITE_FLAGS = O_WRONLY | O_CREAT | O_TRUNC | O_BINARY;
#else
constexpr int READ_FLAGS = O_RDONLY;
constexpr int WRITE_FLAGS = O_WRONLY | O_CREAT | O_TRUNC;
#endif
constexpr int FILE_PERMS = 0644;

#endif  // AMUL_FILE_H
