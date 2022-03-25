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
        Async::Execute([]() {
            Socket socket{ "localhost", "8080" };

            if (!socket.Readable())
            {
                return;
            }
            std::vector<uint8_t> data = {
                'H', 'E', 'L', 'L', 'O', 0
            };
            socket.Send(data);
            socket.Receive([](auto buffer, auto size) {
                for (auto i = 0; i < size; i++)
                {
                    printf("%s\n", buffer.data());
                }
                });
            });
#endif
    }

    virtual void OnDetach() override
    {

    }
};

}
