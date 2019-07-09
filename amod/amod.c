/*

    ###### ###  ### ###### ####
    ##  ## ######## ##  ## ## ##
    ###### ## ## ## ##  ## ##  ## . C	(C) KingFisher Software 1991
    ##  ## ##    ## ##  ## ## ##		    ~~~~~~~~~~~~~~~~~~~
    ##  ## ##    ## ###### ####

                      */

#include "amod/amodwin.h"        /* Custom Window details  */
#include <devices/serial.h>      /* All about Serial.Device*/
#include <exec/types.h>          /* Exec bits and pieces   */
#include <fcntl.h>               /* File control stuff	  */
#include <inovatools1l.h>        /* Flashy Intui PRAGMAS   */
#include <intuition/intuition.h> /* Intuition includes     */
#include <itools1l.h>            /* Inovatools Includes    */
#include <libraries/dos.h>       /* Various DOS thingies   */
#include <stdio.h>               /* Standard I/O controls  */

#define NOCARRIER (serio->io_Status & (1 << 5)) /* Carrier detect in io_Status */

struct Task *  MyTask, *FindTask();
struct Window *OpenWindow(), *win;

struct MsgPort * port, *CreatePort();
struct IOExtSer *serio, *CreateExtIO();
void             DeletePort(), DeleteExtIO();
char             ser_dev[42], ser_unit, rts, serop, script[80];
char             input[80], match[80], wintitle[80], block[1024];
unsigned long    ser_baud, ITBase, IntuitionBase;
struct MsgPort * ReadRep, *WriteRep;
struct IOStdReq  ReadIo, WriteIo;
short int        die;

FILE *fp;

void
CXBRK()
{
    if (die != 0)
        return 0;
    die = 1;
    if (serop == 1)
        CloseDevice((struct IORequest *)serio);
    if (serio != NULL)
        DeleteExtIO((struct IORequest *)serio);
    if (port != 0L)
        DeletePort(port);
    scrend();
}

/* Calling procedure: AMOD [-baud] [serial_device_name [serial_unit [N [script]]]]

 This means:

    AMOD
or	AMOD -4800
or	AMOD serial.device
or	AMOD -4800 serial.device 0
or	AMOD serial.device 0 N
or	AMOD serial.device 0 Y loadpara

    any valid 'serial' type device can be used instead of SERIAL.DEVICE
    and the UNIT number is only relevant if you have more than one!
    The 'N' means you don't want RTS/CTS control; the default is enabled.
    Anything OTHER than N enables RTS/CTS. script is the name of a script
    file to execute to load the BBS in. The default is 'loadbbs'.
*/

