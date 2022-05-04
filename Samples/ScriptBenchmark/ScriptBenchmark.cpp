#include <Immortal.h>

using namespace Immortal;

int main(int argc, char **argv)
{
    argv[1] = { argv[0] };

    LOG::Init();
    {
        Ref<ScriptEngine> engine = new ScriptEngine{ "Immortal Engine", "Test.dll" };
        engine->Execute(argc + 1, argv);
    }

    LOG::Release();

    return 0;
}
