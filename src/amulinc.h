#pragma once
#ifndef AMUL_H_AMULINC_H
#define AMUL_H_AMULINC_H

#include <string>

#include "amul.defs.h"
#include "typedefs.h"

extern thread_local slotid_t t_slotId;

void iocheck();
bool lang_proc(verbid_t verbId, char e);
void look(roomid_t roomNo, RoomDescMode mode);
void action(const char *s, slotid_t towho);
void actionfrom(objid_t obj, const char *s);
void objaction(objid_t obj, const char *s);
void actionin(roomid_t toroom, const char *s);
void action(const char *s, slotid_t towho);
void objannounce(objid_t obj, const char *s);
void announcefrom(objid_t obj, const char *s);
void announcein(roomid_t toroom, const char *s);
void announce(const char *s, int towho);
bool CanSee(slotid_t viewer, slotid_t subject) noexcept;
void lighting(slotid_t player, AnnounceType twho);
std::string MakeInventory(slotid_t slotId, bool stateEmpty);
amulid_t GetConcreteValue(amulid_t id);
string_view GetNamedString(string_view name, string_view defaultValue) noexcept;

#endif  // AMUL_H_AMULINC_H