main(int argc, char *argv[])
{
    register char *        p, ok, arg;
    register unsigned long bd, was;

    MyTask = FindTask(0);
    die = 0;
    serop = 0;
    ser_baud = 2400;

    if ((fp = fopen("RAM:ResultCodes.TXT", "rb")) == NULL)
        exit(0 * printf("Cannot open file RAM:ResultCodes.TXT\n"));
    fclose(fp);

    arg = 0;
    if (argc > 1 && *argv[1] == '-') {
        ser_baud = atoi(argv[1] + 1);
        arg = 1;
        argc--;
    }
    if (argc > 1)
        strcpy(ser_dev, argv[1 + arg]);
    else
        strcpy(ser_dev, SERIALNAME);
    if (argc > 2)
        ser_unit = atoi(argv[2 + arg]);
    else
        ser_unit = 0;
    if (argc > 3 && toupper(argv[3 + arg]) == 'N')
        rts = 0;
    else
        rts = SERF_7WIRE;
    if (argc > 4)
        strcpy(script, argv[4 + arg]);
    else
        strcpy(script, "loadbbs");

    if ((fp = fopen(script, "rb")) == NULL)
        exit(0 * printf("Cannot open script %s.\n", script));
    fclose(fp);

    if (!(port = CreatePort(0L, 0L))) {
        printf("Unable to create serial IO port.\n");
        exit(0);
    }
    serio = CreateExtIO(port, (long)sizeof(struct IOExtSer));
    opendev();

    OpenMyWindow();
    sprintf(wintitle, "* AMOD: [%s/%d] %s...", ser_dev, ser_unit, "Working, so hold your horses!");
    SetWindowTitles(win, wintitle, NULL);
    Title();

    wsend(block);
    do {
        char c;

        /* Clear things up */
        hang();

        /* Initialize modem */
        if ((fp = fopen("RAM:InitModem.TXT", "rb")) != NULL) {
        loop:
            fgets(input, 80, fp);
            if (!feof(fp)) {
                send(input);
                send("\r");
                Delay(40);
                goto loop;
            }
            Delay(32);
            fclose(fp);
        }
        sercmd(CMD_CLEAR);

        /* Now input a FULL string */
    retloop:
        sprintf(wintitle, "* AMOD: [%s/%d] %s...", ser_dev, ser_unit, "Waiting for caller");
        SetWindowTitles(win, wintitle, NULL);
        p = input;
        *p = 0;

        while ((*p = (char)sgetc()) != 13 && *p != 10)
            p++;
        *p = 0;

        if (p == input)
            goto retloop; /* Ignore blank strings */

        fp = fopen("RAM:ResultCodes.TXT", "rb");
        while (!feof(fp)) {
            ok = 0;
            fscanf(fp, "%s", match);
            bd = atoi(match);
            fgets(match, 80, fp);
            if (feof(fp))
                continue;
            while (isspace(match[0]))
                strcpy(match, match + 1);
            match[strlen(match) - 1] = 0;
            if (match[0] == '-') {
                ok = 0;
                strcpy(match, match + 1);
            } else
                ok = 1;
            if (stricmp(match, input) == NULL)
                break;
        };

        fclose(fp);
        sercmd(SDCMD_QUERY);
        if (bd < 300 && ok != 0) {
            CloseDevice((struct IORequest *)serio);
            opendev();
            continue;
        }
        if (ok == 0)
            goto retloop;

        Delay(200);
        sprintf(block, "%ld baud connection!\n", bd);
        send(block);
        sprintf(wintitle, "* AMOD: [%s/%d] %s...", ser_dev, ser_unit, block);
        Delay(40);
        SetWindowTitles(win, wintitle, NULL);
        send("\n\rAMOD - Copyright (C) KingFisher Software, 1990.\r\n");
    menu:
        sercmd(SDCMD_QUERY);
        if (NOCARRIER)
            continue;
        modemcfg();
        send("\r\nCodeOMatic System Menu:\r\n\r\n");
        send("Select:\r\n\r\n  A  -  Enter the Multi-user Dungeon...\r\n");
        send("  B  -  Enter the CodeOMatic BBS...\r\n");
        send("  G  -  Goodbye and hang up\r\n\r\n");
        sercmd(CMD_CLEAR);
        send("Which (A,B or G): ");
        sercmd(SDCMD_QUERY);
        Delay(8);
        input[0] = (char)sgetc();
        sercmd(SDCMD_QUERY);
        sprintf(match, "%c\r\n\r\n", c = toupper(input[0]));
        send(match);
        if (c != 'A' && c != 'B' && c != 'G') {
            send("\r\nInvalid command!\r\n");
            sercmd(SDCMD_QUERY);
            if (NOCARRIER) {
                wsend("Lost carrier!\n");
                sercmd(CMD_CLEAR);
                CloseDevice((struct IORequest *)serio);
                opendev();
                sercmd(CMD_CLEAR);
                continue;
            } else
                goto menu;
        }
        if (c == 'A') {
            send("Please note... AMUL is still under development. Report ANY bugs!\r\n\r\n");
            sprintf(input, "amul -S %ld %s", (ser_baud > 2400) ? ser_baud : bd, ser_dev);
            sprintf(wintitle, "* AMOD: [%s/%d] %s...", ser_dev, ser_unit, "Playing AMUL...");
        }
    goodbye:
        if (c == 'G') {
            send("Thankyou very much for calling the CodeOMatic "
                 "BBS...\r\n\r\nCiao!\r\n\r\n<Click>\r\n");
            Delay(20);
            continue;
        }
        if (c == 'B') {
            send("One moment, please... Loading the CodeOMatic BBS...\r\n\r\n");
            if (strcmp(ser_dev, "serial.device") != NULL)
                sprintf(input, "paragon 2 h b%ld spawned", (ser_baud > 2400) ? ser_baud : bd);
            else
                sprintf(input, "paragon 1 h b%ld l9600 spawned", (ser_baud > 2400) ? ser_baud : bd);
            sprintf(wintitle, "* AMod: [%s/%d] %s...", ser_dev, ser_unit, "Entered the BBS...");
        }
        sercmd(CMD_CLEAR);
        sercmd(SDCMD_QUERY);
        if (NOCARRIER)
            continue;
        SetWindowTitles(win, wintitle, NULL);
        sprintf(block, "Executing '%s'\n", input);
        wsend(block);
        Execute(input, 0, 0);
        Delay(32);
        if (c == 'B') {
            c = 'G';
            goto goodbye;
        }
        sercmd(CMD_CLEAR);
        modemcfg();
        sercmd(SDCMD_QUERY);
        if (NOCARRIER)
            continue;
        sercmd(CMD_CLEAR);
        goto menu;
    } while (p == p);
}

