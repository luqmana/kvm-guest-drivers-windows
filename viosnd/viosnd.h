#ifndef ___VIOSND_H__
#define ___VIOSND_H__

extern "C"
{

    #include <ntddk.h>
    #include <wdf.h>
    #include <portcls.h>

    #include "osdep.h"
    #include "virtio_pci.h"
    #include "virtio.h"
    #include "VirtIOWdf.h"


    NTSTATUS DriverEntry(IN PDRIVER_OBJECT DriverObject, IN PUNICODE_STRING RegistryPath);

}

#endif // ___VIOSND_H__
