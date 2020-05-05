#ifndef INCLUDE_GAMECONSOLE_H_
#define INCLUDE_GAMECONSOLE_H_
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

#endif  // INCLUDE_GAMECONSOLE_H_
