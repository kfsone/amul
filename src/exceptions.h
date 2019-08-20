#ifndef AMUL_EXCEPTIONS_H
#define AMUL_EXCEPTIONS_H

#include <stdexcept>

class EndThread : public std::exception
{
    const char *error{ nullptr };

  public:
    EndThread() noexcept : error{nullptr} {}
    EndThread(const char *reason) noexcept : error(reason) {}

    const char *what() const noexcept override { return error; }
};

#endif  // AMUL_EXCEPTIONS_H
