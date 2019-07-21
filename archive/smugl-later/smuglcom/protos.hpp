#pragma once
// This may look like C, but it's really -*- C++ -*-
// protos.hpp -- function declarations / prototypes

extern int     is_room_flag(const char *s);
extern int     is_room_param(char *&s);
extern int     isoflag1(const char *s);
extern int     isoparm(void);
extern int     isoflag2(const char *s);
extern void    checksys(void);
extern int     iscond(const char *s);
extern int     isact(const char *s);
extern void    section(int i);
extern void    error(const char *s, ...);
extern void    warne(const char *s, ...);
extern void    errabort(void);

/*
int higerhalph(char *s1, char *s2);
void sort_objs(void);
*/
extern void    close_ofps(void);
extern int     nextc(int);
extern void    quit(const char *s, ...);
extern void    fopenw(const char *s);
extern void    fopena(const char *s);
extern void    fopenr(const char *s);
extern void    Err(const char *s, const char *t);
extern FILE   *rfopen(const char *s);
extern void    ttroomupdate(void);
extern void    skipblock(void);
extern char   *skipdata(char *p);
extern void    tidy(char *s);
extern void    clean_trim(char *s);
extern int     is_verb(const char *s);
extern char   *blkget(void);
extern char   *cleanget(off_t off = 0);
extern char   *text_proc(char *p, FILE * destfp);
extern void    lang_proc(void);
extern int     actualval(const char *, arg_t);
extern char   *precon(char *);
extern char   *preact(char *);
extern char   *chkaparms(char *, int);
extern char   *chkcparms(char *, int);
extern void    umsg_proc(void);
extern void    sys_proc(void);
extern void    room_proc(void);
extern void    finish_rooms(void);
extern void    smsg_proc(void);
extern void    mob_proc1(void);
extern void    obds_proc(void);
extern void    objs_proc(void);
extern void    trav_proc(void);
extern void    syn_proc(void);
extern void    opentxt(const char *s);
extern int     isnoun(const char *s);
extern msgno_t ttumsgchk(char *s);
extern char   *getword(char *s);
extern char   *skiplead(const char *skip, char *in);
extern char   *skipline(char *s);
extern void    repspc(char *s);
extern void    clean_up(char *p);
extern void    tx(const char *s);
extern void    get_line(FILE * fp, char *into, int limit);
extern void   *grow(void *ptr, size_t size, const char *msg);
extern void    save_messages(void);
extern msgno_t add_msg(const char *);
extern msgno_t ismsgid(const char *);
extern vocid_t new_word(const char *, bool);
extern vocid_t is_word(const char *);
extern void    save_vocab_index(void);
extern void    hash_stats(void);
extern void    set_version_string(void);
extern container_t add_container(basic_obj self, basic_obj container);
extern bool     is_inside(basic_obj item, basic_obj container);
extern basic_obj add_basic_obj(BASIC_OBJ * ptr, char type, flag_t flags);
extern void    save_basic_objs(void);
extern basic_obj is_bob(char *s, char type = -1);
extern basic_obj is_container(char *s);
extern signed int handle_std_flag(const char *s, flag_t & flags, flag_t filter);

static inline char *
skipspc(char *s)
{
	while (*s && *s == 32)
		s++;
	return s;
}
