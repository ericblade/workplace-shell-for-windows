/*
 * DriveType
 *
 * Purpose:
 *  Augments the Windows API GetDriveType with a call to the CD-ROM
 *  extensions to determine if a drive is a floppy, hard disk,
 *  CD-ROM, RAM-drive, or networked  drive.
 *
 * Parameters:
 *  iDrive       UINT containing the zero-based drive index.
 *
 * Return Value:
 *  UINT         One of the following values describing the drive:
 *               DRIVE_FLOPPY, DRIVE_HARD, DRIVE_CDROM, DRIVE_RAM,
 *               DRIVE_NETWORK.

 */

UINT DriveType(UINT iDrive)
    {
    int     iType;
    BOOL    fCDROM=FALSE;
    BOOL    fRAM=FALSE;


    //Validate possible drive indices.
    if (0 > iDrive  || 25 < iDrive)
        return 0xFFFF;

    iType=GetDriveType(iDrive);

    /*
     * Under Windows NT, GetDriveType returns complete information
     * not provided under Windows 3.x, which we now get through 
     * other means.
     */
   #ifdef WIN32
    return iType;
   #else

    //Check for CD-ROM on FIXED and REMOTE drives only.
    if (DRIVE_FIXED==iType || DRIVE_REMOTE==iType)
        {
        _asm
            {
            mov     ax,1500h        //Check if MSCDEX exists.
            xor     bx,bx
            int     2fh

            or      bx,bx    //BX unchanged if MSCDEX is not around.
            jz      CheckRAMDrive   //No? Go check for RAM drive.

            mov     ax,150Bh  //Check if drive is using CD driver.

            mov     cx,iDrive
            int     2fh

            mov     fCDROM,ax    //AX if the CD-ROM flag.
            or      ax,ax
            jnz     Exit         //Leave if we found a CD-ROM drive.

            CheckRAMDrive:
            }
        }

    //Check for RAM drives on FIXED disks only.
    if (DRIVE_FIXED==iType)
        {
        /*
         * Check for RAM drive is done by reading the boot sector
         * and looking at the number of FATs. RAM disks only have 1

         * while all others have 2.
         */
        _asm
            {
            push    ds

            mov     bx,ss
            mov     ds,bx

            sub     sp,0200h    //Reserve 512 bytes to read a sector
            mov     bx,sp       //and point BX there.

            mov     ax,iDrive   //Read the boot sector of the drive.
            mov     cx,1
            xor     dx,dx

            int     25h
            add     sp,2        //Int 25h requires stack cleanup.

            jc      DriveNotRAM

            mov     bx,sp
            cmp     ss:[bx+15h],0f8h    //Reverify fixed disk.
            jne     DriveNotRAM
            cmp     ss:[bx+10h],1  //Check if there's only one FAT.
            jne     DriveNotRAM
            mov     fRAM,1

            DriveNotRAM:
            add     sp,0200h
            pop     ds

            Exit:
            //Leave fRAM untouched--it's FALSE by default.
            }
        }


    /*
     * If either CD-ROM or RAM drive flags are set, return privately
     * defined flags for them (outside of Win32). Otherwise, return
     * the type given from GetDriveType.
     */

    if (fCDROM)
        return DRIVE_CDROM;

    if (fRAM)
        return DRIVE_RAM;

    //Drive B on a one-drive system returns < 2 from GetDriveType.
    return iType;
   #endif
    }
