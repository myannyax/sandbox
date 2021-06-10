#include "runner.h"
#include "modules/exec.h"
#include "modules/wait.h"
#include "modules/UserNamespace.h"
#include "modules/MountNamespace.h"

#include <vector>
#include <string>

int main(int argc, char** argv) {
    Runner runner;

    ExecModule execModule;
    execModule.exe = argv[1];
    for (int i = 1; i < argc; ++i) {
        execModule.argv.push_back(argv[i]);
    }

    WaitModule waitModule;
    UserNamespace userNamespaceModule;
    MountNamespace mountNamespaceModule;

    execModule.apply(runner);
    waitModule.apply(runner);
    //userNamespaceModule.apply(runner);
    //mountNamespaceModule.apply(runner);

    runner.run();
    return waitModule.exitCode;
}
