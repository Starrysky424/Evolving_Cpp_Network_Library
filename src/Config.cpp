#include "Config.h"

#include <fstream>
#include <string>
#include "nlohmann/json.hpp"

    using json = nlohmann::json;

void Config::init(int argc, char **argv)
{
    for (int i = 1; i < argc; i++)
    {
        if (std::string(argv[i]) == "--config" && i + 1 < argc)
        {
            config_path = argv[i + 1];
            break;
        }
    }

    loadFromFile(config_path);

    parseCommandLine(argc, argv);
}

void Config::loadFromFile(const std::string &path)
{
    std::ifstream file(path);

    if (!file.is_open())
        return;

    json j = json::parse(file);

#define X(field, json_key, cli_flag, default_val) \
    if (j.contains(json_key))                     \
        field = j[json_key];

    CONFIG_ITEMS(X)

#undef X
}

void Config::parseCommandLine(int argc, char **argv)
{
    for (int i = 1; i < argc; i++)
    {
        std::string arg = argv[i];

        if (arg == "--config" && i + 1 < argc)
        {
            config_path = argv[++i];
            continue;
        }

        if (arg == "--dump-config")
        {
            dump_config_ = true;
            continue;
        }

        if (arg == "--help")
        {
            // 后续添加
        }

#define X(field, json_key, cli_flag, default_val) \
    if (arg == cli_flag && i + 1 < argc)          \
    {                                             \
        field = std::stoi(argv[++i]);             \
        continue;                                 \
    }

        CONFIG_ITEMS(X)

#undef X
    }
}

void Config::print() const
{
#define X(field, json_key, cli_flag, default_val) \
    printf("%-20s %d\n", json_key, field);

    CONFIG_ITEMS(X)

#undef X
}

