#include "settings.h"

std::string Settings::webSockAdr;
std::string Settings::label;
std::string Settings::streamId;
std::string Settings::streamSource;
Settings::Mode Settings::mode;
bool Settings::useUI;
int Settings::period;

Settings::Settings()
{

}

void Settings::PrintUsage() {
    printf("run as: WebRTCTest <web sock adr> <label> <stream id> <stream source: camera|file name> <mode: publisher|player> <use ui: true|false> <save period (0 means no save)>\n");
}

bool Settings::Parse(int argc, char* argv[]) {
    if(argc < 8) {
        return false;
    }

    int index = 1;
    webSockAdr = argv[index++];
    label = argv[index++];
    streamId = argv[index++];
    streamSource = argv[index++];
    std::string strMode = argv[index++];
    if(strMode.compare("publisher") == 0){
        mode = Settings::Mode::Publisher;
    }
    else if(strMode.compare("player") == 0){
        mode = Settings::Mode::Player;
    }
    else {
        printf("undefined mode:%s\n", strMode.c_str());
        return false;
    }
    std::string strUI = argv[index++];
    if(strUI.compare("true") == 0){
        useUI = true;
    }
    else if(strUI.compare("false") == 0){
        useUI = false;
    }
    else {
        printf("undefined ui usage selection:%s\n", strUI.c_str());
        return false;
    }

    std::string strPeriod = argv[index++];
    period = std::stoi (strPeriod, nullptr);

    Print();
    return true;
}

void Settings::Print() {
    printf("Settings:\n");
    printf("- webSockAdr:%s\n", webSockAdr.c_str());
    printf("- label:%s\n", label.c_str());
    printf("- stream id:%s\n", streamId.c_str());
    printf("- stream source:%s\n", streamSource.c_str());
    printf("- mode:%s\n", (mode == Mode::Publisher) ? "publisher" : "player");
    printf("- use ui:%s\n", useUI ? "true" : "false");
    printf("- period:%d\n", period);
}
