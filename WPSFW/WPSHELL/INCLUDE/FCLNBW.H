/**********************************************************************/
/*                                                                    */
/*              Licensed Materials - Property of IBM                  */
/*              20G0400 (C) Copyright IBM Corp. 1992.                 */
/*                    See Copyright Instructions.                     */
/*                                                                    */
/**********************************************************************/
/**********************************************************************/
/*                                                                    */
/*                          N O T E B O O K                           */
/*                                                                    */
/**********************************************************************/

#define CCL_NOTEBOOK      "CCL Notebook"
#define CCL_NOTEBOOK_DLL  "FCLNBW.DLL"

/**********************************************************************/
/* Ownerdraw id                                                       */
/**********************************************************************/
#define ODT_NOTEBOOK 101

/**********************************************************************/
/* Message ids                                                        */
/**********************************************************************/
#define BKM_CALCPAGERECT         (WM_USER+0x0356) /* Calc book/page rectangle  */
#define BKM_DELETEPAGE           (WM_USER+0x0357) /* Delete page(s)            */
#define BKM_INSERTPAGE           (WM_USER+0x0358) /* Insert page               */
#define BKM_INVALIDATETABS       (WM_USER+0x0359) /* Invalidate tab area       */
#define BKM_TURNTOPAGE           (WM_USER+0x035a) /* Turn to page              */
#define BKM_QUERYPAGECOUNT       (WM_USER+0x035b) /* Query number of pages     */
#define BKM_QUERYPAGEID          (WM_USER+0x035c) /* Query page identifier     */
#define BKM_QUERYPAGEDATA        (WM_USER+0x035d) /* Query page user data      */
#define BKM_QUERYPAGEWINDOWHWND  (WM_USER+0x035e) /* Query page window handle  */
#define BKM_QUERYTABBITMAP       (WM_USER+0x035f) /* Query tab bitmap handle   */
#define BKM_QUERYTABTEXT         (WM_USER+0x0360) /* Query tab text pointer    */
#define BKM_SETDIMENSIONS        (WM_USER+0x0361) /* Set tab/dogear dimensions */
#define BKM_SETPAGEDATA          (WM_USER+0x0362) /* Set page user data        */
#define BKM_SETPAGEWINDOWHWND    (WM_USER+0x0363) /* Set page window handle    */
#define BKM_SETSTATUSLINETEXT    (WM_USER+0x0364) /* Set status line text      */
#define BKM_SETTABBITMAP         (WM_USER+0x0365) /* Set tab bitmap            */
#define BKM_SETTABTEXT           (WM_USER+0x0366) /* Set tab text              */
#define BKM_SETNOTEBOOKCOLORS    (WM_USER+0x0367) /* Set Notebook colors       */
#define BKM_QUERYPAGESTYLE       (WM_USER+0x0368) /* Query page style          */
#define BKM_QUERYSTATUSLINETEXT  (WM_USER+0x0369) /* Query status line text    */

#define BKN_PAGESELECTED         130     /* New page selected by user */
#define BKN_NEWPAGESIZE          131     /* App page size changed     */
#define BKN_HELP                 132     /* Help notification         */
#define BKN_PAGEDELETED          133     /* Page deleted notification */

/**********************************************************************/
/* Page deletion flags (usDeleteFlag)                                 */
/**********************************************************************/
#define BKA_ALL                  0x0001  /* all pages                 */
#define BKA_SINGLE               0x0002  /* single page               */
#define BKA_TAB                  0x0004  /* minor/major section       */

/**********************************************************************/
/* Page insertion/query order (usPageOrder, usQueryOrder)             */
/**********************************************************************/
#define BKA_LAST                 0x0002  /* Insert/Query last page    */
#define BKA_FIRST                0x0004  /* Insert/Query first page   */
#define BKA_NEXT                 0x0008  /* Insert/Query after page   */
#define BKA_PREV                 0x0010  /* Insert/Query before page  */
#define BKA_TOP                  0x0020  /* Query topmost page        */
#define BKA_DELETED              0x0040  /* Query deleted page        */
#define BKA_PREVSELECTED         0x0080  /* Query prev selected page  */

/**********************************************************************/
/* Notebook region types (usBookRegion, usType)                       */
/**********************************************************************/
#define BKA_MAJORTAB             0x0001  /* Major Tab                 */
#define BKA_MINORTAB             0x0002  /* Minor Tab                 */
#define BKA_PAGEBUTTON           0x0100  /* Page Turning Button       */

