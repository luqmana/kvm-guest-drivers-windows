/*
 * Copyright (C) 2015-2017 Red Hat, Inc.
 *
 * Written By: Gal Hammer <ghammer@redhat.com>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met :
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and / or other materials provided with the distribution.
 * 3. Neither the names of the copyright holders nor the names of their contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "pvpanic.h"
#include "bugcheck.tmh"

static KBUGCHECK_CALLBACK_RECORD CallbackRecord;
static KBUGCHECK_REASON_CALLBACK_RECORD DumpCallbackRecord;

KBUGCHECK_CALLBACK_ROUTINE PVPanicOnBugCheck;
KBUGCHECK_REASON_CALLBACK_ROUTINE PVPanicOnDumpBugCheck;

VOID PVPanicOnBugCheck(IN PVOID Buffer, IN ULONG Length)
{
    //Trigger the PVPANIC_PANICKED event if the crash dump isn't enabled,
    if ((Buffer != NULL) && (Length == sizeof(PVOID)) && !bEmitCrashLoadedEvent)
    {
        PUCHAR PortAddress = (PUCHAR)Buffer;
        WRITE_PORT_UCHAR(PortAddress, (UCHAR)(PVPANIC_PANICKED));
    }
}

VOID PVPanicOnDumpBugCheck(
    KBUGCHECK_CALLBACK_REASON Reason,
    PKBUGCHECK_REASON_CALLBACK_RECORD Record,
    PVOID Data,
    ULONG Length)
{
    UNREFERENCED_PARAMETER(Data);
    UNREFERENCED_PARAMETER(Length);

    //Trigger the PVPANIC_CRASHLOADED event before the crash dump.
    if ((PvPanicPortAddress != NULL) && (Reason == KbCallbackDumpIo) && !bEmitCrashLoadedEvent)
    {
        WRITE_PORT_UCHAR(PvPanicPortAddress, (UCHAR)(PVPANIC_CRASHLOADED));
        bEmitCrashLoadedEvent = TRUE;
    }
    //Deregister BugCheckReasonCallback after PVPANIC_CRASHLOADED is triggered.
    if (bEmitCrashLoadedEvent)
        KeDeregisterBugCheckReasonCallback(Record);
}

BOOLEAN PVPanicRegisterBugCheckCallback(IN PVOID PortAddress)
{
    BOOLEAN bBugCheck;

    KeInitializeCallbackRecord(&CallbackRecord);
    KeInitializeCallbackRecord(&DumpCallbackRecord);

    bBugCheck = KeRegisterBugCheckCallback(&CallbackRecord, PVPanicOnBugCheck,
                  (PVOID)PortAddress, sizeof(PVOID), (PUCHAR)("PVPanic"));
    if (bSupportCrashLoaded)
    {
        BOOLEAN bReasonBugCheck;
        bReasonBugCheck = KeRegisterBugCheckReasonCallback(&DumpCallbackRecord,
                  PVPanicOnDumpBugCheck, KbCallbackDumpIo, (PUCHAR)("PVPanic"));
        return bBugCheck && bReasonBugCheck;
    }
    return bBugCheck;
}

BOOLEAN PVPanicDeregisterBugCheckCallback()
{
    BOOLEAN bBugCheck;

    bBugCheck = KeDeregisterBugCheckCallback(&CallbackRecord);
    if (bSupportCrashLoaded)
    {
        BOOLEAN bReasonBugCheck;
        bReasonBugCheck = KeDeregisterBugCheckReasonCallback(&DumpCallbackRecord);
        return bBugCheck && bReasonBugCheck;
    }

    return bBugCheck;
}
