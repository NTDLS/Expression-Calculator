#ifndef _MAINDLG_CPP
#define _MAINDLG_CPP
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Windows.H>
#include <Stdio.H>
#include <Stdlib.H>
#include <CommCtrl.H>
#include <limits.H>

#include "../Source/Entry.H"

#include "../Dialogs/MainDlg.H"
#include "../Dialogs/VarDlg.H"
#include "../Dialogs/NetLogoDlg.H"

#include "../../CMathParser/CMathParser.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace NSWFL::Windows;
using namespace NSWFL::Registry;
using namespace NSWFL::File;
using namespace NSWFL::String;
using namespace NSWFL::Collections;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//---------------------(Variable Declarations)

#define WM_INITCUSTOMPROC (WM_USER + 1000)
HWND hMainDialog = NULL;
char *gExpression = NULL;
HWND hExpression = NULL;
HWND hVariables = NULL;
HWND hResultList = NULL;
char sLastVariableFile[MAX_PATH];

Stack QS(BEHAVIOR_FILO);

bool bShowWork = false;
bool bSmartRounding = true;
bool bIntergerMath = false;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void LoadVariables(char *sFileName)
{
	ListView_DeleteAllItems(hVariables);

	StringBuilder variables;

	if(variables.AppendFromFile(sFileName))
	{
		int iLineCount = 0;
		char **sLines = Split(variables.Buffer, variables.Length, &iLineCount, "\n");

		if(sLines != NULL && iLineCount > 0)
		{
			for(int iLine = 0; iLine < iLineCount; iLine++)
			{
				int iPairCount = 0;
				char **variablePair = Split(sLines[iLine], (int)strlen(sLines[iLine]), &iPairCount, "\t");
				if(variablePair != NULL && iPairCount == 2)
				{
					ReplaceCharacter(variablePair[0], '\t', ' ');
					ReplaceCharacter(variablePair[1], '\t', ' ');

					Trim(variablePair[0]);
					Trim(variablePair[1]);

					InsertVariable(variablePair[0], variablePair[1]);
				}

				FreeArray(variablePair, iPairCount);
			}
		}
		FreeArray(sLines, iLineCount);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void SaveVariables(char *sFileName)
{
	StringBuilder variables;

	char sVarName[MAX_RESULT];
	char sVarValue[MAX_RESULT];

	memset(sVarName, 0, sizeof(sVarName));
	memset(sVarValue, 0, sizeof(sVarValue));
		
	int iIndex = -1;
	while((iIndex = ListView_GetNextItem(hVariables, iIndex, LVNI_ALL)) >= 0)
	{
		ListView_GetItemText(hVariables, iIndex, 0, sVarName, sizeof(sVarName));
		ListView_GetItemText(hVariables, iIndex, 1, sVarValue, sizeof(sVarValue));

		ReplaceCharacter(sVarName, '\t', ' ');
		ReplaceCharacter(sVarValue, '\t', ' ');

		variables.AppendF("%s\t%s\r\n", sVarName, sVarValue);
	}

	variables.WriteToFile(sFileName);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

LRESULT CALLBACK ExpressionEditProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static WNDPROC lpOldProc;
	static HWND hOwner;

	if(uMsg == WM_INITCUSTOMPROC)
	{
		memcpy(&lpOldProc, (WNDPROC *)lParam, sizeof(WNDPROC));
		memcpy(&hOwner, (WNDPROC *)wParam, sizeof(HWND));
	}
    else if(uMsg == WM_GETDLGCODE)
	{
		return (DLGC_WANTALLKEYS | CallWindowProc(lpOldProc, hWnd, uMsg, wParam, lParam));
	}
	else if(uMsg == WM_CHAR)
	{
		if(wParam == VK_RETURN)
		{
			return 0;
		}
	}
	else if(uMsg == WM_KEYDOWN)
	{
		if(wParam == VK_RETURN)
		{
		}
		else if(wParam == VK_ESCAPE)
		{
			Set_Text(hWnd, "");
			return 0;
		}
	}

	return CallWindowProc(lpOldProc, hWnd, uMsg, wParam, lParam);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

LRESULT CALLBACK VariableListProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static WNDPROC lpOldProc;
	static HWND hOwner;

	if(uMsg == WM_INITCUSTOMPROC)
	{
		memcpy(&lpOldProc, (WNDPROC *)lParam, sizeof(WNDPROC));
		memcpy(&hOwner, (WNDPROC *)wParam, sizeof(HWND));
	}
    else if(uMsg == WM_GETDLGCODE)
	{
		return (DLGC_WANTALLKEYS | CallWindowProc(lpOldProc, hWnd, uMsg, wParam, lParam));
	}
    else if(uMsg == WM_LBUTTONDBLCLK)
    {
		int iCurrentSel = ListView_GetNextItem(hWnd, -1, LVNI_SELECTED);
		if(iCurrentSel >= 0)
		{
			char sExpression[MAX_RESULT];

			ListView_GetItemText(hWnd, iCurrentSel, 0, sExpression, sizeof(sExpression));
			TextBoxInsert(hExpression, " ${");
			TextBoxInsert(hExpression, sExpression);
			TextBoxInsert(hExpression, "}");
			SetFocus(hExpression);
		}
	}
	else if(uMsg == WM_COMMAND)
    {
        if(wParam == ID_VARIABLE_ADD)
        {
			strcpy_s(gsVarName, sizeof(gsVarName), "");
			strcpy_s(gsVarExpr, sizeof(gsVarExpr), "");
			DialogBox(ghAppInstance, MAKEINTRESOURCE(IDD_VARIABLE), hOwner, (DLGPROC)VarDialog);
		}
        else if(wParam == ID_VARIABLE_EDIT)
        {
			int iCurrentSel = ListView_GetNextItem(hWnd, -1, LVNI_SELECTED);
			if(iCurrentSel >= 0)
			{
				ListView_GetItemText(hWnd, iCurrentSel, 0, gsVarName, sizeof(gsVarName));
				ListView_GetItemText(hWnd, iCurrentSel, 1, gsVarExpr, sizeof(gsVarExpr));

				DialogBox(ghAppInstance, MAKEINTRESOURCE(IDD_VARIABLE), hOwner, (DLGPROC)VarDialog);
			}
		}
        else if(wParam == ID_VARIABLE_DELETE)
        {
			int iIndex = ListView_GetNextItem(hWnd, -1, LVNI_SELECTED);
			ListView_DeleteItem(hWnd, iIndex);
			if(iIndex >= ListView_GetItemCount(hWnd))
			{
				iIndex--;
			}
			ListView_SetItemState(hWnd, iIndex, LVIS_SELECTED, LVIS_SELECTED);
		}
        else if(wParam == ID_VARIABLE_DELETEALL)
        {
			if(MessageBox(hWnd, "Are you sure you want to delete all variables?", gsTitleCaption, MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON2) == IDYES)
			{
				ListView_DeleteAllItems(hWnd);
			}
		}
	}
	else if(uMsg == WM_RBUTTONDOWN)
    {
		CallWindowProc(lpOldProc, hWnd, uMsg, wParam, lParam);

		int iSelected = ListView_GetSelectedCount(hWnd);

		HMENU hParent = LoadMenu(ghAppInstance, MAKEINTRESOURCE(IDR_VARPOPUP));
		HMENU hChild = GetSubMenu(hParent, 0);

		if(iSelected != 1)
		{
			EnableMenuItem(hChild, ID_VARIABLE_EDIT, MF_GRAYED);
			EnableMenuItem(hChild, ID_VARIABLE_DELETE, MF_GRAYED);
		}
	
        POINT xy;
        GetCursorPos(&xy);
        TrackPopupMenuEx(hChild, TPM_LEFTALIGN|TPM_RIGHTBUTTON, (xy.x) + 1, xy.y, hWnd, NULL);
        DestroyMenu(hChild);
        DestroyMenu(hParent);
		return 0;
    }
	else if(uMsg == WM_CHAR)
	{
		if(wParam == VK_RETURN)
		{
			return 0;
		}
		else if(wParam == VK_TAB)
		{
			return 0;
		}
	}
	else if(uMsg == WM_KEYDOWN)
	{
		if(wParam == VK_TAB)
		{
			PostMessage(hOwner, WM_NEXTDLGCTL, 0, 0L);
			return 0;
		}
		else if(wParam == VK_DELETE)
		{
			int iIndex = ListView_GetNextItem(hWnd, -1, LVNI_SELECTED);
			ListView_DeleteItem(hWnd, iIndex);
			if(iIndex >= ListView_GetItemCount(hWnd))
			{
				iIndex--;
			}
			ListView_SetItemState(hWnd, iIndex, LVIS_SELECTED, LVIS_SELECTED);
		}
	}

	return CallWindowProc(lpOldProc, hWnd, uMsg, wParam, lParam);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

LRESULT CALLBACK ResultListProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static WNDPROC lpOldProc;
	static HWND hOwner;

	if(uMsg == WM_INITCUSTOMPROC)
	{
		memcpy(&lpOldProc, (WNDPROC *)lParam, sizeof(WNDPROC));
		memcpy(&hOwner, (WNDPROC *)wParam, sizeof(HWND));
	}
	else if(uMsg == WM_COMMAND)
	{
		if(wParam == ID_RESULT_ADDVARIABLE)
		{
			int iCurrentSel = ListView_GetNextItem(hWnd, -1, LVNI_SELECTED);
			if(iCurrentSel >= 0)
			{
				char sResult[MAX_RESULT];
				ListView_GetItemText(hWnd, iCurrentSel, 0, sResult, sizeof(sResult));

				strcpy_s(gsVarName, sizeof(gsVarName), "");
				strcpy_s(gsVarExpr, sizeof(gsVarExpr), sResult);
				DialogBox(ghAppInstance, MAKEINTRESOURCE(IDD_VARIABLE), hOwner, (DLGPROC)VarDialog);
			}
			return 0;
		}
		else if(wParam == ID_RESULT_COPYTOEXPRESSION)
		{
			int iCurrentSel = ListView_GetNextItem(hWnd, -1, LVNI_SELECTED);
			if(iCurrentSel >= 0)
			{
				char sResult[MAX_RESULT];
				ListView_GetItemText(hWnd, iCurrentSel, 0, sResult, sizeof(sResult));

				TextBoxInsert(hExpression, " ");
				TextBoxInsert(hExpression, sResult);
				TextBoxInsert(hExpression, " ");
				SetFocus(hExpression);
			}
			return 0;
		}
		else if(wParam == ID_RESULT_COPYTOCLIPBOARD)
		{
			int iCurrentSel = ListView_GetNextItem(hWnd, -1, LVNI_SELECTED);
			if(iCurrentSel >= 0)
			{
				char sResult[MAX_RESULT];
				ListView_GetItemText(hWnd, iCurrentSel, 0, sResult, sizeof(sResult));

				if(OpenClipboard(NULL))
				{
					HANDLE hMemory = NULL;

					size_t iSize = strlen(sResult);
					if((hMemory = LocalAlloc(LPTR, iSize + 1)))
					{
						EmptyClipboard();
						hMemory = LocalLock(hMemory); 
						memcpy(hMemory, sResult, iSize); 
						LocalUnlock(hMemory); 

						SetClipboardData(CF_TEXT, hMemory); 

						//LocalFree(hMemory); //Not supposed to free. Odd.
					}
					CloseClipboard();
				}
			}

			return 0;
		}
		else if(wParam == ID_RESULT_DELETE)
		{
			int iIndex = ListView_GetNextItem(hWnd, -1, LVNI_SELECTED);
			ListView_DeleteItem(hWnd, iIndex);
			if(iIndex >= ListView_GetItemCount(hWnd))
			{
				iIndex--;
			}
			ListView_SetItemState(hWnd, iIndex, LVIS_SELECTED, LVIS_SELECTED);
			return 0;
		}
		else if(wParam == ID_RESULT_DELETEALL)
		{
			int iIndex = ListView_DeleteAllItems(hWnd);
			return 0;
		}
	}
    else if(uMsg == WM_GETDLGCODE)
	{
		return (DLGC_WANTALLKEYS | CallWindowProc(lpOldProc, hWnd, uMsg, wParam, lParam));
	}
    else if(uMsg == WM_LBUTTONDBLCLK)
    {
		int iCurrentSel = ListView_GetNextItem(hWnd, -1, LVNI_SELECTED);
		if(iCurrentSel >= 0)
		{
			char sResult[MAX_RESULT];
			ListView_GetItemText(hWnd, iCurrentSel, 0, sResult, sizeof(sResult));

			TextBoxInsert(hExpression, " ");
			TextBoxInsert(hExpression, sResult);
			TextBoxInsert(hExpression, " ");
			SetFocus(hExpression);
		}
	}
	else if(uMsg == WM_RBUTTONDOWN)
    {
		CallWindowProc(lpOldProc, hWnd, uMsg, wParam, lParam);

		int iSelected = ListView_GetSelectedCount(hWnd);

		HMENU hParent = LoadMenu(ghAppInstance, MAKEINTRESOURCE(IDR_RESULTPOPUP));
		HMENU hChild = GetSubMenu(hParent, 0);

		if(iSelected == 0)
		{
			EnableMenuItem(hChild, ID_RESULT_ADDVARIABLE, MF_GRAYED);
			EnableMenuItem(hChild, ID_RESULT_COPYTOEXPRESSION, MF_GRAYED);
			EnableMenuItem(hChild, ID_RESULT_COPYTOCLIPBOARD, MF_GRAYED);
			EnableMenuItem(hChild, ID_RESULT_DELETE, MF_GRAYED);
		}

		POINT xy;
		GetCursorPos(&xy);
		TrackPopupMenuEx(hChild, TPM_LEFTALIGN|TPM_RIGHTBUTTON, (xy.x) + 1, xy.y, hWnd, NULL);
		DestroyMenu(hChild);
	    DestroyMenu(hParent);
		
		return 0;
    }
	else if(uMsg == WM_CHAR)
	{
		if(wParam == VK_RETURN)
		{
			return 0;
		}
		else if(wParam == VK_TAB)
		{
			return 0;
		}
	}
	else if(uMsg == WM_KEYDOWN)
	{
		if(wParam == VK_TAB)
		{
			PostMessage(hOwner, WM_NEXTDLGCTL, 0, 0L);
			return 0;
		}
		else if(wParam == VK_DELETE)
		{
			int iIndex = ListView_GetNextItem(hWnd, -1, LVNI_SELECTED);
			ListView_DeleteItem(hWnd, iIndex);
			if(iIndex >= ListView_GetItemCount(hWnd))
			{
				iIndex--;
			}
			ListView_SetItemState(hWnd, iIndex, LVIS_SELECTED, LVIS_SELECTED);
		}
	}

	return CallWindowProc(lpOldProc, hWnd, uMsg, wParam, lParam);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AddExpressionEditHandler(HWND hOwner, HWND hWnd)
{
	WNDPROC gpOldProc = (WNDPROC) SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LRESULT) &ExpressionEditProc);
	SendMessage(hWnd, WM_INITCUSTOMPROC, (WPARAM)&hOwner, (LPARAM) &gpOldProc);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AddVariableListHandler(HWND hOwner, HWND hWnd)
{
	WNDPROC gpOldProc = (WNDPROC) SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LRESULT) &VariableListProc);
	SendMessage(hWnd, WM_INITCUSTOMPROC, (WPARAM)&hOwner, (LPARAM) &gpOldProc);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AddResultListHandler(HWND hOwner, HWND hWnd)
{
	WNDPROC gpOldProc = (WNDPROC) SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LRESULT) &ResultListProc);
	SendMessage(hWnd, WM_INITCUSTOMPROC, (WPARAM)&hOwner, (LPARAM) &gpOldProc);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void InsertVariable(const char *sName, const char *sValue)
{
	int iItem = ListView_GetItemCount(hVariables);

	LV_ITEM lvItem;
    memset(&lvItem, 0, sizeof(lvItem));

	char sVarName[MAX_RESULT];
	char sVarValue[MAX_RESULT];
	strcpy_s(sVarName, sizeof(sVarName), sName);
	strcpy_s(sVarValue, sizeof(sVarValue), sValue);

	ReplaceCharacter(sVarName, '\t', ' ');
	ReplaceCharacter(sVarValue, '\t', ' ');

    lvItem.pszText = (char *)sVarName;
    lvItem.mask = LVIF_TEXT;
	lvItem.iItem = iItem;
    lvItem.iSubItem = 0;
    ListView_InsertItem(hVariables, &lvItem);

    lvItem.pszText  = (char *)sVarValue;
    lvItem.mask     = LVIF_TEXT;
	lvItem.iItem = iItem;
    lvItem.iSubItem = 1;
    ListView_SetItem(hVariables, &lvItem);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ReplaceVariable(const char *sName, const char *sValue)
{
	char sVarName[MAX_RESULT];
	char sVarValue[MAX_RESULT];
	strcpy_s(sVarName, sizeof(sVarName), sName);
	strcpy_s(sVarValue, sizeof(sVarValue), sValue);

	ReplaceCharacter(sVarName, '\t', ' ');
	ReplaceCharacter(sVarValue, '\t', ' ');

	int iIndex = -1;
	
	char sVar[MAX_RESULT];
	bool bFound = false;
		
	while((iIndex = ListView_GetNextItem(hVariables, iIndex, LVNI_ALL)) >= 0)
	{
		ListView_GetItemText(hVariables, iIndex, 0, sVar, sizeof(sVar));
		if(_strcmpi(sName, sVar) == 0)
		{
			LV_ITEM lvItem;
			memset(&lvItem, 0, sizeof(lvItem));

			lvItem.pszText = (char *)sVarValue;
			lvItem.mask = LVIF_TEXT;
			lvItem.iItem = iIndex;
			lvItem.iSubItem = 1;
			ListView_SetItem(hVariables, &lvItem);
			return;
		}
	}

	InsertVariable(sName, sVarValue);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void InsertResult(const char *sText)
{
	LV_ITEM lvItem;
	memset(&lvItem, 0, sizeof(lvItem));

	int iCount = ListView_GetItemCount(hResultList);

	lvItem.pszText = (char *)sText;
	lvItem.mask = LVIF_TEXT;
	lvItem.iItem = iCount;
	lvItem.iSubItem = 0;
	ListView_InsertItem(hResultList, &lvItem);

	ListView_EnsureVisible(hResultList, iCount, FALSE);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MathDebugTextCallback(const char *sText)
{
	StringBuilder text(sText);

	text.Replace("\t", "    ");
	text.Replace("\n", "");

	InsertResult(text.Buffer);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

INT_PTR CALLBACK MainDialog(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static HMENU hMenu = NULL;
	static size_t iQueuPos = 0;

    //--------------------------------------------------------------------------

    if(uMsg == WM_INITDIALOG) // Received A Initilize Dialog Message
    {
        hMainDialog = hWnd;

        HMENU hContextMenu = GetSystemMenu(hWnd, FALSE);
        AppendMenu(hContextMenu, MF_SEPARATOR, 0, 0);
        AppendMenu(hContextMenu, MF_STRING, MAINDIALOG_MENU_ABOUT, "About");

		hMenu = LoadMenu(ghAppInstance, MAKEINTRESOURCE(IDR_MAINMENU));
		SetMenu(hWnd, hMenu);
		
		SendMessage(hWnd, (UINT)WM_SETTEXT, (WPARAM)0, (LPARAM)gsTitleCaption);
        SendMessage(hWnd, WM_SETICON, TRUE, (LPARAM) LoadIcon(ghAppInstance, MAKEINTRESOURCE(IDI_MAIN)));

        hExpression = GetDlgItem(hWnd, IDC_EXPRESSION);
        hResultList = GetDlgItem(hWnd, IDC_RESULTLIST);
        hVariables  = GetDlgItem(hWnd, IDC_VARIABLES);
		
		SendMessage(hResultList, (UINT)LVM_SETEXTENDEDLISTVIEWSTYLE,
			(WPARAM)0, (LPARAM)LVS_EX_HEADERDRAGDROP|LVS_EX_FULLROWSELECT);

		SendMessage(hVariables, (UINT)LVM_SETEXTENDEDLISTVIEWSTYLE,
			(WPARAM)0, (LPARAM)LVS_EX_HEADERDRAGDROP|LVS_EX_FULLROWSELECT);

		ListView_SetTextColor(hVariables, RGB(0,0,150));

		LV_COLUMN lvColumn;

		memset(&lvColumn, 0, sizeof(LV_COLUMN));
		lvColumn.cx        = 460;
		lvColumn.pszText   = "Result";
		lvColumn.iSubItem  = 0;
		lvColumn.mask      = LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM | LVCF_FMT;
		lvColumn.fmt       = LVCFMT_LEFT;
		ListView_InsertColumn(hResultList, 0, &lvColumn);

		memset(&lvColumn, 0, sizeof(LV_COLUMN));
		lvColumn.cx        = 64;
		lvColumn.pszText   = "Name";
		lvColumn.iSubItem  = 0;
		lvColumn.mask      = LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM | LVCF_FMT;
		lvColumn.fmt       = LVCFMT_LEFT;
		ListView_InsertColumn(hVariables, 0, &lvColumn);

		lvColumn.cx        = 105;
		lvColumn.pszText   = "Value";
		lvColumn.iSubItem  = 1;
		lvColumn.mask      = LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM | LVCF_FMT;
		lvColumn.fmt       = LVCFMT_LEFT;
		ListView_InsertColumn(hVariables, 1, &lvColumn);

		Set_Text(hExpression, "");

		AddExpressionEditHandler(hWnd, hExpression);
		AddResultListHandler(hWnd, hResultList);
		AddVariableListHandler(hWnd, hVariables);

		InsertVariable("r",  "0");
		InsertVariable("pi", "3.141592653589");
		InsertVariable("e",  "2.718281828459");



		memset(sLastVariableFile, 0, sizeof(sLastVariableFile));
		
        CenterWindow(hWnd);

        return TRUE; // Return TRUE to set the keyboard focus, Otherwise return FALSE.
    }

    //--------------------------------------------------------------------------

    if(uMsg == WM_COMMAND)
    {
		//------------------------------------------------------------------------
        if(wParam == IDC_HISTORY_PREV)
        {
			if(iQueuPos > 0)
			{
				iQueuPos--;
				Set_Text(hExpression, (char *)QS.Peek(iQueuPos)->Value);
				SetFocus(hExpression);
			}

			EnableWindow(GetDlgItem(hWnd, IDC_HISTORY_NEXT), TRUE);
			EnableWindow(GetDlgItem(hWnd, IDC_HISTORY_PREV), !(iQueuPos == 0));
		}
		//------------------------------------------------------------------------
        else if(wParam == IDC_HISTORY_NEXT)
        {
			if(iQueuPos < (QS.StackSize() - 1))
			{
				iQueuPos++;
				Set_Text(hExpression, (char *)QS.Peek(iQueuPos)->Value);
				SetFocus(hExpression);
			}

			EnableWindow(GetDlgItem(hWnd, IDC_HISTORY_PREV), TRUE);
			EnableWindow(GetDlgItem(hWnd, IDC_HISTORY_NEXT), !(iQueuPos >= (QS.StackSize() - 1)));
		}
		//------------------------------------------------------------------------
        else if(wParam == IDC_CALCULATE)
        {
			CMathParser MP;
			StringBuilder sExp;

			sExp.AppendFromTextBox(hExpression);
			if(sExp.Length > 0)
			{
				QS.Push(sExp.Buffer, sExp.Length);
				iQueuPos = QS.StackSize();
				EnableWindow(GetDlgItem(hWnd, IDC_HISTORY_NEXT), FALSE);
			}
			else {
				if(QS.StackSize() > 0)
				{
					sExp.Append((char *)QS.Peek(QS.StackSize() - 1)->Value);
				}
				else {
					return FALSE;
				}
			}

			/*
			int iFirstChar = sExp.FirstCharPos();
			{
				if(iFirstChar >= 0)
				{
					if(MP.IsMathChar(sExp.Buffer[iFirstChar]))
					{
						sExp.Truncate();
						sExp.Append("${R}");
						sExp.AppendTextBox(hExpression);
					}
				}
			}
			*/

			int iTotalReplaces = 0;
			int Replaces = 0;
			do{
				Replaces = 0;

				int iVarPos = -1;
				while((iVarPos = ListView_GetNextItem(hVariables, iVarPos, LVNI_ALL)) >= 0)
				{
					char sVar[MAX_RESULT];
					char sVarText[MAX_RESULT];

					ListView_GetItemText(hVariables, iVarPos, 0, sVar, sizeof(sVar));
					sprintf_s(sVarText, sizeof(sVarText), "${%s}", sVar);
					ListView_GetItemText(hVariables, iVarPos, 1, sVar, sizeof(sVar));

					Replaces += sExp.Replace(sVarText, sVar, false);
					iTotalReplaces += Replaces;
				}

				if(iTotalReplaces > 1000)
				{
					MessageBox(hWnd, "Infinite loop detected while parsing variables.", gsTitleCaption, MB_ICONWARNING);
					sExp.Destroy();
					return FALSE;
				}
			} while(Replaces > 0);

			if(bShowWork)
			{
				MP.DebugMode(true);
				MP.DebugProc(MathDebugTextCallback);
			}

			char sResult[_CVTBUFSIZE];

			if(bIntergerMath)
			{
				int iResult = 0;

				if(MP.Calculate(sExp.Buffer, sExp.Length, &iResult) == CMathParser::ResultOk)
				{
					sprintf_s(sResult, sizeof(sResult), "%d", iResult);
					InsertResult(sResult);
					Set_Text(hExpression, "");
					ReplaceVariable("r", sResult);
					EnableWindow(GetDlgItem(hWnd, IDC_HISTORY_PREV), TRUE);
				}
				else{
					InsertResult(MP.LastError()->Text);
				}
			}
			else 
			{
				double dResult = 0;

				if(MP.Calculate(sExp.Buffer, sExp.Length, &dResult) == CMathParser::ResultOk)
				{
					if(bSmartRounding)
					{
						MP.SmartRound(dResult, sResult, sizeof(sResult));
					}
					else{
						sprintf_s(sResult, sizeof(sResult), "%.64f", dResult);
					}

					InsertResult(sResult);
					Set_Text(hExpression, "");
					ReplaceVariable("r", sResult);
					EnableWindow(GetDlgItem(hWnd, IDC_HISTORY_PREV), TRUE);
				}
				else{
					InsertResult(MP.LastError()->Text);
				}
			}

			sExp.Destroy();

			SetFocus(hExpression);
			return TRUE;
        }
		//------------------------------------------------------------------------
        else if(wParam == ID_HELP_DOCUMENTATION)
        {
			char sPath[MAX_PATH];
			char sHelpFile[MAX_PATH];
			DWORD dwPathSz = sizeof(sPath);
			if(Get_StringRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "Path", sPath, dwPathSz))
			{
				strcpy_s(sHelpFile, sizeof(sHelpFile), sPath);
				strcat_s(sHelpFile, sizeof(sHelpFile), "\\help.chm");
				ShellExecute(hWnd, "open", sHelpFile, NULL, sPath, SW_SHOW);
			}
			else{
				MessageBox(hWnd, "Registry key not found.", gsTitleCaption, MB_ICONEXCLAMATION);
			}

			return TRUE;
        }
		//------------------------------------------------------------------------
		else if(wParam == ID_HELP_ABOUT)
        {
            _AboutDialogInfo ADI;
    
            ADI.DisplayIcon  = LoadIcon(ghAppInstance, MAKEINTRESOURCE(IDI_MAIN));
            ADI.TitleCaption = gsTitleCaption;
            ADI.FileVersion  = gsFileVersion;
            ADI.BuildDate    = __DATE__;
            ADI.BuildTime    = __TIME__;
            ADI.CopyRight    = gsAppCopyRight;
            ADI.OwnerHandle  = hWnd;

            NetLogo(&ADI);

			return TRUE;
        }
		//------------------------------------------------------------------------
        else if(wParam == ID_OPTIONS_INTEGERMATH)
        {
			bIntergerMath = !bIntergerMath;

			if(bIntergerMath)
			{
				CheckMenuItem(hMenu, ID_OPTIONS_INTEGERMATH, MF_BYCOMMAND|MF_CHECKED);
				EnableMenuItem(hMenu, ID_OPTIONS_SMARTROUNDING, MF_BYCOMMAND|MF_GRAYED);
			}
			else {
				CheckMenuItem(hMenu, ID_OPTIONS_INTEGERMATH, MF_BYCOMMAND|MF_UNCHECKED);
				EnableMenuItem(hMenu, ID_OPTIONS_SMARTROUNDING, MF_BYCOMMAND|MF_ENABLED);
			}			
			return TRUE;
        }
		//------------------------------------------------------------------------
        else if(wParam == ID_OPTIONS_SHOWWORK)
        {
			bShowWork = !bShowWork;

			if(bShowWork)
			{
				CheckMenuItem(hMenu, ID_OPTIONS_SHOWWORK, MF_BYCOMMAND|MF_CHECKED);
			}
			else {
				CheckMenuItem(hMenu, ID_OPTIONS_SHOWWORK, MF_BYCOMMAND|MF_UNCHECKED);
			}			
			return TRUE;
        }
		//------------------------------------------------------------------------
        else if(wParam == ID_OPTIONS_SMARTROUNDING)
        {
			bSmartRounding = !bSmartRounding;

			if(bSmartRounding)
			{
				CheckMenuItem(hMenu, ID_OPTIONS_SMARTROUNDING, MF_BYCOMMAND|MF_CHECKED);
			}
			else {
				CheckMenuItem(hMenu, ID_OPTIONS_SMARTROUNDING, MF_BYCOMMAND|MF_UNCHECKED);
			}
			return TRUE;
        }
		//------------------------------------------------------------------------
		else if(wParam == ID_FILE_SAVEVARIABLES)
        {
			char sInitialDirectory[MAX_PATH];
			GetModuleFilePath(sInitialDirectory, sizeof(sInitialDirectory));

			if(OSFileDialog(hWnd, sLastVariableFile, sizeof(sLastVariableFile), "ecv",
				"Save an expression calculator variable file.", "Variable-Files\0*.ecv\0All-Files\0*.*\0\0",
				sInitialDirectory, SFD, OFN_EXPLORER + OFN_LONGNAMES + OFN_PATHMUSTEXIST))
			{
				SaveVariables(sLastVariableFile);
			}

            return TRUE;
        }
		//------------------------------------------------------------------------
		else if(wParam == ID_FILE_LOADVARIABLES)
        {
			char sInitialDirectory[MAX_PATH];
			GetModuleFilePath(sInitialDirectory, sizeof(sInitialDirectory));

			if(OSFileDialog(hWnd, sLastVariableFile, sizeof(sLastVariableFile), "ecv",
				"Select an expression calculator variable file.", "Variable-Files\0*.ecv\0All-Files\0*.*\0\0",
				sInitialDirectory, OFD, OFN_EXPLORER + OFN_LONGNAMES + OFN_PATHMUSTEXIST))
			{
				LoadVariables(sLastVariableFile);
			}

            return TRUE;
        }
		//------------------------------------------------------------------------
		else if(wParam == ID_FILE_EXIT)
        {
            EndDialog(hWnd, 0);
            DestroyWindow(hWnd);

			hMainDialog = NULL;

            return TRUE;
        }
		//------------------------------------------------------------------------
		else if(wParam == ID_OPERATORS_MULTIPLICATION) {
			TextBoxWrite(hExpression, "*");
		}
		else if(wParam == ID_OPERATORS_DIVISION) {                 
			TextBoxWrite(hExpression, "/");                        
		}                                                          
		else if(wParam == ID_OPERATORS_ADDITION) {                 
			TextBoxWrite(hExpression, "+");                        
		}                                                          
		else if(wParam == ID_OPERATORS_SUBTRACTION) {              
			TextBoxWrite(hExpression, "-");                        
		}                                                          
		else if(wParam == ID_OPERATORS_MODULATION) {               
			TextBoxWrite(hExpression, "%");                        
		}                                                          
		else if(wParam == ID_OPERATORS_NOTEQUAL) {                 
			TextBoxWrite(hExpression, "<>");                       
		}                                                          
		else if(wParam == ID_OPERATORS_BITWISEOREQUAL) {           
			TextBoxWrite(hExpression, "|=");                       
		}                                                          
		else if(wParam == ID_OPERATORS_BITWISEANDEQUAL) {          
			TextBoxWrite(hExpression, "&=");                       
		}                                                          
		else if(wParam == ID_OPERATORS_BITWISEXOREQUAL) {          
			TextBoxWrite(hExpression, "^=");
		}
		else if(wParam == ID_OPERATORS_LESSOREQUAL) {
			TextBoxWrite(hExpression, "<=");
		}
		else if(wParam == ID_OPERATORS_GREATEROREQUAL) {
			TextBoxWrite(hExpression, ">=");
		}
		else if(wParam == ID_OPERATORS_EQUALS) {
			TextBoxWrite(hExpression, "=");
		}
		else if(wParam == ID_OPERATORS_GREATERTHAN) {
			TextBoxWrite(hExpression, ">");
		}
		else if(wParam == ID_OPERATORS_LESSTHAN) {
			TextBoxWrite(hExpression, "<");
		}
		else if(wParam == ID_OPERATORS_AND) {
			TextBoxWrite(hExpression, "&&");
		}
		else if(wParam == ID_OPERATORS_OR) {
			TextBoxWrite(hExpression, "||");
		}
		else if(wParam == ID_OPERATORS_BITWISEOR) {
			TextBoxWrite(hExpression, "|");
		}
		else if(wParam == ID_OPERATORS_BITWISEAND) {
			TextBoxWrite(hExpression, "&");
		}
		else if(wParam == ID_OPERATORS_EXCLUSIVEOR) {
			TextBoxWrite(hExpression, "^");
		}
		else if(wParam == ID_OPERATORS_NOT) {
			TextBoxWrite(hExpression, "!");
		}
		else if(wParam == ID_OPERATORS_OPENPARENTHESIS) {
			TextBoxWrite(hExpression, "(");
		}
		else if(wParam == ID_OPERATORS_CLOSEPARENTHESIS) {
			TextBoxWrite(hExpression, ")");
		}
		//------------------------------------------------------------------------

        return FALSE;
    }

    //--------------------------------------------------------------------------

	if(uMsg == WM_PAINT)
    {
        HDC ThisHDC;
        PAINTSTRUCT ThisPS;

        ThisHDC = BeginPaint(hWnd, &ThisPS);

        //- Any painting should be done here.

        EndPaint(hWnd, &ThisPS);
        return TRUE;
    }

    //--------------------------------------------------------------------------

    if(uMsg == WM_SYSCOMMAND) //- Received a system menu message.
    {
        if(LOWORD(wParam) == MAINDIALOG_MENU_ABOUT) //- About.
        {
            _AboutDialogInfo ADI;
    
            ADI.DisplayIcon  = LoadIcon(ghAppInstance, MAKEINTRESOURCE(IDI_MAIN));
            ADI.TitleCaption = gsTitleCaption;
            ADI.FileVersion  = gsFileVersion;
            ADI.BuildDate    = __DATE__;
            ADI.BuildTime    = __TIME__;
            ADI.CopyRight    = gsAppCopyRight;
            ADI.OwnerHandle  = hWnd;

            NetLogo(&ADI);

            return TRUE;
        }

        return FALSE;
    }

    //--------------------------------------------------------------------------

    if(uMsg == WM_CLOSE) //- Received close message.
    {
        EndDialog(hWnd, 0);
        DestroyWindow(hWnd);

		hMainDialog = NULL;

        return TRUE;
    }

    //--------------------------------------------------------------------------

    return FALSE;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
