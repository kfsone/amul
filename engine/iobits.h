#pragma once

void ans(const char *s);
void iocheck();
void lockusr();
int  ioproc(const char *s);
int  esc(const char *p, char *s);
void interact(int msg, int n, int d);
void sendex(int n, int d, int p1, int p2, int p3, int p4);
void putc(char c);
void tx(const char *s);
void utx(int n, const char *s);
void utxn(int player, const char *format, int n);
void txc(char c);
void txn(const char *format, int n);
void txs(const char *format, const char *s);
void Inp(char *s, int l);
