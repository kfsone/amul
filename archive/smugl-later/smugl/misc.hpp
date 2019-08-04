#ifndef SMUGL_SMUGL_MISC_H
#define SMUGL_SMUGL_MISC_H

struct fileInfo {
    const char *name;
    size_t size;
};

size_t filesize(const char *filename);
fileInfo *locate_file(const char *name, bool it_matters);
long read_file(const char *name, void *&base, bool it_matters);
void pressret();
void ShowFile(const char *s);

#endif  // SMUGL_SMUGL_MISC_H
