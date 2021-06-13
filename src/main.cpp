#include "runner.h"
#include "util/config.h"
#include "modules/exec.h"
#include "modules/ptrace.h"
#include "modules/fs.h"
#include "modules/UserNamespace.h"
#include "modules/MountNamespace.h"
#include "modules/limit.h"
#include "modules/priority.h"
#include "util/log.h"

#include <string>
#include <cxxopts.hpp>
#include <iostream>
#include <syscall.h>
#include <signal.h>

int main(int argc, char** argv) {
    cxxopts::Options options{"my_sandbox", "Sandbox"};

    options.add_options()
            ("h,help", "Print usage")
            ("c,config", "Config path", cxxopts::value<std::string>())
            ("l,log", "Log path", cxxopts::value<std::string>()->default_value(""))
            ;

    cxxopts::ParseResult parse_result;
    try {
        parse_result = options.parse(argc, argv);

        if (parse_result.unmatched().empty()) {
            throw cxxopts::OptionParseException{"Expected at least one positional argument"};
        }
    } catch (cxxopts::OptionParseException& e) {
        std::cerr << e.what() << std::endl;
        std::cerr << options.help();
        return 1;
    }

    if (parse_result.count("help")) {
        std::cerr << options.help();
        return 0;
    }

    YamlConfig config;
    if (parse_result.count("config")) {
        config.init(parse_result["config"].as<std::string>());
    }
    MultiprocessLog::init(parse_result["log"].as<std::string>());

    Runner runner;

    ExecModule execModule;
    execModule.exe = parse_result.unmatched()[0];
    execModule.argv = parse_result.unmatched();

    PtraceModule ptraceModule;
    ptraceModule.onSyscall(SYS_kill, [&](ProcessState& state) {
        auto sig = state.syscall.args[1];
        if (sig == SIGTRAP) {
            state.syscall.result = -EPERM;
        }
    });

    UserNamespace userNamespaceModule{config};
    MountNamespace mountNamespaceModule{config};
    LimitsModule limitsModule{config, ptraceModule};
    PriorityModule priorityModule{config, ptraceModule};
    FilesystemModule filesystemModule{config, ptraceModule, {}};
    filesystemModule.apply();

    execModule.apply(runner);
    ptraceModule.apply(runner);

    if (config.config["user_namespace"].as<bool>()) {
        userNamespaceModule.apply(runner);
    }

    if (config.config["mount_namespace"].as<bool>()) {
        mountNamespaceModule.apply(runner);
    }

    limitsModule.apply(runner);
    priorityModule.apply(runner);

    runner.run();
    return ptraceModule.exitCode;
}
