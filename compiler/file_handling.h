#pragma once

#include <cstdint>
#include <cstdio>

void              close_ofps();
char              nextc(bool required);
[[noreturn]] void quit();
FILE *            fopenw(const char *s);
FILE *            fopena(const char *s);
FILE *            fopenr(const char *s);
FILE *            rfopen(const char *s);
void              ttroomupdate();
void              opentxt(const char *s);
void              skipblock();
void              tidy(char *s);
int               is_verb(const char *s);

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