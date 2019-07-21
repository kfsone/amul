// SMUGL Parser
static const char cvsid[] = "$Id: parser.cc,v 1.9 1999/06/11 14:26:45 oliver Exp $";

#include "smugl.hpp"
#include "parser.hpp"
#include "ipc.hpp"
#include "ranks.hpp"
#include "aliases.hpp"
#include "rooms.hpp"
#include "lang.hpp"
#include "langtable.hpp"
#include "consts.hpp"

vocid_t verb = -1;              // Current verb
char input[MAX_PHRASE_SIZE + 1]; // Current input
char phrase[MAX_PHRASE_SIZE + 1]; // Current phrase
TOKEN token[MAX_TOK];           // Details of the token
int tokens;                     // Number of tokens in current phrase

Indecision maybe;

int parse_failed = FALSE;


////////////////////////////////////////////////////
// message_failed(message_id)
// Report a message and mark that the parse attempt failed
// Macro'ised to insert some consistency
#define FAILED(msg) \
    { \
    tx(message(msg), '\n'); \
    parse_failed = TRUE; \
    }

////////////////////////////////////////////////////
// sanitise_input()
// Removes excess whitespace from an input string
void
sanitise_input(void)
    {
    char *start = input;
    while (isspace(*start))     // Remove leading spaces
        start++;
    if (!*start)                // Ignore a white-space only input
        {
        input[0] = 0;
        return;
        }
    char *rptr = start;         // Read
    char *wptr = input;         // Write
    char in_quote = 0;
    // Now iterate through the string and do any substitutions or
    // tokenisations neccesary
    while (*rptr)
        {
        if (in_quote)
            {                   // Do nothing clever while inside quotes
            if (*rptr == in_quote)
                in_quote = 0;
            }
        else if (*rptr == '"' || *rptr == '\'')
            in_quote = *rptr;
        else if (isspace(*rptr))
            {
            while (isspace(*(rptr + 1)) && *rptr)
                rptr++;
            }
        *(wptr++) = tolower(*(rptr++));
        }
    *(wptr--) = 0;
    // Remove trailing whitespace
    while (wptr >= input && isspace(*wptr))
        *(--wptr) = 0;
    }

////////////////////////////////////////////////////
// parse()
// First layer of parser; breaks the input into
// phrases (or sentences) and passes them on to the
// next level
void
parse(char *string)
    {
    int phrase_no = 0;

    // Start-of-parseing setup
    forced = FALSE;
    exiting = ecFalse;
    parse_failed = FALSE;

    char *dest;
    char *token;
    while (*string && !forced && !exiting && !parse_failed)
        {
        dest = phrase;          // Start a new phrase
        *dest = 0;

        while (*string)
            {
            // Skip next whitespace set
            while (isspace(*string))
                string++;
            if (!*string)
                break;

            // Add whitespace between tokens
            if (dest > phrase)
                *(dest++) = ' ';

            // In quotes, do an almost raw copy,
            // stopping only for EOL or the same quote
            if (*string == '"' || *string == '\'')
                {
                char quote = *(string++);
                *(dest++) = quote;
                while (*string && *string != quote)
                    *(dest++) = *(string++);
                if (*string)
                    *(dest++) = *(string++);
                continue;
                }

            // Capture the beginning of this token
            token = dest;

            while (*string && strchr(",.!?; ", *string) == 0)
                *(dest++) = *(string++);
            *dest = 0;

            if (*string && strchr(",.!?;", *string))
                {
                string++;
                break;
                }

            // Tokens that we treat as 'eol'
            if (strcmp(token, "then") == 0)
                {
                while (token > phrase && isspace(*(token - 1)))
                    *(--token) = 0;
                break;
                }
            }

        while (dest > phrase && isspace(*(dest-1)))
            *(--dest) = 0;
        *dest = 0;              // Ensure null term
        if (!phrase[0])
            continue;

        // After our first phrase, remind the user
        // what we're parsing
        if (phrase_no)
            {
            // XXX: ans("3m");
            txprintf("%s%s\n", message(myRank->prompt),
                     phrase);
            // XXX: ans("0m");
            }
        phrase_no++;
        if (tokenise_phrase(phrase) == -1)
            return;
        if (parse_phrase() == slotFailed)
            return;
        }
    }