sercmd(int cmd)
{
    serio->IOSer.io_Command = cmd;
    DoIO((struct IORequest *)serio);
    Delay(10);
}

sgetc() /* Ask for a character to be input to *s */
{
    char c;

    c = 0;
    serio->IOSer.io_Data = (APTR)&c; /* Buffer */
    serio->IOSer.io_Length = 1;      /* One char at a time */
    serio->IOSer.io_Message.mn_ReplyPort = port;
loop:
    serio->IOSer.io_Command = CMD_READ;
    SendIO((struct IORequest *)serio);
    Wait(-1);
    if (GetMsg((struct MsgPort *)port) != NULL)
        return (int)c & 255;
    AbortIO((struct IORequest *)serio);
    goto loop;
    return (int)c & 255;
}

ssend(char *s)
{
    serio->IOSer.io_Data = (APTR)s;
    serio->IOSer.io_Length = strlen(s);
    serio->IOSer.io_Command = CMD_WRITE;
    DoIO((struct IORequest *)serio);
}

send(char *s)
{
    wsend(s);
    ssend(s);
}

modeminit()
{
    serio->io_ExtFlags = 0;
    serio->io_Baud = ser_baud;
    serio->io_SerFlags = SERF_SHARED + rts;
    serio->io_Status = 136;
    serio->IOSer.io_Message.mn_ReplyPort = port;
}

modemcfg()
{
    modeminit();
    sercmd(SDCMD_SETPARAMS);
    Delay(20);
}

opendev()
{
    serop = 0;
    modeminit();
    if (OpenDevice(ser_dev, ser_unit, (struct IORequest *)serio, 0) != 0)
        CXBRK(printf("Cannot access device \"%s\"!\n", ser_dev));
    serio->IOSer.io_Device = serio->IOSer.io_Device;
    serio->IOSer.io_Unit = serio->IOSer.io_Unit;

    sercmd(CMD_RESET);
    sercmd(CMD_CLEAR);
    sercmd(CMD_FLUSH);
    sercmd(SDCMD_QUERY);

    modemcfg();
    if (serio->IOSer.io_Error != NULL)
        CXBRK(printf("Unable to configure %s\n", ser_dev));
    serop = 1;
}

