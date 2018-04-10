#ifndef _ENTRY_CPP
#define _ENTRY_CPP
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Windows.H>
#include <Stdio.H>
#include <Stdlib.H>
#include <math.H>
#include <float.H>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Entry.H"
#include "Init.H"

#include "../../@Libraries/NSWFL/NSWFL.h"

#include "../Dialogs/MainDlg.H"

using namespace NSWFL::Math;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

HINSTANCE ghAppInstance = NULL;

char gsAppCopyRight[64];
char gsFileVersion[10];
char gsTitleCaption[64];
char gsRegistryKey[255];

CMemPool gMem; //The global memory object.

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef _DEBUG
int main(int argc, char *argv[])
#else
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevious, LPSTR CmdLine, int CmdShow)
#endif
{
    #ifdef _DEBUG
	ghAppInstance = GetModuleHandle(NULL);
    #else
	ghAppInstance = hInstance;
    #endif

    int iResult = 0;

	if(!InitializeApp())
    {
        return 1;
    }

    DialogBox(ghAppInstance, MAKEINTRESOURCE(IDD_MAIN), NULL, MainDialog);

    if(!UninitializeApp())
    {
        return 1;
    }

	return iResult;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif

