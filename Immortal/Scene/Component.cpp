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
                auto error = tpack.decoder->Decode(codedFrame);
                if (error == CodecError::Succeed)
                {
                    Vision::Picture picture = tpack.decoder->GetPicture();
                    int frame = picture.pts - tpack.lastPicture->pts;

                    std::lock_guard lock{ *tpack.mutex };
                    if (!tpack.lastPicture->Available())
                    {
                        *tpack.lastPicture = picture;
                        tpack.lastPicture->pts = 0;
                    }
                    for (int i = 0; i < frame - 1; i++)
                    {
                        tpack.lastPicture->pts++;
                        tpack.fifo->push(*tpack.lastPicture);
                    }
                    tpack.fifo->push(picture);
                    *tpack.lastPicture = picture;
                }
                else if (error != CodecError::Again)
                {
                    continue;
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

void SpriteRendererComponent::UpdateSprite(const Vision::Picture &picture)
{
    struct {
        Vision::ColorSpace colorSpace;
        int _10Bits;
    } properties {
        Vision::ColorSpace::YUV,
        0
    };

    Texture::Description desc{
        Format::RGBA8,
        Wrap::Clamp,
        Filter::Bilinear,
        false
    };

    auto width = picture.desc.width;
    auto height = picture.desc.height;
    if (picture.desc.width != Sprite->Width() || picture.desc.height != Sprite->Height())
    {
        Sprite = Render::Create<Image>(picture.desc.width, picture.desc.height, nullptr, desc);
        Result = Render::Create<Image>(picture.desc.width, picture.desc.height, nullptr, desc);
    }

    if (picture.desc.format == Format::RGBA8)
    {
        Sprite->Update(picture[0]);
        return;
    }

    if (picture.desc.format == Format::YUV420P10 || picture.desc.format == Format::YUV422P10)
    {
        desc.format = Format::R16;
        properties._10Bits = 1;
    }
    else
    {
        desc.format = Format::R8;
    }

    if (!pNext)
    {
        pNext = new Extension;

        int widthFactor = 2;
        int heightFactor = 2;
        if (picture.desc.format == Format::YUV422P10 || picture.desc.format == Format::YUV422P)
        {
            heightFactor = 1;
        }
        auto width = picture.desc.width / widthFactor;
        auto height = picture.desc.height / heightFactor;
        pNext->input[0] = Render::Create<Image>(picture.desc.width, picture.desc.height, nullptr, desc);
        pNext->input[1] = Render::Create<Image>(width, height, nullptr, desc);
        pNext->input[2] = Render::Create<Image>(width, height, nullptr, desc);
    }

    pNext->input[0]->Update(picture[0], picture.shared->linesize[0] / desc.format.BytesPerPixel());
    pNext->input[1]->Update(picture[1], picture.shared->linesize[1] / desc.format.BytesPerPixel());
    pNext->input[2]->Update(picture[2], picture.shared->linesize[2] / desc.format.BytesPerPixel());

    properties.colorSpace = picture.GetProperty<Vision::ColorSpace>();
    Pipelines::ColorSpace->AllocateDescriptorSet((uint64_t)this);
    Pipelines::ColorSpace->Bind(pNext->input[0], 0);
    Pipelines::ColorSpace->Bind(pNext->input[1], 1);
    Pipelines::ColorSpace->Bind(pNext->input[2], 2);
    Pipelines::ColorSpace->Bind(Sprite, 3);
    Pipelines::ColorSpace->PushConstant(sizeof(properties), &properties);
    Pipelines::ColorSpace->Dispatch(SLALIGN(width, 1), SLALIGN(height, 1), 1);

    Sprite->Blit();
}

}
