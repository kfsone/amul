// Definition of mobile classes and functions

#include "smugl/smugl.hpp"
#include "include/structs.hpp"

#include "smugl/data.hpp"
#include "smugl/mobiles.hpp"
#include "smugl/rooms.hpp"

//class MobileIdx MobileIdx;

class Room *
Mobile::dmoveRm(void)			// Return DMOVE room of a mobile
{
	return data->roombase + dmove;
}

/* XXX: Debugging version of describe */
bool
Mobile::describe(void)
{
	txprintf("mobile=%s(%ld)\n", word(id), id);
	txprintf("speed=%d travel=%d fight=%d act=%d wait=%d\n", speed, travel, fight, act, wait);
	txprintf("fear=%d attack=%d dead=%d flags=%d", fear, attack, dead, flags);
	if (dmove != -1)
		txprintf(" dmove=%s", word(dmoveRm()->id));
	txprintf("\nhitp=%d str=%d stam=%d dext=%d wisdom=%d exp=%d mag=%d\n",
			 hitpower, strength, stamina, dext, wisdom, experience, magicpts);
	if (arr != -1)
		txprintf("arrive=\"%s\"\n", message(arr));
	if (dep != -1)
		txprintf("depart=\"%s\"\n", message(dep));
	if (flee != -1)
		txprintf("flee=\"%s\"\n", message(flee));
	if (hit != -1)
		txprintf("hit=\"%s\"\n", message(hit));
	if (miss != -1)
		txprintf("miss=\"%s\"\n", message(miss));
	if (death != -1)
		txprintf("death=\"%s\"\n", message(death));
	return true;
}

//////////////////////////////////////// Mobile Index functions

Mobile *
MobileIdx::locate(char *s)		// Locate a mobile by name
{
	long    w;

	w = is_word(s);
	if (w == -1)
		return NULL;
	return locate(w);
}

Mobile *
MobileIdx::locate(long id)		// Locate a mobile by vocab id
{
	class Mobile *ptr;
	int     i;

	for (i = 0, ptr = data->mobbase; i < data->mobiles; i++, ptr++)
	{
		if (ptr->id == id)
			return ptr;
	}
	return NULL;
}