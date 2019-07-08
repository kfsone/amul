#pragma once

#include <cstdint>
#include <cstdio>
#include <string>

void              close_ofps();
char              nextc(bool required);
[[noreturn]] void quit();
FILE *            fopenw(std::string filename);
FILE *            fopena(std::string filename);
FILE *            fopenr(std::string filename);
FILE *            rfopen(std::string filename);
void              ttroomupdate();
void              opentxt(std::string filename);
void              skipblock();
void              tidy(char *s);
int               is_verb(std::string token);
int               verbCount() noexcept;

struct Buffer final {
    Buffer() {}
    Buffer(size_t offset) { open(offset); }
    ~Buffer();
    void   open(size_t offset);
    void   free();
    size_t m_size{0};
    void * m_data{nullptr};
};

int32_t filesize();

template <typename T>
auto
fwritesafe(const T &value, FILE *fp)
{
    return fwrite(&value, sizeof(value), 1, fp);
}

template <typename T>
auto
freadsafe(T &value, FILE *fp)
{
    return fread(&value, sizeof(value), 1, fp);
}
