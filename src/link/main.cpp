// Compiled with:
// ..\..\build\bin\clang -target pc-windows-i386 -o build/link-proxy.exe -lshlwapi main.cpp
// TODO: Create a `.vcxproj` instead.

#include <cstring> // for `_stricmp`
#include <cstdlib> // for `system`
#include <Windows.h> // for `GetCommandLineA`
#include <string> // for `string`
#include <shlwapi.h> // for `PathRemoveFileSpecA`

using namespace std;

int main(int argc, char *argv[]) {
    // If the command-line arguments contain `/winmd:only`,
    // we want to invoke Microsoft linker to create `.winmd`.
    // Otherwise, we want to invoke our patched version of
    // LLVM linker.

    bool contains = false;
    for (int i = 0; i != argc; ++i) {
        if (!_stricmp(argv[i], "/winmd:only")) {
            contains = true;
            break;
        }

        // If the argument is a response file, search it.
        if (argv[i][0] == '@' && !system(("find /C /I \"/winmd:only\" \""
            + string(argv[i] + 1) + "\" >NUL").c_str())) {
            contains = true;
            break;
        }
    }

    // Skip the executable name
    // (source: https://stackoverflow.com/a/36876057/9080566).
    char *s = GetCommandLineA();
    if (*s == '"') {
        ++s;
        while (*s)
            if (*s++ == '"')
                break;
    } else {
        while (*s && *s != ' ' && *s != '\t')
            ++s;
    }
    // Skip spaces preceding the first argument.
    while (*s == ' ' || *s == '\t')
        ++s;

    if (contains) {
        return system(("link " + string(s)).c_str());
    }

    // Get directory name of the executable.
    PathRemoveFileSpecA(argv[0]);

    return system((string(argv[0]) + "\\lld-link.exe " + s).c_str());
}
