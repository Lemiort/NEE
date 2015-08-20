#ifndef UIBASE_H
#define UIBASE_H
#include "Text_2D.h"

class UiBase
{
    public:
        UiBase();
        virtual ~UiBase();
        virtual void Init();

        bool visible;
    protected:
    private:
};

#endif // UIBASE_H
