/**********************************************************************/
/*                                                                    */
/*              Licensed Materials - Property of IBM                  */
/*              20G0400 (C) Copyright IBM Corp. 1992.                 */
/*                    See Copyright Instructions.                     */
/*                                                                    */
/**********************************************************************/
/**********************************************************************/
/*                                                                    */
/*                         S P I N B U T T O N                        */
/*                                                                    */
/**********************************************************************/

/**********************************************************************/
/* Spinbutton Window Class                                            */
/**********************************************************************/

#define CCL_SPINBTN     "CCLxSpin Button"
#define CCL_SPINBTN_DLL "FCLSPBW.DLL"

/**********************************************************************/
/* SPINBUTTON Creation Flags                                          */
/**********************************************************************/

/**********************************************************************/
/* Character Acceptance                                               */
/**********************************************************************/
#define SPBS_ALLCHARACTERS 0x00000000L /* Default: All chars accepted */
#define SPBS_NUMERICONLY   0x00000001L /* Only 0 - 9 accepted & VKeys */
#define SPBS_READONLY      0x00000002L /* No chars allowed in entryfld*/

/**********************************************************************/
/* Type of Component                                                  */
/**********************************************************************/
#define SPBS_MASTER        0x00000010L
#define SPBS_SERVANT       0x00000000L /* Default: Servant            */

/**********************************************************************/
/* Type of Justification                                              */
/**********************************************************************/
#define SPBS_JUSTDEFAULT  0x00000000L /* Default: Same as Left        */
#define SPBS_JUSTLEFT     0x00000008L
#define SPBS_JUSTRIGHT    0x00000004L
#define SPBS_JUSTCENTER   0x0000000CL

/**********************************************************************/
/* Border or not                                                      */
/**********************************************************************/
#define SPBS_NOBORDER     0x00000020L /* Borderless SpinField         */
                                      /* Default is to have a border. */

/**********************************************************************/
/* Fast spin or not                                                   */
/**********************************************************************/
#define SPBS_FASTSPIN     0x00000100L /* Allow fast spinning.  Fast   */
                                      /* spinning is performed by     */
                                      /* skipping over numbers        */

/**********************************************************************/
/* Pad numbers on front with 0's                                      */
/**********************************************************************/
#define SPBS_PADWITHZEROS 0x00000080L /* Pad the number with zeroes   */

/**********************************************************************/
/* SPINBUTTON Messages                                                */
/**********************************************************************/

/**********************************************************************/
/* Notification from Spinbutton to the application is sent in a       */
/* WM_CONTROL message.                                                */
/**********************************************************************/
#define SPBN_UPARROW       0x20A      /* up arrow button was pressed  */
#define SPBN_DOWNARROW     0x20B      /* down arrow button was pressed*/
#define SPBN_ENDSPIN       0x20C      /* mouse button was released    */
#define SPBN_CHANGE        0x20D      /* spinfield text has changed   */
#define SPBN_SETFOCUS      0x20E      /* spinfield received focus     */
#define SPBN_KILLFOCUS     0x20F      /* spinfield lost focus         */

/**********************************************************************/
/* Messages from application to Spinbutton                            */
/**********************************************************************/
#define SPBM_OVERRIDESETLIMITS  (WM_USER+0x200) /* Set spinbtn limits   */
#define SPBM_QUERYLIMITS        (WM_USER+0x201) /* Query spinbtn limits */
#define SPBM_SETTEXTLIMIT       (WM_USER+0x202) /* Max entryfield chars */
#define SPBM_SPINUP             (WM_USER+0x203) /* spin entry field up  */
#define SPBM_SPINDOWN           (WM_USER+0x204) /* spin entry field down*/
#define SPBM_QUERYVALUE         (WM_USER+0x205) /* Tell entry field to  */
                                                /*  send current value  */

/**********************************************************************/
/* Query Flags                                                        */
/**********************************************************************/
#define SPBQ_UPDATEIFVALID    0       /* Default                      */
#define SPBQ_ALWAYSUPDATE     1
#define SPBQ_DONOTUPDATE      3

/**********************************************************************/
/* Return value for Empty Field.                                      */
/*    If ptr too long, variable sent in query msg                     */
/*                                                                    */
/* NOTE:  Do not use WM_USER + 0x20A                                  */
/**********************************************************************/
#define SPBM_SETARRAY        (WM_USER+0x206)  /* Change data to spin  */
#define SPBM_SETLIMITS       (WM_USER+0x207)  /* Change numeric Limits*/
#define SPBM_SETCURRENTVALUE (WM_USER+0x208)  /* Change current value */
#define SPBM_SETMASTER       (WM_USER+0x209)  /* Tell entryfield who  */
                                              /* its master is        */

/**********************************************************************/
/* Spin button limits structure                                       */
/**********************************************************************/
typedef struct _SPBLIMITS
{
   LONG       lUpperLimit;
   LONG       lLowerLimit;
} SPBLIMITS;
typedef SPBLIMITS FAR *LPSPBLIMITS;

/**********************************************************************/
/* Spin button query value structure                                  */
/**********************************************************************/
typedef struct _SPBQUERYVALUE
{
   LPVOID     lpValueBuf;
   WORD       wBufSize;
   WORD       wValue;
} SPBQUERYVALUE;
typedef SPBQUERYVALUE FAR *LPSPBQUERYVALUE;
