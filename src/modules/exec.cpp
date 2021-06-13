#include "exec.h"

#include <cstring>
#include <stdexcept>
#include <unistd.h>
#include <sys/capability.h>
#include <sys/prctl.h>

void ExecModule::apply(Runner& runner) {
    runner.addHook(Hook::Exec, [&](pid_t) {
        // drop capabilities
        cap_t cap = cap_get_proc();
        if (!cap) {
            throw std::runtime_error{"Failed to get capabilities"};
        }
        cap_clear(cap);
        cap_set_proc(cap);
        cap_free(cap);
        prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0);

        std::vector<char*> argvp;
        for (auto& s : argv) argvp.push_back(s.data());
        argvp.push_back(nullptr);

        if (inheritEnv) {
            execvp(exe.data(), argvp.data());
        } else {
            std::vector<char*> envp;
            for (auto& s : env) envp.push_back(s.data());
            envp.push_back(nullptr);
            execvpe(exe.data(), argvp.data(), envp.data());
        }

        throw std::runtime_error{
            std::string{"Failed to execute program: "} +
            std::strerror(errno)
        };
    });
}