hang()
{
    AbortIO((struct IORequest *)serio);
loop:
    CloseDevice((struct IORequest *)serio);
    Delay(40);
    opendev();
    send("\r\n");
    Delay(30);
    CloseDevice((struct IORequest *)serio);
    Delay(40);
    opendev();
    Delay(20);
    send("+++");
    Delay(60);
    send("ATZ\r\n");
    Delay(100);
    send("\r\n");
    Delay(20);
    sercmd(CMD_FLUSH);
    Delay(10);
    sercmd(CMD_CLEAR);
    Delay(10);
    sercmd(SDCMD_QUERY); /* Check that we lost 'im! */
    if (!NOCARRIER) {
        Delay(32);
        goto loop;
    }
}

OpenMyWindow()
{
    if ((IntuitionBase = OpenLibrary("intuition.library", 0L)) == NULL) {
        puts("Can't open Intuition library!");
        scrend();
    }
    if ((ITBase = OpenLibrary("inovatools1.library", 0L)) == NULL) {
        puts("Unable to use InovaTools1.Library (C) InovaTronics ... No flash stuff then.");
    }
    /* Create ports for IO */
    ReadRep = CreatePort("ReadPort", 0);
    WriteRep = CreatePort("WritePort", 0);
    if (ReadRep == 0L || WriteRep == 0L) {
        puts("unable to allocate reply ports.\n");
        scrend();
    }

    if (ITBase != NULL)
        win = FlashyOpenWindow(&NewWindowStructure1);
    else
        win = OpenWindow(&NewWindowStructure1);

    if (win == NULL) {
        puts("unable to open window!\n");
        scrend();
    }

    ReadIo.io_Data = (APTR)win; /* Window handle */

    if (OpenDevice("console.device", 0, (struct IORequest *)&ReadIo, 0) != NULL) {
        sprintf(block, "Unable to open %s!", "console.device");
        puts(block);
        scrend();
    }
    WriteIo.io_Device = ReadIo.io_Device;
    WriteIo.io_Unit = ReadIo.io_Unit;
    WriteIo.io_Command = 3; /* Write */
    ReadIo.io_Command = 2;  /* Read */
    ReadIo.io_Message.mn_ReplyPort = ReadRep;
    WriteIo.io_Message.mn_ReplyPort = WriteRep;
}

scrend()
{
    if (ReadIo.io_Device != NULL)
        CloseDevice((struct IORequest *)&ReadIo);
    if (ReadRep != NULL)
        DeletePort(ReadRep);
    if (WriteRep != NULL)
        DeletePort(WriteRep);
    if (win != NULL) {
        if (ITBase != NULL)
            FlashyCloseWindow(win);
        else
            CloseWindow(win);
    }
    if (IntuitionBase != 0L)
        CloseLibrary((struct Library *)IntuitionBase);
    if (ITBase != 0L)
        CloseLibrary((struct Library *)ITBase);
    exit(0);
}

wsend(register char *s) /* Send to Window */
{
    WriteIo.io_Data = (APTR)s;
    WriteIo.io_Length = strlen(s);
    DoIO((struct IORequest *)&WriteIo);
}

Title()
{
    wsend("   =========================================================================\n");
    wsend("   =                                                                       =\n");
    wsend("   =   ####   ###  ###  ######  #####                                      =\n");
    wsend("   =   ####   ###  ###  ######  ######        +++++++++++++++++++++++++    =\n");
    wsend("   =  ##  ##  ########  ##  ##  ##   ##       + Answer that MODem 1.0 +    =\n");
    wsend("   =  ##  ##  ########  ##  ##  ##   ##       + ~~~~~~~~~~~~~~~~~~~~~ +    =\n");
    wsend("   =  ######  ## ## ##  ##  ##  ##   ##  1.0  + By Oliver Smith, 1991 +    =\n");
    wsend("   =  ######  ## ## ##  ##  ##  ##   ##       + ~~~~~~~~~~~~~~~~~~~~~ +    =\n");
    wsend("   =  ##  ##  ##    ##  ######  ######        +++++++++++++++++++++++++    =\n");
    wsend("   =  ##  ##  ##    ##  ######  #####                                      =\n");
    wsend("   =                                                                       =\n");
    wsend("   =========================================================================\n");
    Delay(50);
}
