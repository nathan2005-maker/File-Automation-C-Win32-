#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <stdint.h>
#include <ctype.h>
#include <conio.h>

#define MAIN_DIRECTORY "\\\\10.1.1.180\\publico\\Fiscal\\Scan\\"
#define BUFFER_SIZE 4096
#define MAX_FILENAME 260

typedef enum {
    MODE_AUTO,
    MODE_MANUAL_RULE
} OperationMode;

volatile OperationMode currentMode = MODE_AUTO;
uint8_t manualMap[26] = { 0 };

/* ================= UTIL ================= */

uint8_t getAutoFolder(const uint8_t* name) {
    uint8_t c = (uint8_t)toupper(name[0]);
    return (c >= 'A' && c <= 'Z') ? c : 'X';
}

void clearRules(void) {
    for (int i = 0; i < 26; i++)
        manualMap[i] = 0;
}

void addRule(void) {
    uint8_t from, to;

    printf("\nArquivo comeca com (A-Z): ");
    from = (uint8_t)toupper(_getch());
    printf("%c\n", from);

    printf("Enviar para pasta (A-Z): ");
    to = (uint8_t)toupper(_getch());
    printf("%c\n", to);

    if (from >= 'A' && from <= 'Z' && to >= 'A' && to <= 'Z') {
        manualMap[from - 'A'] = to;
        currentMode = MODE_MANUAL_RULE;
        printf("Regra criada: %c -> %c\n\n", from, to);
    }
    else {
        printf("Regra invalida\n\n");
    }
}

int waitForFile(const char* path, DWORD timeout) {
    DWORD start = GetTickCount();
    HANDLE h;

    while (1) {
        h = CreateFileA(path, GENERIC_READ | GENERIC_WRITE,
            0, NULL, OPEN_EXISTING, 0, NULL);

        if (h != INVALID_HANDLE_VALUE) {
            CloseHandle(h);
            return 1;
        }

        if (GetTickCount() - start > timeout)
            return 0;

        Sleep(100);
    }
}

void moveFile(const uint8_t* name, uint8_t folder) {
    char src[MAX_PATH], dstDir[MAX_PATH], dst[MAX_PATH];

    snprintf(dstDir, MAX_PATH, "%s%c", MAIN_DIRECTORY, folder);
    CreateDirectoryA(dstDir, NULL);

    snprintf(src, MAX_PATH, "%s%s", MAIN_DIRECTORY, name);
    snprintf(dst, MAX_PATH, "%s\\%s", dstDir, name);

    if (waitForFile(src, 5000)) {
        MoveFileA(src, dst)
            ? printf("Movido: %s -> %c\n", name, folder)
            : printf("Erro ao mover %s (%lu)\n", name, GetLastError());
    }
    else {
        printf("Arquivo %s ainda em uso\n", name);
    }
}

/* ================= MAIN ================= */

int main(void) {
    HANDLE hDir;
    OVERLAPPED ov = { 0 };
    HANDLE hEvent;
    uint8_t buffer[BUFFER_SIZE];
    DWORD bytes;
    FILE_NOTIFY_INFORMATION* fni;
    DWORD offset;

    hDir = CreateFileA(
        MAIN_DIRECTORY,
        FILE_LIST_DIRECTORY,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        NULL,
        OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
        NULL
    );

    if (hDir == INVALID_HANDLE_VALUE) {
        printf("Erro ao abrir diretorio (%lu)\n", GetLastError());
        return 1;
    }

    /* cria pastas A-Z */
    for (char c = 'A'; c <= 'Z'; c++) {
        char dir[MAX_PATH];
        snprintf(dir, MAX_PATH, "%s%c", MAIN_DIRECTORY, c);
        CreateDirectoryA(dir, NULL);
    }

    hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    ov.hEvent = hEvent;

    printf("Monitorando: %s\n", MAIN_DIRECTORY);
    printf("[R] Regra  [C] Limpar  [A] Auto  [Q] Sair\n\n");

    while (1) {

        /* ===== TECLADO ===== */
        if (_kbhit()) {
            uint8_t key = (uint8_t)toupper(_getch());

            if (key == 'R') addRule();
            else if (key == 'C') {
                clearRules();
                currentMode = MODE_AUTO;
                printf("Regras limpas\n");
            }
            else if (key == 'A') {
                currentMode = MODE_AUTO;
                printf("Modo automatico\n");
            }
            else if (key == 'Q') {
                break;
            }
        }

        /* ===== MONITOR ===== */
        ResetEvent(hEvent);

        ReadDirectoryChangesW(
            hDir,
            buffer,
            BUFFER_SIZE,
            FALSE,
            FILE_NOTIFY_CHANGE_FILE_NAME,
            NULL,
            &ov,
            NULL
        );

        DWORD wait = WaitForSingleObject(hEvent, 100);

        if (wait == WAIT_OBJECT_0) {
            GetOverlappedResult(hDir, &ov, &bytes, FALSE);

            offset = 0;
            do {
                fni = (FILE_NOTIFY_INFORMATION*)(buffer + offset);

                if (fni->Action == FILE_ACTION_ADDED) {
                    wchar_t wname[MAX_FILENAME];
                    uint8_t name[MAX_FILENAME];

                    DWORD len = fni->FileNameLength / sizeof(WCHAR);
                    if (len >= MAX_FILENAME) len = MAX_FILENAME - 1;

                    wcsncpy(wname, fni->FileName, len);
                    wname[len] = L'\0';
                    wcstombs((char*)name, wname, MAX_FILENAME);

                    uint8_t first = (uint8_t)toupper(name[0]);
                    uint8_t folder;

                    if (currentMode == MODE_MANUAL_RULE &&
                        first >= 'A' && first <= 'Z' &&
                        manualMap[first - 'A'] != 0)
                        folder = manualMap[first - 'A'];
                    else
                        folder = getAutoFolder(name);

                    moveFile(name, folder);
                }

                offset += fni->NextEntryOffset;
            } while (fni->NextEntryOffset != 0);
        }
    }

    CloseHandle(hEvent);
    CloseHandle(hDir);
    return 0;
}
