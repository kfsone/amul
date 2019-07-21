#pragma once
// This may look like C, but it's really -*- C++ -*-
//
// language table entry processor. This houses all the condition and
// action code, and the tables used to invoke them

extern bool    do_condition(VBTAB * vt, bool lastCond);
extern slotResult do_action(VBTAB * vt, bool lastCond);
