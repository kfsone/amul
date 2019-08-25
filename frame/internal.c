/****** AMUL3.C/internal ******************************************
 *
 *   NAME
 *	internal -- process internal control command.
 *
 *   SYNOPSIS
 *	internal( Command )
 *
 *	void internal( BYTE );
 *
 *   FUNCTION
 *	Processes an internal-command string pointed to by Command. Options
 *	available are listed below.
 *
 *   INPUTS
 *	Command - points to an ASCIZ string containing a command sequence.
 *		available commands are:
 *			?		Displays available commands.
 *			p [password]	Change users password
 *			lf [on|off]	Toggles linefeeds on/off
 *			ar [on|off]	Toggles auto-redo on/off
 *			r <redo char>	Changes users redo-char
 *			mN <macro>	Changes macro #N (n=1-4)
 *			ansi [on|off]	Toggles ANSI on/off for the user
 *			llen <line len>	Changes users line-length
 *			plen <page len>	Changes users page-length
 *
 ******************************************************************************
 *
 */

internal(char *s)
{
    char *p;

    if (*s == '?') {
        tx("AMULEd v0.5 - All commands prefixed with a \"/\"\n\n");
        tx(" /?\tDisplays this list\n");
        tx(" /p\tChange password\n");
        tx(" /lf\tToggle linefeeds on/off\n");
        tx(" /ar\tToggle auto-redo on/off\n");
        tx(" /r\tSet redo-character\n");
        tx(" /mN\tSet macro #N (n=1-4)\n");
        tx(" /an\tToggle ANSI on/off\n");
        tx(" /x\tSet line-length\n");
        tx(" /y\tSet page-length\n\n");
        return;
    }

    p = s;
    while (*p != 0) {
        *p = tolower(*p);
        p++;
    }

    if (*s == 'r') {
        getrchar();
        return;
    }
    if (*s == 'x') {
        getllen();
        return;
    }
    if (*s == 'y') {
        getslen();
        return;
    }

    if (*s == 'l') {
        me->flags = me->flags ^ ufCRLF;
        txs("LineFeed follows carriage return %sABLED.\n", (me->flags & ufCRLF) ? "EN" : "DIS");
        return;
    }

    if (*s == 'a') {
        switch (*(s + 1)) {
        case 'n':
            me->flags = me->flags ^ ufANSI;
            ans("1m");
            txs("ANSI control codes now %sABLED.\n", (me->flags & ufANSI) ? "EN" : "DIS");
            ans("0;37m");
            save_me();
            return;
        }
    }

    if (*s == 'p') {
        tx("Enter old password  : ");
        Inp(input, 250);
        if (input[0] == 0) {
            tx("Cancelled.\n");
            return;
        }
        if (stricmp(input, me->passwd) != NULL) {
            tx("Invalid password.\n");
            return;
        }
        tx("Enter new password  : ");
        Inp(input, 250);
        if (input[0] == 0) {
            tx("Cancelled.\n");
            return;
        }
        if (stricmp(input, me->passwd) == NULL) {
            tx("Passwords are the same.\n");
            return;
        }
        tx("Confirm new password: ");
        Inp(block, 250);
        if (stricmp(input, block) != NULL) {
            tx("Passwords did not match.\n");
            return;
        }
        strcpy(me->passwd, input);
        tx("Password changed.\n");
        save_me();
        return;
    }

    tx("Invalid internal command. Type /? for list of commands.\n");
}
