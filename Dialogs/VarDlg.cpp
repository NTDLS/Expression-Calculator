#ifndef _VarDlg_Cpp
#define _VarDlg_Cpp
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Windows.H>
#include <Stdio.H>
#include <Stdlib.H>
#include <CommCtrl.H>

#include "../Source/Entry.H"

#include "../Dialogs/MainDlg.H"
#include "../Dialogs/VarDlg.H"
#include "../Dialogs/NetLogoDlg.H"

#include "../../@Libraries/CMathParser/CMathParser.H"
#include "../../@Libraries/CStringBuilder/CStringBuilder.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace NSWFL::Windows;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//---------------------(Variable Declarations)

HWND hVarDialog = NULL;
char gsVarName[MAX_RESULT];
char gsVarExpr[MAX_RESULT];

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

INT_PTR CALLBACK VarDialog(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static HMENU hMenu = NULL;
	static HWND hVarName;
	static HWND hVarExpr;

    //--------------------------------------------------------------------------

    if(uMsg == WM_INITDIALOG) // Received A Initilize Dialog Message
    {
        hVarDialog = hWnd;

        HMENU hContextMenu = GetSystemMenu(hWnd, FALSE);
        AppendMenu(hContextMenu, MF_SEPARATOR, 0, 0);
        AppendMenu(hContextMenu, MF_STRING, VARDIALOG_MENU_ABOUT, "About");
		
		SendMessage(hWnd, (UINT)WM_SETTEXT, (WPARAM)0, (LPARAM)gsTitleCaption);
        SendMessage(hWnd, WM_SETICON, TRUE, (LPARAM) LoadIcon(ghAppInstance, MAKEINTRESOURCE(IDI_MAIN)));

        hVarName = GetDlgItem(hWnd, IDC_VARNAME);
        hVarExpr = GetDlgItem(hWnd, IDC_VAREXPR);

		Set_Text(hVarName, gsVarName);
		Set_Text(hVarExpr, gsVarExpr);

        CenterWindow(hWnd);

        return TRUE; // Return TRUE to set the keyboard focus, Otherwise return FALSE.
    }

    //--------------------------------------------------------------------------

    if(uMsg == WM_COMMAND)
    {
        if(wParam == IDC_OK)
        {
			CStringBuilder sName;
			sName.Initialize();
			sName.AppendFromTextBox(hVarName);
			if(sName.Length > 0)
			{
				CStringBuilder sExpr;
				sExpr.Initialize();
				sExpr.AppendFromTextBox(hVarExpr);
				if(sExpr.Length)
				{
					ReplaceVariable(sName.Buffer, sExpr.Buffer);
					EndDialog(hWnd, 0);
					DestroyWindow(hWnd);
					hVarDialog = NULL;
				}
				sExpr.Destroy();
			}
			sName.Destroy();
		}
        else if(wParam == IDC_CANCEL)
        {
			EndDialog(hWnd, 0);
			DestroyWindow(hWnd);

			hVarDialog = NULL;
		}

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
        if(LOWORD(wParam) == VARDIALOG_MENU_ABOUT) //- About.
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

		hVarDialog = NULL;

        return TRUE;
    }

    //--------------------------------------------------------------------------

    return FALSE;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