/**********************************************************************/
/* Page insertion/query styles (usPageStyle,usQueryEnd)               */
/**********************************************************************/
#define BKA_STATUSTEXTON         0x0001  /* status area text          */
#define BKA_MAJOR                0x0040  /* Major Tab                 */
#define BKA_MINOR                0x0080  /* Minor Tab                 */
#define BKA_AUTOPAGESIZE         0x0100  /* Page window position/size */
#define BKA_END                  0x0200  /* Query to end of book      */

/**********************************************************************/
/* Tab window contents (usTabDisplay)                                 */
/**********************************************************************/
#define BKA_TEXT                 0x0400  /* text data                 */
#define BKA_BITMAP               0x0800  /* bitmap                    */

/**********************************************************************/
/* Notebook window styles (ulNotebookStyles)                          */
/**********************************************************************/

/**********************************************************************/
/* Backpage Orientation                                               */
/**********************************************************************/
#define BKS_BACKPAGESBR          0x00000001  /* Bottom right          */
#define BKS_BACKPAGESBL          0x00000002  /* Bottom left           */
#define BKS_BACKPAGESTR          0x00000004  /* Top right             */
#define BKS_BACKPAGESTL          0x00000008  /* Top left              */

/**********************************************************************/
/* Major Tab Side                                                     */
/**********************************************************************/
#define BKS_MAJORTABRIGHT        0x00000010  /* Major tabs right      */
#define BKS_MAJORTABLEFT         0x00000020  /* Major tabs left       */
#define BKS_MAJORTABTOP          0x00000040  /* Major tabs top        */
#define BKS_MAJORTABBOTTOM       0x00000080  /* Major tabs bottom     */

/**********************************************************************/
/* Tab Type                                                           */
/**********************************************************************/
#define BKS_SQUARETABS           0x00000000  /* Square edged tabs     */
#define BKS_ROUNDEDTABS          0x00000100  /* Round  edged tabs     */
#define BKS_POLYGONTABS          0x00000200  /* Polygon edged tabs    */

/**********************************************************************/
/* Binding type                                                       */
/**********************************************************************/
#define BKS_SOLIDBIND            0x00000000  /* Solid binding         */
#define BKS_SPIRALBIND           0x00000400  /* Spiral binding        */

/**********************************************************************/
/* Status line text justification                                     */
/**********************************************************************/
#define BKS_STATUSTEXTLEFT       0x00000000  /* left justify text     */
#define BKS_STATUSTEXTRIGHT      0x00001000  /* right justify text    */
#define BKS_STATUSTEXTCENTER     0x00002000  /* center text           */

/**********************************************************************/
/* Tab text justification                                             */
/**********************************************************************/
#define BKS_TABTEXTCENTER        0x00000000  /* Center tab text       */
#define BKS_TABTEXTLEFT          0x00004000  /* Left justify tab text */
#define BKS_TABTEXTRIGHT         0x00008000  /* Right justify tab text*/

/**********************************************************************/
/* Notebook color attributes                                          */
/**********************************************************************/
#define BKA_BACKGROUNDCOLOR           0x0001
#define BKA_FOREGROUNDCOLOR           0x0002
#define BKA_BORDERCOLOR               0x0003
#define BKA_CURSORCOLOR               0x0004
#define BKA_BACKGROUNDPAGECOLOR       0x0005
#define BKA_BACKGROUNDMAJORCOLOR      0x0006
#define BKA_BACKGROUNDMINORCOLOR      0x0007
#define BKA_FOREGROUNDMAJORCOLOR      0x0008
#define BKA_FOREGROUNDMINORCOLOR      0x0009

/**********************************************************************/
/* Error message ids                                                  */
/**********************************************************************/
#define BOOKERR_INVALID_PARAMETERS   -1L /* Invalid parameters        */

/**********************************************************************/
/* BKM_QUERYTABTEXT message structure                                 */
/**********************************************************************/
typedef struct _PBOOKTEXT {  /* ptabtxt */
  LPSTR  pString;                        /* ptr to string buffer      */
  WORD   textLen;                        /* length of string to query */
} BOOKTEXT;
typedef BOOKTEXT FAR *PBOOKTEXT;
