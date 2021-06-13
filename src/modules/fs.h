#pragma once

#include "ptrace.h"
#include "util/config.h"

#include <filesystem>
#include <vector>

enum class FilesystemAction {
    Allow,
    Deny,
    Readonly
};

struct FilesystemRule {
    std::filesystem::path path;
    FilesystemAction action;
};

struct FilesystemModule {
    const YamlConfig& config;
    PtraceModule& ptraceModule;
    std::vector<FilesystemRule> rules;

    void apply();

private:
    FilesystemAction getAction(const std::filesystem::path& path, pid_t pid) const;
    void handle(ProcessState& state, const std::filesystem::path& path, unsigned flags) const;
    void handleAt(ProcessState& state, const std::filesystem::path& path, unsigned flags, int fd) const;
};
