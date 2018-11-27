#ifndef SETTINGS_H
#define SETTINGS_H

#include <string>

class Settings
{

public:
    enum Mode {
        Player,
        Publisher
    };

    Settings();
    static bool Parse(int argc, char* argv[]);
    static bool ParseLocal(char* flag, char* value);
    static void Print();
    static void PrintUsage();

    static std::string webSockAdr;
    static std::string label;
    static std::string streamId;
    static std::string streamSource;
    static Mode mode;
    static bool useUI;
    static int period;
    static bool verbose;
    static bool isSequre;
};

#endif // SETTINGS_H
