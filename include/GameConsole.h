#ifndef INCLUDE_GAMECONSOLE_H
#define INCLUDE_GAMECONSOLE_H
#include <string>

typedef void(stringOutFunc)(std::string);
class GameConsole {
public:
    GameConsole();
    virtual ~GameConsole();
    void Print(std::string str);
    void (*outFunction)(std::string);
    // void ConnectOutput(stringOutFunc f);
    // std::vector<stringOutFunc> outFunctions;
protected:
private:
};

#endif  // INCLUDE_GAMECONSOLE_H