// Determines if the given token represents one of the special words
////////////////////////////////////////////////////
// special_word(char *token)
// Determines if the given token represents one of the special words
// e.g., him, her, she, it, etc...
static inline basic_obj
special_word(char *token)
    {
    if (strcmp(token, "me") == 0
        || strcmp(token, "myself") == 0
        || strcmp(token, "self") == 0)
        return me->id;
    if (last_him
        && (strcmp(token, "he") == 0
            || strcmp(token, "him") == 0
            || strcmp(token, "his") == 0))
        return last_him->id;
    if (last_her
        && (strcmp(token, "she") == 0
            || strcmp(token, "her") == 0
            || strcmp(token, "hers") == 0))
        return last_her->id;
    return vocUNKNOWN;
    }

////////////////////////////////////////////////////
// tokenise_phrase(char *string)
// Break a phrase up into tokens
int
tokenise_phrase(char *string)
    {
    // Nuke the current phrase
    for (tokens = 0; tokens < MAX_TOK; tokens++)
        token[tokens].type = tokUNK;

    tokens = -1;
    // Iterate throughout the length of the input string;
    // we can expect two types of token:
    // tokSTRING: A text string which we keep 'as is'
    // tokWORD:   A valid vocabulary entry
    // also
    // tokUNK:    We didn't recognise this [shouldn't be used]
    while (string && *string)
        {
        if (*string == '"' || *string == '\'')
            {
            tokens++;                // A valid token
            char quot = *(string++);
            token[tokens].type = tokSTRING;
            token[tokens].ptr = string;
            while (*string && *string != quot)
                string++;
            if (*string)
                *(string++) = 0;
            if (isspace(*string))
                string++;
            }
        else
            {
            vocid_t this_id;
            char *next_tok = string;
            char original_char = 0;
            // Check the immediate string for a word; since words can
            // contain spaces, we expand outward, checking 
            do
                {
                // Find the next space
                next_tok = strchr(next_tok, ' ');
                if (next_tok)
                    {
                    original_char = *next_tok;
                    *next_tok = 0;
                    }
                // Try it as a regular word
                this_id = is_word(string);
                // Try it as a special case
                if (this_id == -1)
                    this_id = special_word(string);
                if (this_id == -1 && next_tok)
                    *next_tok = original_char;
                if (next_tok && original_char)
                    next_tok++;
                }
            while (next_tok && this_id == -1);

            if (this_id == -1)
                {
                FAILED(INVALIDVERB);
                return -1;
                }

            long alias;
            while ((alias = Alias::locate(this_id)) != -1)
                {
                if ((this_id = Alias::meaning(alias)) == -1)
                    break;
                }
            // At this point, if the token is a noise word, then
            // token-id will be -1, and we know to ignore this token
            if (this_id != -1)
                {
                tokens++;
                token[tokens].type = tokWORD;
                token[tokens].id   = this_id;
                }
            string = next_tok;
            }
        }
    return 0;
    }

////////////////////////////////////////////////////
// matching_phrase(SLOTTAB *slot)
// Returns TRUE or FALSE whether the current tokens
// may possibly match given language slot
static int
matching_phrase(SLOTTAB *slot)
    {
    // This is fiddly, on the grounds that we may have to juggle
    // tokens around. Why? Because of adjectives. The trouble is,
    // we won't know they're adjectives until we find something that
    // uses them in adjective context. In the mean time, they might
    // well not be. If you think this is overkill, remember:
    // pot the plant in the plant pot

    // 1. If we only have one token, we match WNONE and WANY
    if (debug)
	txprintf("Tokens:%d, wtype0:%d, wtype1:%d\n",
		    tokens, slot->wtype[0], slot->wtype[1]);
    if (tokens == 0 && (slot->wtype[0] == slot->wtype[1]))
	{
	if (slot->wtype[0] == WNONE || slot->wtype[0] == WANY)
	    return TRUE;
	// We obviously don't suit this one
	return FALSE;
	}

    // 2. The next simplest case is when we have two tokens,
    // the verb and another word. It must be a noun-class object
    if (tokens == 1 && slot->wtype[1] == WNONE)
	{
	return FALSE;
	}
    return FALSE;
    }

