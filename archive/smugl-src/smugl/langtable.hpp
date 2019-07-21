// This may look like C, but it's really -*- C++ -*-
// $Id: langtable.hpp,v 1.1 1999/06/11 14:26:45 oliver Exp $
//
// language table entry processor. This houses all the condition and
// action code, and the tables used to invoke them

#ifndef LANGTABLE_H

# define LANGTABLE_H 1

bool do_condition(VBTAB *vt, bool lastCond);
slotResult do_action(VBTAB *vt, bool lastCond);

#endif /* LANGTABLE_H */
