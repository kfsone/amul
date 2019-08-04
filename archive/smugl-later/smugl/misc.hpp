#pragma once
// This may look like C, but it's really -*- C++ -*-

struct fileInfo {
    const char* name;
    size_t size;
};

extern size_t filesize(const char* filename);
extern fileInfo* locate_file(const char* name, bool it_matters);
extern long read_file(const char* name, void*& base, bool it_matters);
extern void pressret(void);
extern void ShowFile(const char* s);
