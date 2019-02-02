#ifndef GAMECONSOLE_H
#define GAMECONSOLE_H
#include <string>
//#include <vector>

typedef void (stringOutFunc) (std::string);
class GameConsole
{
    public:
        GameConsole();
        virtual ~GameConsole();
        void Print(std::string str);
        void (*outFunction)(std::string);
        //void ConnectOutput(stringOutFunc f);
        //std::vector<stringOutFunc> outFunctions;
    protected:
    private:
};

#endif // GAMECONSOLE_H
