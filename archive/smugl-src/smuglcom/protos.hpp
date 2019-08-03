// This may look like C, but it's really -*- C++ -*-
// protos.hpp -- function declarations / prototypes
// $Id: protos.hpp,v 1.8 1997/05/22 02:21:39 oliver Exp $

#ifndef SMUGLCOM_PROTOS
#define SMUGLCOM_PROTOS 1

#include "typedefs.hpp"

int is_room_flag(const char *s);
int is_room_param(char *&s);
int isoflag1(const char *s);
int isoparm(void);
int isoflag2(const char *s);
void checksys(void);
int iscond(const char *s);
int isact(const char *s);
void section(int i);
void error(const char *s, ...);
void warne(const char *s, ...);
void errabort(void);
/*
int higerhalph(char *s1, char *s2);
void sort_objs(void);
*/
void close_ofps(void);
int nextc(int);
void quit(const char *s, ...);
void fopenw(const char *s);
void fopena(const char *s);
void fopenr(const char *s);
void Err(const char *s, const char *t);
struct FILE *rfopen(const char *s);
void ttroomupdate(void);
void skipblock(void);
char *skipdata(char *p);
void tidy(char *s);
void clean_trim(char *s);
int is_verb(const char *s);
char *blkget(void);
char *cleanget(off_t off = 0);
char *text_proc(char *p, FILE *destfp);
void lang_proc(void);
int actualval(const char *, arg_t);
char *precon(char *);
char *preact(char *);
char *chkaparms(char *, int);
char *chkcparms(char *, int);
void umsg_proc(void);
void sys_proc(void);
void room_proc(void);
void finish_rooms(void);
void smsg_proc(void);
void mob_proc1(void);
void obds_proc(void);
void objs_proc(void);
void trav_proc(void);
void syn_proc(void);
void opentxt(const char *s);
int isnoun(const char *s);
msgno_t ttumsgchk(char *s);
char *getword(char *s);
char *skiplead(const char *skip, char *in);
char *skipline(char *s);
void repspc(char *s);
void clean_up(char *p);
void tx(const char *s);
void get_line(FILE *fp, char *into, int limit);
void *grow(void *ptr, size_t size, const char *msg);
void save_messages(void);
msgno_t add_msg(const char *);
msgno_t ismsgid(const char *);
vocid_t new_word(const char *, int);
vocid_t is_word(const char *);
void save_vocab_index(void);
void hash_stats(void);
void set_version_string(void);
container_t add_container(basic_obj self, basic_obj container);
int is_inside(basic_obj item, basic_obj container);
basic_obj add_basic_obj(struct BASIC_OBJ *ptr, char type, flag_t flags);
void save_basic_objs(void);
basic_obj is_bob(char *s, char type = -1);
basic_obj is_container(char *s);
signed int handle_std_flag(const char *s, flag_t &flags, flag_t filter);

static inline char *
skipspc(char *s)
{
    while (*s && *s == 32)
        s++;
    return s;
}

#endif /* SMUGLCOM_PROTOS */
