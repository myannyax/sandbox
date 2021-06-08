#include "exec.h"

#include <unistd.h>

void ExecModule::apply(Runner& runner) {
    runner.addHook(Hook::Exec, [&](pid_t) {
        std::vector<char*> argvp;
        for (auto& s : argv) argvp.push_back(s.data());
        argvp.push_back(nullptr);

        if (inheritEnv) {
            execv(exe.data(), argvp.data());
        } else {
            std::vector<char*> envp;
            for (auto& s : env) envp.push_back(s.data());
            envp.push_back(nullptr);
            execve(exe.data(), argvp.data(), envp.data());
        }
    });
}
