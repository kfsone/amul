//
// AMUD/frame/Internal.C	Handles AMUD's built-in "/" commands
//

/****** AMUD3.C/internal ******************************************
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
*			r <redo char>	Changes users redo-char
*			mN <macro>	Changes macro #N (n=1-4)
*			ansi [on|off]	Toggles ANSI on/off for the user
*			llen <line len>	Changes users line-length
*			plen <page len>	Changes users page-length
*
******************************************************************************
*
*/

internal( register char *s ) {
	if(*s=='?') { tx(*(errtxt-4)); return; }	// List commands
		
	if(*s=='x') { getllen(); return; }
	if(*s=='y') { getslen(); return; }

	if(*s=='l') {
		me->flags = me->flags ^ ufCRLF;
		txs("LineFeed follows carriage return %sABLED.\n",(me->flags & ufCRLF)?"EN":"DIS");
		return;
	}

	if(*s=='a') {
		switch(*(s+1)) {
		    case 'n':
			me->flags = me->flags ^ ufANSI;
			ans("1m"); txs("ANSI control codes now %sABLED.\n",(me->flags & ufANSI)?"EN":"DIS"); ans("0m");
			return;
		}
	}

	if(*s=='p') {
		if(him.passwd[0]) {
			tx("^GPersona does not have a password.\n"); return;
		}
		tx("Old password: "); Inp(input,-20);
		if(!input[0]) { tx("Cancelled.\n"); return; }
		if(strcmp(input,me->passwd)) { tx("Invalid password.\n"); return; }
		tx("New password: "); Inp(input,-20);
		if(!input[0]) { tx("Cancelled.\n"); return; }
		if(!strcmp(input,me->passwd)) { tx("Passwords are the same.\n"); return; }
		tx("Verify      : "); Inp(block,-20);
		if(strcmp(input,block)) { tx("Passwords don't match.\n"); return; }
		strcpy(me->passwd, input); tx("Password changed.\n"); save_me();
		return;
	}

	tx("Invalid internal command. Type /? for list of commands.\n");
}

flagbits() {		// Get the users flag bits
	sprintf(block,*(errtxt-3),me->llen,me->slen,(me->flags & ufANSI)?"On":"Off",(me->flags & ufCRLF)?"On":"Off");
	tx(block); Inp(str,2);
	if(toupper(str[0])=='Y') { getllen(); getslen(); getflags(); }
}

getllen() {
	tx("\nHow wide (in characters) is your display?\n");
	sprintf(input,"%ld %s",me->llen,"characters"); txs("Screen width [RETURN for %s]: ",input);
	Inp(str,4); if(str[0]) me->llen=atoi(str); if(me->llen<8) me->llen=40;
}

getslen() {
	tx("\nHow many LINES of text does your display have?\n");
	sprintf(input,"%ld %s",me->slen,"lines"); txs("Screen length [RETURN for %s]: ",input);
	Inp(str,3); if(str[0]) me->slen=atoi(str);
}

getflags() {
	tx("^MFollow Carriage Return with a Line Feed? [Y/n]: "); Inp(str,2);
	if(toupper(*str)!='N') me->flags=me->flags|ufCRLF; else me->flags=me->flags^ufCRLF;
	tx("^MUse ANSI control codes? [y/N]: "); Inp(str,2);
	if(toupper(str[0])=='Y') me->flags=me->flags|ufANSI; else me->flags=me->flags^ufANSI;
}
