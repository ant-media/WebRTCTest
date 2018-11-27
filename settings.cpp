#include "settings.h"
#include "string.h"

std::string Settings::webSockAdr = "localhost";
std::string Settings::label = "nolabel";
std::string Settings::streamId = "myStream";
std::string Settings::streamSource = "camera";
Settings::Mode Settings::mode = Settings::Mode::Player;
bool Settings::useUI = true;
int Settings::period = 0;
bool Settings::verbose = false;
bool Settings::isSequre = false;

Settings::Settings()
{

}

void Settings::PrintUsage() {
    printf("WebRTC Test Tool for Ant Media Server v0.2\n\n");
    printf("Flag \t Name      \t Default   \t Description                 \n");
    printf("---- \t ----      \t -------   \t -----------                 \n");
    printf("s    \t Server Ip \t localhost \t server ip                   \n");
    printf("q    \t Sequrity  \t false     \t true(wss) or false(ws)      \n");
    printf("l    \t Label     \t nolabel   \t window lable                \n");
    printf("i    \t Stream Id \t myStream  \t id for stream               \n");
    printf("f    \t File Name \t camera    \t media file in same directory\n");
    printf("m    \t Mode      \t player    \t publisher or player         \n");
    printf("u    \t Show GUI  \t true      \t true or false               \n");
    printf("p    \t Period    \t 0         \t frame period to save as png \n");
    printf("v    \t Verbose   \t false     \t true or false               \n");
}

bool Settings::ParseLocal(char* flag, char* value) {
    if(flag[0] != '-' && strlen(flag)<2) {
        return false;
    }
    if(flag[1] == 's') {
        webSockAdr = value;
    }
    else if(flag[1] == 'q') {
        std::string seq = value;
        if(seq.compare("true") == 0){
            isSequre = true;
        }
        else if(seq.compare("false") == 0){
            isSequre = false;
        }
        else {
            printf("undefined sequrity selection:%s\n", seq.c_str());
            return false;
        }
    }
    else if(flag[1] == 'l') {
        label = value;
    }
    else if(flag[1] == 'i') {
        streamId = value;
    }
    else if(flag[1] == 'f') {
        streamSource = value;
    }
    else if(flag[1] == 'm') {
        std::string strMode = value;
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
    }
    else if(flag[1] == 'u') {
        std::string strUI = value;
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
    }
    else if(flag[1] == 'p') {
        std::string strPeriod = value;
        period = std::stoi (strPeriod, nullptr);
    }
    else if(flag[1] == 'v') {
        std::string verb = value;
        if(verb.compare("true") == 0){
            verbose = true;
        }
        else if(verb.compare("false") == 0){
            verbose = false;
        }
        else {
            printf("undefined verbosity usage selection:%s\n", verb.c_str());
            return false;
        }
    }
    else {
        return false;
    }
    return true;
}

bool Settings::Parse(int argc, char* argv[]) {
    if(argc == 2) {
        return false;
    }
    for(int i = 1; i < argc-1; i+=2) {
        if(!ParseLocal(argv[i], argv[i+1]))
        {
            return false;
        }
    }

    Print();
    return true;
}

void Settings::Print() {
    printf("Settings:\n");
    printf("- webSockAdr:%s\n", webSockAdr.c_str());
    printf("- is sequre:%s\n", isSequre ? "true" : "false");
    printf("- label:%s\n", label.c_str());
    printf("- stream id:%s\n", streamId.c_str());
    printf("- stream source:%s\n", streamSource.c_str());
    printf("- mode:%s\n", (mode == Mode::Publisher) ? "publisher" : "player");
    printf("- use ui:%s\n", useUI ? "true" : "false");
    printf("- period:%d\n", period);
    printf("- verbose:%s\n", verbose ? "true" : "false");
}
