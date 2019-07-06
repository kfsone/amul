/*
              ****    AMUL.CONS.H.....Adventure Compiler    ****
              ****                constants                 ****

*/

#ifndef AMAN
extern const char *obputs[NPUTS];
extern const char *prep[NPREP];
#endif

extern const uint8_t ncop[NCONDS];
extern const uint8_t nacp[NACTS];

#ifndef COMPILER
const char managerName[] = "AMUL Manager Port";  // MU driver port
const char plyrfn[] = "Players Data";            // User Details
#endif
