#include <windows.h>
#include <commdlg.h>
#include <shlobj.h> // For SHBrowseForFolder and related functions
#include <string>

#define ID_FILE_OPEN 1
#define ID_FILE_SAVE 2
#define ID_FILE_SAVE_AS 3
#define ID_SELECT_ALL 4
#define ID_PROJECT_NEW 5
#define ID_FILE_BUILD 6 // New identifier for Build

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    static HWND hEdit;
    static OPENFILENAME ofn;
    static char szFileName[MAX_PATH] = {0};
    static std::string currentFileName;
    static bool isModified = false;

    switch (uMsg) {
        case WM_CREATE: {
            // Create the edit control
            hEdit = CreateWindowEx(
                0, 
                "EDIT", 
                "", 
                WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL, 
                0, 0, 0, 0, 
                hwnd, 
                NULL, 
                ((LPCREATESTRUCT)lParam)->hInstance, 
                NULL
            );

            // Create the menu
            HMENU hMenu = CreateMenu();
            HMENU hFileMenu = CreateMenu();
            HMENU hProjectMenu = CreateMenu();

            AppendMenu(hFileMenu, MF_STRING, ID_FILE_OPEN, "Open");
            AppendMenu(hFileMenu, MF_STRING, ID_FILE_SAVE, "Save");
            AppendMenu(hFileMenu, MF_STRING, ID_FILE_SAVE_AS, "Save As");
            AppendMenu(hFileMenu, MF_STRING, ID_FILE_BUILD, "Build"); // Add Build menu item
            AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hFileMenu, "File");

            AppendMenu(hProjectMenu, MF_STRING, ID_PROJECT_NEW, "New");
            AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hProjectMenu, "Project");

            SetMenu(hwnd, hMenu);

            // Initialize OPENFILENAME structure
            ZeroMemory(&ofn, sizeof(ofn));
            ofn.lStructSize = sizeof(ofn);
            ofn.hwndOwner = hwnd;
            ofn.lpstrFile = szFileName;
            ofn.nMaxFile = sizeof(szFileName);
            ofn.lpstrFilter = "Text Files\0*.TXT\0All Files\0*.*\0";
            ofn.nFilterIndex = 1;
            ofn.lpstrFileTitle = NULL;
            ofn.nMaxFileTitle = 0;
            ofn.lpstrInitialDir = NULL;
            ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_OVERWRITEPROMPT;
            break;
        }

        case WM_COMMAND: {
            switch (LOWORD(wParam)) {
                case ID_FILE_OPEN: {
                    if (GetOpenFileName(&ofn)) {
                        HANDLE hFile = CreateFile(ofn.lpstrFile, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
                        if (hFile != INVALID_HANDLE_VALUE) {
                            DWORD dwSize = GetFileSize(hFile, NULL);
                            char* buffer = new char[dwSize + 1];
                            DWORD dwRead;
                            ReadFile(hFile, buffer, dwSize, &dwRead, NULL);
                            buffer[dwSize] = '\0';
                            SetWindowText(hEdit, buffer);
                            delete[] buffer;
                            CloseHandle(hFile);
                            currentFileName = ofn.lpstrFile;
                            isModified = false;
                            SetWindowText(hwnd, (currentFileName + " - Notepad Clone").c_str());
                        }
                    }
                    break;
                }

                case ID_FILE_SAVE: {
                    if (currentFileName.empty()) {
                        if (GetSaveFileName(&ofn)) {
                            HANDLE hFile = CreateFile(ofn.lpstrFile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
                            if (hFile != INVALID_HANDLE_VALUE) {
                                DWORD dwSize = GetWindowTextLength(hEdit);
                                char* buffer = new char[dwSize + 1];
                                GetWindowText(hEdit, buffer, dwSize + 1);
                                DWORD dwWritten;
                                WriteFile(hFile, buffer, dwSize, &dwWritten, NULL);
                                delete[] buffer;
                                CloseHandle(hFile);
                                currentFileName = ofn.lpstrFile;
                                isModified = false;
                                SetWindowText(hwnd, (currentFileName + " - Notepad Clone").c_str());
                            }
                        }
                    } else {
                        HANDLE hFile = CreateFile(currentFileName.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
                        if (hFile != INVALID_HANDLE_VALUE) {
                            DWORD dwSize = GetWindowTextLength(hEdit);
                            char* buffer = new char[dwSize + 1];
                            GetWindowText(hEdit, buffer, dwSize + 1);
                            DWORD dwWritten;
                            WriteFile(hFile, buffer, dwSize, &dwWritten, NULL);
                            delete[] buffer;
                            CloseHandle(hFile);
                            isModified = false;
                            SetWindowText(hwnd, (currentFileName + " - Notepad Clone").c_str());
                        }
                    }
                    break;
                }

                case ID_FILE_SAVE_AS: {
                    if (GetSaveFileName(&ofn)) {
                        HANDLE hFile = CreateFile(ofn.lpstrFile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
                        if (hFile != INVALID_HANDLE_VALUE) {
                            DWORD dwSize = GetWindowTextLength(hEdit);
                            char* buffer = new char[dwSize + 1];
                            GetWindowText(hEdit, buffer, dwSize + 1);
                            DWORD dwWritten;
                            WriteFile(hFile, buffer, dwSize, &dwWritten, NULL);
                            delete[] buffer;
                            CloseHandle(hFile);
                            currentFileName = ofn.lpstrFile;
                            isModified = false;
                            SetWindowText(hwnd, (currentFileName + " - Notepad Clone").c_str());
                        }
                    }
                    break;
                }

                case ID_SELECT_ALL: {
                    SendMessage(hEdit, EM_SETSEL, 0, -1);
                    break;
                }

                case ID_PROJECT_NEW: {
                    BROWSEINFO bi = {0};
                    bi.ulFlags = BIF_USENEWUI | BIF_NEWDIALOGSTYLE;
                    LPITEMIDLIST pidl = SHBrowseForFolder(&bi);
                    if (pidl != NULL) {
                        char path[MAX_PATH];
                        if (SHGetPathFromIDList(pidl, path)) {
                            std::string projectPath = std::string(path) + "\\Debug";
                            if (CreateDirectory(projectPath.c_str(), NULL) || GetLastError() == ERROR_ALREADY_EXISTS) {
                                std::string mainFilePath = projectPath + "\\main.cpp";
                                HANDLE hFile = CreateFile(mainFilePath.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
                                if (hFile != INVALID_HANDLE_VALUE) {
                                    const char* initialContent = "// main.cpp - Your project start\n";
                                    DWORD dwWritten;
                                    WriteFile(hFile, initialContent, strlen(initialContent), &dwWritten, NULL);
                                    CloseHandle(hFile);
                                } else {
                                    MessageBox(hwnd, "Failed to create main.cpp file.", "Error", MB_OK | MB_ICONERROR);
                                }
                            } else {
                                MessageBox(hwnd, "Failed to create Debug directory.", "Error", MB_OK | MB_ICONERROR);
                            }
                        }
                        CoTaskMemFree(pidl);
                    }
                    break;
                }

                case ID_FILE_BUILD: {
                    const char* batchFile = "Build64.bat";
                    STARTUPINFO si = { sizeof(si) };
                    PROCESS_INFORMATION pi;
                    
                    if (CreateProcess(NULL, (LPSTR)batchFile, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
                        WaitForSingleObject(pi.hProcess, INFINITE);
                        CloseHandle(pi.hProcess);
                        CloseHandle(pi.hThread);
                    } else {
                        MessageBox(hwnd, "Failed to execute build script.", "Error", MB_OK | MB_ICONERROR);
                    }
                    break;
                }
            }
            break;
        }

        case WM_SYSKEYDOWN:
        case WM_KEYDOWN: {
            if (GetKeyState(VK_CONTROL) < 0) {
                if (wParam == 'A') {
                    SendMessage(hEdit, EM_SETSEL, 0, -1);
                } else if (wParam == 'S') {
                    if (isModified || currentFileName.empty()) {
                        SendMessage(hwnd, WM_COMMAND, ID_FILE_SAVE, 0);
                    }
                } else if (GetKeyState(VK_SHIFT) < 0 && wParam == 'A') {
                    SendMessage(hwnd, WM_COMMAND, ID_FILE_SAVE_AS, 0);
                }
            }
            break;
        }

        case WM_SIZE: {
            // Resize the edit control to fit the client area
            MoveWindow(hEdit, 0, 0, LOWORD(lParam), HIWORD(lParam), TRUE);
            break;
        }

        case WM_SETFOCUS: {
            if (isModified) {
                std::string title = currentFileName.empty() ? "Untitled - Notepad Clone*" : currentFileName + " - Notepad Clone*";
                SetWindowText(hwnd, title.c_str());
            }
            break;
        }

        case WM_DESTROY: {
            PostQuitMessage(0);
            break;
        }

        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    CoInitialize(NULL);

    const char CLASS_NAME[] = "NotepadClone";

    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
        0, 
        CLASS_NAME, 
        "Untitled - Notepad Clone", 
        WS_OVERLAPPEDWINDOW, 
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 
        NULL, 
        NULL, 
        hInstance, 
        NULL
    );

    if (hwnd == NULL) {
        CoUninitialize();
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    CoUninitialize();
    return 0;
}
