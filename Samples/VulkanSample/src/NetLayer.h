#pragma once

#include <Immortal.h>

namespace Immortal
{

class NetLayer : public Layer
{
public:
    NetLayer(const std::string &label) :
        Layer(label)
    {

    }

    virtual void OnAttach() override
    {
#ifdef _WIN32
            Socket socket{ "localhost", "8080" };

            if (!socket.Readable())
            {
                return;
            }          
#endif
    }

    virtual void OnDetach() override
    {

    }
};

}