////////////////////////////////////////////////////
// Iterate across a given slottab sequence
// As yet we don't think we know what the user has
// said, were simply looking for patterns that may
// match what they said.
slotResult
slot_process(SLOTTAB *slot)
    {
    unsigned int ent;
    bool lastCond = FALSE;
    VBTAB *vt;

    assert(slot->ptr != NULL);

    for (ent = 0, vt = slot->ptr; ent < slot->ents; ent++, vt++)
	{
	if (debug)
	    txprintf("Entry#%d: cond=%s%d, action=%d\n",
		    ent, (vt->not_condition) ? "!" : "", vt->condition,
		    vt->action);

	lastCond = do_condition(vt, lastCond);

	if (debug)
	    {
    	    if (lastCond)
    		txprintf("returned TRUE\n", ent);
    	    else
   		txprintf("returned FALSE\n", ent);
	    }

	if (lastCond == TRUE)
	    {
	    slotResult sr;
	    sr = do_action(vt, lastCond);
	    if (debug)
		txprintf("result was %d\n", sr);
	    if (sr != slotProcessed)
		return sr;
	    }
	}
    return slotProcessed;
    }

////////////////////////////////////////////////////
// parse_phrase()
// Try and match the current tokens against the
// language table and make sense of everything
slotResult
parse_phrase()
    {
    int i;
    if (debug)
        {
        int tokens_shown;
        tx("De-tokenised phrase is:\n");
        for (i = 0, tokens_shown = 0; i < MAX_TOK && token[i].type != tokUNK; i++)
            {
            char tbuf[16];
            sprintf(tbuf, "[%d: @t%d] ", i, i);
            tx(tbuf);
            tokens_shown++;
            }
        if (tokens_shown)
            tx("\n");
        }

    // Firstly; how does the phrase start?
    switch (token[0].type)
        {
      case tokUNK:              // Unknown 'verb'
            // We shouldn't reach here, but it never hurts to check
            FAILED(INVALIDVERB);
            return slotFailed;

      case tokSTRING:           // User is 'say'ing something
            // XXX: Eventually something like:
            // token[1] = token[0]
            // token[0] = tokenSAY;
            // return parse_phrase;

            announce_into(me->Location(), "@me says \"@t0\".");
            return slotProcessed;

      case tokWORD:             // A verb we know and love
            if ((maybe.Vb = VerbIdx::locate(token[0].id)))
                {
                verb = token[0].id;

                if (maybe.Vb->flags & VB_TRAVEL)
                    {
                    if (cur_loc->leave(maybe.Vb))
                        return slotProcessed;
                    FAILED(CANTGO);
                    return slotFailed;
                    }

		assert(maybe.Vb->ents == 0 || maybe.Vb->ptr != NULL);

		for (i = 0; i < maybe.Vb->ents; i++)
		    {
		    slotResult srResult;

		    SLOTTAB *slot = maybe.Vb->ptr + i;
		    if (debug)
			txprintf("Slot %d has %ld entries for (%s:%ld,%s:%ld)\n",
			    i, slot->ents,
			    syntax[slot->wtype[0] + 1], slot->slot[0],
			    syntax[slot->wtype[1] + 1], slot->slot[1]);
		    if (matching_phrase(slot) == FALSE)
			continue;
		    if (debug)
			txprintf("+ Interested in that one.\n");

		    srResult = slot_process(slot);
		    if (srResult != slotIgnore)
			return srResult;
		    }
                }

            if ((maybe.Rm = RoomIdx::locate(token[0].id)))
                {
                if (me->go_to(maybe.Rm->bob,
                              message(SGOVANISH), message(SGOAPPEAR)))
                    return slotProcessed;
                parse_failed = TRUE;
                return slotFailed;
                }

	    // In that case, it's a regular verb; start processing it's
	    // entries in the language table
	    break;

      default:
            FAILED(INVALIDVERB);
            break;
        }
    return slotIgnore;
    }

