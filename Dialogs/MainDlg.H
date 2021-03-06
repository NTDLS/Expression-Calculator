#ifndef _MAINDLG_H
#define _MAINDLG_H
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//---------------------(Constant Definitions)
#define MAINDIALOG_MENU_ABOUT 1000

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//---------------------(Variable Declarations)
extern HWND hMainDialog;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//---------------------(Function Prototypes)
INT_PTR CALLBACK MainDialog(HWND hHwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void ReplaceVariable(const char *sName, const char *sExpression);
void InsertVariable(const char *sName, const char *sExpression);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
