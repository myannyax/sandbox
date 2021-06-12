#include "runner.h"
#include "util/config.h"
#include "modules/exec.h"
#include "modules/wait.h"
#include "modules/UserNamespace.h"
#include "modules/MountNamespace.h"
#include "modules/memory_limit.h"

#include <string>
#include <cxxopts.hpp>

int main(int argc, char** argv) {
    cxxopts::Options options{"my_sandbox", "Sandbox"};

    options.add_options()
            ("h,help", "Print usage")
            ("c,config", "Config path", cxxopts::value<std::string>())
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

    Runner runner;

    ExecModule execModule;
    execModule.exe = parse_result.unmatched()[0];
    execModule.argv = parse_result.unmatched();

    WaitModule waitModule;
    UserNamespace userNamespaceModule;
    MountNamespace mountNamespaceModule;
    MemoryLimitsModule memoryLimitsModule{config};

    execModule.apply(runner);
    waitModule.apply(runner);
    //userNamespaceModule.apply(runner);
    //mountNamespaceModule.apply(runner);
    memoryLimitsModule.apply(runner);

    runner.run();
    return waitModule.exitCode;
}
