#include "runner.h"
#include "modules/exec.h"
#include "modules/wait.h"

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

    execModule.apply(runner);
    waitModule.apply(runner);

    runner.run();
    return waitModule.exitCode;
}
