#include "impch.h"
#include "Component.h"
#include "Script/ScriptEngine.h"
#include "FileSystem/FileSystem.h"
#include "Object.h"

namespace Immortal
{

#define ASSET_SET_OBJECT() {                                                  \
        if (!vtable.__setId) {                                                \
            return;                                                           \
        }                                                                     \
        Anonymous params[] = {                                                \
            &id,                                                              \
            &scene,                                                           \
        };                                                                    \
        ScriptEngine::Invoke(vtable.__setId, object, params);                 \
    }

void ScriptComponent::Init(int id, Scene *scene)
{
#if HAVE_MONO
    auto namespaceName = std::string{ "Immortal" };
    className = FileSystem::ExtractFileName(path);

    object = ScriptEngine::CreateObject(namespaceName, className);
    vtable.__setId     = ScriptEngine::Search(object, "GameObject", "SetId_");
    vtable.__update    = ScriptEngine::Search(object, "GameObject", "Update");
    vtable.__onKeyDown = ScriptEngine::Search(object, "GameObject", "OnKeyDown");
#endif
}

void ScriptComponent::Update(int id, Scene *scene, float deltaTime)
{
#if HAVE_MONO
    ASSET_SET_OBJECT()
    Anonymous params[] = { &deltaTime };
    ScriptEngine::Invoke(vtable.__update, object, params);
#endif
}

void ScriptComponent::OnKeyDown(int id, Scene *scene, int keyCode)
{
#if HAVE_MONO
    ASSET_SET_OBJECT()
    Anonymous params[] = { &keyCode };
    ScriptEngine::Invoke(vtable.__onKeyDown, object, params);
#endif
}

VideoPlayerComponent::VideoPlayerComponent(Ref<Vision::VideoCodec> decoder, Ref<Vision::Interface::Demuxer> demuxer) :
    decoder{ decoder },
    demuxer{ demuxer },
    lastPicture{ new Vision::Picture{} },
    fifo{ new std::queue<Vision::Picture>{} },
    mutex{ new std::mutex{} },
    state{ new State{} }
{
    struct {
        Vision::VideoCodec *decoder;
        Vision::Interface::Demuxer *demuxer;
        Vision::Picture *lastPicture;
        std::queue<Vision::Picture> *fifo;
        bool *exited;
        std::mutex *mutex;
    } tpack{
        decoder,
        demuxer,
        lastPicture,
        fifo,
        &state->exited,
        mutex.get(),
    };

    VideoPlayerComponent *that = this;
    thread = new Thread{ [=]() {
        do
        {
            Vision::CodedFrame codedFrame;
            while (tpack.fifo->size() < 7 && !tpack.demuxer->Read(&codedFrame))
            {
                if (tpack.decoder->Decode(codedFrame) == CodecError::Succeed)
                {
                    Vision::Picture picture = tpack.decoder->GetPicture();
                    size_t frame = picture.pts - tpack.lastPicture->pts;

                    std::lock_guard lock{ *tpack.mutex };
                    if (!tpack.lastPicture->data)
                    {
                        *tpack.lastPicture = picture;
                        tpack.lastPicture->pts = 0;
                        tpack.fifo->push(*tpack.lastPicture);
                    }
                    for (size_t i = 0; i < frame - 1; i++)
                    {
                        tpack.lastPicture->pts++;
                        tpack.fifo->push(*tpack.lastPicture);
                    }
                    tpack.fifo->push(picture);
                    *tpack.lastPicture = picture;
                }
            }
        } while (!*tpack.exited);
    } };

    thread->Start();
}

VideoPlayerComponent::~VideoPlayerComponent()
{
    if (state)
    {
        state->exited = true;
    }
    thread.Release();
}

Vision::Picture VideoPlayerComponent::GetPicture()
{
    if (fifo->empty())
    {
        return Vision::Picture{};
    }

    Vision::Picture ret = fifo->front();

    return ret;
}

void VideoPlayerComponent::PopPicture()
{
    std::lock_guard lock{ *mutex };
    fifo->pop();
}

}
