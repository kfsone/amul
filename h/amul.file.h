#ifndef AMUL_H_AMUL_FILE_H
#define AMUL_H_AMUL_FILE_H

#include <h/amul.type.h>

#include <fcntl.h>
#include <string>
#include <string_view>
#include <utility>

#if defined(HAVE_UNISTD_H)
#include <unistd.h>
#elif defined(_MSC_VER)
#include <io.h>
#endif

#include <sys/stat.h>

constexpr int READ_FLAGS = O_RDONLY;
constexpr int WRITE_FLAGS = O_WRONLY | O_CREAT | O_TRUNC;
constexpr int FILE_PERMS = 0644;

#endif  // AMUL_H_AMUL_FILE_H
