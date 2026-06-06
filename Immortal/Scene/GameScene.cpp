#include "GameScene.h"

#include "Framework/Timer.h"

#include "Graphics/AsyncCompute.h"
#include "Render/Graphics.h"
#include "Shared/Log.h"
#include "Render/Render2D.h"

#include "Object.h"
#include "Component.h"
#include "GameObject.h"
#include "String/LanguageSettings.h"
#include "Helper/Platform.h"
#include "ImGui/Utils.h"
#include "Render/FrameGraphDebugUi.h"
#include "Math/Math.h"
#include "Widget/Widget.h"
#include <algorithm>
#include <cmath>
#include <cfloat>
#include <limits>
#include <string>
#include <utility>

namespace Immortal
{

struct Transform
{
    Matrix4 ViewProjection;
    Matrix4 SkyboxPorjection;
    Matrix4 SceneRotation;
    Matrix4 Model;
};

struct Shading
{
    struct {
        Vector4 direction;
        Vector4 radiance;
    } lights[4];
    Vector3 CameraPosition;
    float Exposure;
    float Gamma;
};

struct Model
{
    Matrix4 Transform;
    Vector3 Color;
    float   Roughness;
    float   Metallic;
    int     ObjectID;
};

GameScene::GameScene(const String &name, bool isEditorScene_) :
    Scene{ name, isEditorScene_ },
    frameGraph{},
    editorCamera{ 90.0f, 1, 1, 0.1f, 1000.0f }
{

}

GameScene::~GameScene()
{

}

void GameScene::InitRenderTask()
{
	auto *act = Graphics::GetAsyncComputeThread();

	if (!skyboxTask)
	{
		skyboxTask = new SkyboxTask;
		skyboxTask->SetFilePath("skybox.hdr");
		skyboxTask->Build(act);
		iblTask = new IBLTask;
		iblTask->Build(act);
	}

	if (!meshletTask)
	{
		meshletTask = new MeshletTask;
		meshletTask->Build(act);
	}
	if (!meshletShadowTask)
	{
		meshletShadowTask = new MeshletShadowTask;
		meshletShadowTask->Build(act);
	}
	if (!deferredTask)
	{
		deferredTask = new DeferredTask;
		deferredTask->Build(act);
	}

	if (!postProcessTask)
	{
		postProcessTask = new PostProcessTask;
		postProcessTask->Build(act);
	}

	if (!depthToRgbTasks[0])
	{
		for (auto &t : depthToRgbTasks)
		{
			t = new DepthToRgbTask;
			t->Build(act);
		}
	}
}

void GameScene::RebuildFrameGraph()
{
	bool isDeferred = IsDeferred();

	InitRenderTask();

	iblTask->LinkSkybox(skyboxTask.Get());
	meshletTask->SetIBLTask(iblTask.Get());

	if (isDeferred)
	{
		deferredTask->SetResolveLightingMode((uint32_t)lightingModel);
		deferredTask->SetIBLTask(iblTask.Get());
		deferredTask->SetGBufferRenderTarget(gbufferRT);
		meshletTask->SetRenderPath(MeshletRenderPath::Deferred);
	}
	else
	{
		meshletTask->SetRenderPath(MeshletRenderPath::Forward);
	}

	frameGraph->ClearPasses();

	if (userShadowEnabled)
	{
		const bool csm = shadowMode == SceneShadowMode::CSM2x2;
		auto addShadowPass = [&](const Ref<RenderTask> &task, const std::string &passName, const std::string &rtName, uint32_t cascadeIdx, bool firstShadow)
		{
			auto &p = frameGraph->AddPass(passName);
			p.task  = task;
			p.phase = RenderPassPhase::DrawMesh;

			auto t = frameGraph->QueryRenderTarget(rtName);
			p.renderTarget = t->renderTarget;
			p.isShadowDepthPass   = true;
			p.filterDrawMeshByTag = false;
			p.shadowCascadeIndex = cascadeIdx;
			if (firstShadow)
			{
				p.clearValues = t->clearValues;
			}
			p.useDepthBias      = true;
			p.depthBiasConstant = 1.25f;
			p.depthBiasSlope    = 1.75f;
		};


		{
			size_t size = csm ? 4 : 1;
			for (uint32_t ci = 0; ci < size; ++ci)
			{
				const std::string rt = std::string("ShadowRT") + std::to_string(ci);
				addShadowPass(meshletShadowTask, std::string("ShadowDepth_") + std::to_string(ci), rt, ci, true);
			}
		}
	}

	if (isDeferred)
	{
		{
			auto &p = frameGraph->AddPass("GBuffer");
			p.task            = meshletTask;
			p.phase           = RenderPassPhase::DrawMesh;
			p.isGBufferPass   = true;
			p.lightingMode    = (uint32_t) MeshletPipelineSlot::GBuffer;

			auto t = frameGraph->QueryRenderTarget("GBufferRT");
			p.renderTarget = t->renderTarget;
			p.clearValues  = t->clearValues;
		}
		{
			auto &p = frameGraph->AddPass("GBufferSkeletal");
			p.task           = meshletTask;
			p.phase          = RenderPassPhase::DrawMesh;
			p.isGBufferPass  = true;
			p.lightingMode   = (uint32_t) MeshletPipelineSlot::GBuffer;
		}
	}

	{
		auto &p = frameGraph->AddPass("SkyboxCubemap");
		p.task  = skyboxTask;
		p.phase = RenderPassPhase::Execute;
		p.endRenderPassBeforeExecute = true;
	}

	if (iblTask)
	{
		auto &p = frameGraph->AddPass("IBL");
		p.task  = iblTask;
		p.phase = RenderPassPhase::Execute;
		p.endRenderPassBeforeExecute = true;
	}

	{
		auto t = frameGraph->QueryRenderTarget("SceneHDR");
		auto &p = frameGraph->AddPass("SkyboxDraw");
		p.task            = skyboxTask;
		p.phase           = RenderPassPhase::Composite;
		p.renderTarget    = t->renderTarget;
		p.clearValues     = t->clearValues;
	}

	if (isDeferred)
	{
		auto t = frameGraph->QueryRenderTarget("SceneHDR");
		auto &p = frameGraph->AddPass("DeferredResolve");
		p.task         = deferredTask;
		p.phase        = RenderPassPhase::Composite;
		p.renderTarget = t->renderTarget;
	}
	else
	{
		{
			auto &p = frameGraph->AddPass("ForwardDraw");
			p.task            = meshletTask;
			p.phase           = RenderPassPhase::DrawMesh;
			p.isGBufferPass   = false;
			p.lightingMode    = (uint32_t)lightingModel;
		}
	}

	if (postProcessTask)
	{
		auto t = frameGraph->QueryRenderTarget("SceneOutput");
		auto &p = frameGraph->AddPass("PostBloomTonemap");
		p.task         = postProcessTask;
		p.phase        = RenderPassPhase::Composite;
		p.renderTarget = t->renderTarget;
		p.clearValues  = t->clearValues;
	}

	frameGraph->Build();
}

void GameScene::OnGuiRender()
{
    ImGui::Begin(Translator::Translate("Scene Editor").c_str());

	if (ImGui::CollapsingHeader(Translator::Translate("Rendering").c_str(), ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::PushID("Rendering");
		bool changed = false;

		const char *modeNames[] = { "Forward", "Deferred" };
		int mode = (int)renderMode;
		if (ImGui::Combo(Translator::Translate("Render Mode").c_str(), &mode, modeNames, 2))
		{
			renderMode = (SceneRenderMode)mode;
			changed = true;
		}

		const char *lightNames[] = { "Unlit", "Phong", "PBR", "NPR" };
		int lm = (int)lightingModel;
		if (ImGui::Combo(Translator::Translate("Lighting").c_str(), &lm, lightNames, 4))
		{
			lightingModel = (SceneLightingModel)lm;
			changed = true;
		}

		if (ImGui::Checkbox(Translator::Translate("Shadows").c_str(), &userShadowEnabled))
		{
			changed = true;
		}

		const char *shadowModeNames[] = { "Shadow Mapping", "CSM (2x2)" };
		int sm = (int)shadowMode;
		if (ImGui::Combo(Translator::Translate("Shadow Mode").c_str(), &sm, shadowModeNames, 2))
		{
			shadowMode = (SceneShadowMode)sm;
			changed = true;
		}
		if (ImGui::Checkbox(Translator::Translate("Shadow PCF (forward)").c_str(), &shadowForwardPcfEnabled))
		{
			changed = true;
		}

		if (changed)
		{
			SetViewportSize(viewportSize);
		}
		ImGui::PopID();
	}

    if (isEditorScene && ImGui::CollapsingHeader(Translator::Translate("Scene Grid").c_str(), ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::PushID("SceneGrid");
        bool show = showEditorGrid;
        if (ImGui::Checkbox(Translator::Translate("Show Grid").c_str(), &show))
        {
            showEditorGrid = show;
        }
        ImGui::PopID();
    }

    ImGui::DragFloat(Translator::Translate("Exposure").c_str(), &settings.exposure, 0.01f, 0, 50.0f);
    ImGui::DragFloat(Translator::Translate("Gamma").c_str(), &settings.gamma, 0.01f, 0, 50.0f);

    static int item = 5;
    uint32_t resolutions[] = { 64, 128, 256, 512, 1024, 2048, 4096 };

    if (ImGui::Combo(Translator::Translate("Resolution").c_str(), &item, "64\000128\000256\000512\0001024\0002048\0004096\000"))
    {
        settings.environmentResolution = resolutions[item];
    }

    if (skyboxTask && ImGui::CollapsingHeader(Translator::Translate("Skybox").c_str(), ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::PushID("Skybox");
        if (ImGui::Button(Translator::Translate("Load HDR").c_str()))
        {
            auto path = FileDialogs::OpenFile(FileFilter::Image);
            if (path.has_value())
            {
                skyboxTask->SetFilePath(path.value());
                if (iblTask)
                {
                    iblTask->MarkDirty();
                }
            }
        }
        ImGui::PopID();
    }

    if (iblTask && ImGui::CollapsingHeader(Translator::Translate("IBL").c_str(), ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::PushID("IBL");

        static const uint32_t faceSizes[] = { 16, 32, 64, 128, 256 };
        static const char *faceSizeLabels[] = { "16", "32", "64", "128", "256" };
        uint32_t currentFaceSize = iblTask->GetIrradianceFaceSize();
        int faceSizeIdx = 1;
        for (int i = 0; i < (int)SL_ARRAY_LENGTH(faceSizes); i++)
        {
            if (faceSizes[i] == currentFaceSize)
            {
                faceSizeIdx = i;
                break;
            }
        }
        if (ImGui::Combo(Translator::Translate("Irradiance Size").c_str(), &faceSizeIdx, faceSizeLabels, (int)SL_ARRAY_LENGTH(faceSizeLabels)))
        {
            iblTask->SetIrradianceFaceSize(faceSizes[faceSizeIdx]);
        }

        int sampleCount = (int)iblTask->GetIrradianceSampleCount();
        if (ImGui::SliderInt(Translator::Translate("Sample Count").c_str(), &sampleCount, 8, 1024))
        {
            iblTask->SetIrradianceSampleCount((uint32_t)sampleCount);
        }

        ImGui::PopID();
    }

    if (atmosphereTask && ImGui::CollapsingHeader(Translator::Translate("Atmosphere").c_str(), ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::PushID("Atmosphere");

        bool enabled = atmosphereTask->GetEnabled();
        if (ImGui::Checkbox(Translator::Translate("Enabled").c_str(), &enabled))
        {
            atmosphereTask->SetEnabled(enabled);
        }

        float sunIntensity = atmosphereTask->GetSunIntensity();
        if (ImGui::DragFloat(Translator::Translate("Sun Intensity").c_str(), &sunIntensity, 0.1f, 0.0f, 200.0f))
        {
            atmosphereTask->SetSunIntensity(sunIntensity);
        }

        ImGui::PopID();
    }

    if (deferredTask && lightingModel == SceneLightingModel::NPR && ImGui::CollapsingHeader(Translator::Translate("NPR / Toon Shading").c_str(), ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::PushID("NPR");
        auto &npr = deferredTask->GetNPRParams();

        ImGui::ColorEdit3(Translator::Translate("Shadow Color").c_str(), &npr.shadowColor.x);
        ImGui::SliderFloat(Translator::Translate("Shadow Threshold").c_str(), &npr.shadowThreshold, -0.5f, 1.0f);
        ImGui::SliderFloat(Translator::Translate("Shadow Softness").c_str(), &npr.shadowSoftness, 0.0f, 0.3f);
        ImGui::SliderFloat(Translator::Translate("Lit Intensity").c_str(), &npr.litIntensity, 0.1f, 3.0f);
        ImGui::SliderFloat(Translator::Translate("Rim Power").c_str(), &npr.rimPower, 1.0f, 12.0f);
        ImGui::SliderFloat(Translator::Translate("Rim Intensity").c_str(), &npr.rimIntensity, 0.0f, 2.0f);

        if (ImGui::TreeNode(Translator::Translate("Edge Detection").c_str()))
        {
            ImGui::SliderFloat(Translator::Translate("Depth Threshold").c_str(), &npr.edgeDepthThreshold, 0.0001f, 0.05f, "%.4f");
            ImGui::SliderFloat(Translator::Translate("Normal Threshold").c_str(), &npr.edgeNormalThreshold, 0.05f, 2.0f);
            ImGui::SliderFloat(Translator::Translate("Edge Intensity").c_str(), &npr.edgeIntensity, 0.0f, 2.0f);
            ImGui::TreePop();
        }

        ImGui::PopID();
    }

    if (outlineTask && ImGui::CollapsingHeader(Translator::Translate("Selection Outline").c_str(), ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::PushID("SelectionOutline");

        Vector3 color = outlineTask->GetOutlineColor();
        if (ImGui::ColorEdit3(Translator::Translate("Outline Color").c_str(), &color.x))
        {
            outlineTask->SetOutlineColor(color);
        }

        float thickness = outlineTask->GetThickness();
        if (ImGui::SliderFloat(Translator::Translate("Outline Thickness").c_str(), &thickness, 1.0f, 8.0f))
        {
            outlineTask->SetThickness(thickness);
        }

        ImGui::PopID();
    }

	if (isEditorScene && shadowMapRT && ImGui::CollapsingHeader(Translator::Translate("Shadow debug").c_str(), ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::PushID("ShadowMapDepthVis");
		ImGui::Checkbox(Translator::Translate("Show shadow map depth (full screen)").c_str(), &showShadowMapDepthDebug);
		ImGui::PopID();
	}

	if (frameGraph && ImGui::CollapsingHeader(Translator::Translate("Frame graph (debug)").c_str()))
	{
		ImGui::PushID("FrameGraphDebug");
		frameGraph->OnFrameGraphDebugGui();
		if (shadowMapRT)
		{
			if (ImGui::TreeNodeEx(Translator::Translate("Shadow map (scene pass)").c_str(), ImGuiTreeNodeFlags_DefaultOpen))
			{
				FrameGraphDebugTextureThumbnail(shadowMapRT->GetDepthAttachment(), "Directional shadow depth (raw)");
				ImGui::DragFloat(Translator::Translate("Depth vis min (raw)").c_str(), &shadowMapLinearizeZNear, 0.002f, 0.0f, 0.999f, "%.4f");
				ImGui::DragFloat(Translator::Translate("Depth vis max (raw)").c_str(), &shadowMapLinearizeZFar, 0.002f, 0.001f, 1.0f, "%.4f");
				if (shadowMapLinearizeZNear >= shadowMapLinearizeZFar)
				{
					shadowMapLinearizeZFar = std::min(1.0f, shadowMapLinearizeZNear + 0.002f);
				}
				if (depthToRgbTasks[0])
				{
					for (size_t i = 0; i < 4; i++)
					{
						auto &t = depthToRgbTasks[i];
						FrameGraphDebugTextureThumbnail(t->GetOutputTexture(), "Shadow depth (depth2rgb compute)");
					}
				}
				ImGui::TreePop();
			}
		}
		ImGui::PopID();
	}

    ImGui::End();
}

void GameScene::OnRenderRuntime()
{
    {
        registry.view<ScriptComponent>().each([=, this](auto object, ScriptComponent &script) {
                script.Update((int)object, this, Time::DeltaTime);
            });
    }

    SceneCamera *sceneCamera = nullptr;
    {
        auto view = registry.view<TransformComponent, CameraComponent>();
        for (auto &o : view)
        {
            auto [transform, camera] = view.get<TransformComponent, CameraComponent>(o);
            if (camera.Primary)
            {
                sceneCamera = &camera.Camera;
                sceneCamera->SetTransform(transform);
                break;
            }
        }
    }

    if (!sceneCamera)
    {
        primaryCamera = dynamic_cast<SceneCamera*>(&observerCamera);
    }
    else
    {
        primaryCamera = sceneCamera;
    }

    primaryCamera->SetViewportSize(viewportSize);

    if (!sceneCamera)
    {
        observerCamera.OnUpdate(Time::DeltaTime);
    }
    OnRender(*primaryCamera);
}

void GameScene::OnRenderEditor()
{
	if (!isEditorScene)
	{
		return;
	}
	Camera *host = GetEditorPrimaryCamera();
	if (!host)
	{
		return;
	}
	OnRender(*host);
}

void GameScene::SetCameraType(SceneCameraType type)
{
	editorHostCameraType = type;
}

Camera *GameScene::GetEditorPrimaryCamera()
{
	if (!isEditorScene)
	{
		return nullptr;
	}
	return editorHostCameraType == SceneCameraType::EditorOrthographic
	    ? static_cast<Camera *>(&editorOrthographicCamera)
	    : static_cast<Camera *>(&editorCamera);
}

const Camera *GameScene::GetEditorPrimaryCamera() const
{
	if (!isEditorScene)
	{
		return nullptr;
	}
	return editorHostCameraType == SceneCameraType::EditorOrthographic
	    ? static_cast<const Camera *>(&editorOrthographicCamera)
	    : static_cast<const Camera *>(&editorCamera);
}

void GameScene::ComputeSceneWorldAABB(Vector3 &outMin, Vector3 &outMax) const
{
	constexpr float kDefaultExtent = 32.0f;
	glm::vec3 mn( std::numeric_limits<float>::max());
	glm::vec3 mx(-std::numeric_limits<float>::max());

	auto view = registry.view<TransformComponent, MeshComponent>();
	bool anyMesh = false;
	for (auto e : view)
	{
		auto &mc = view.get<MeshComponent>(e);
		if (!mc.Mesh)
		{
			continue;
		}
		Vector3 localMin, localMax;
		mc.Mesh->GetAABB(localMin, localMax);
		if (localMin.x >= localMax.x || localMin.y >= localMax.y || localMin.z >= localMax.z)
		{
			continue;
		}
		auto &tf = view.get<TransformComponent>(e);
		const glm::mat4 world = tf.Transform();
		const glm::vec3 corners[8] = {
			glm::vec3(localMin.x, localMin.y, localMin.z),
			glm::vec3(localMax.x, localMin.y, localMin.z),
			glm::vec3(localMin.x, localMax.y, localMin.z),
			glm::vec3(localMax.x, localMax.y, localMin.z),
			glm::vec3(localMin.x, localMin.y, localMax.z),
			glm::vec3(localMax.x, localMin.y, localMax.z),
			glm::vec3(localMin.x, localMax.y, localMax.z),
			glm::vec3(localMax.x, localMax.y, localMax.z),
		};
		for (int i = 0; i < 8; ++i)
		{
			const glm::vec3 wp = glm::vec3(world * glm::vec4(corners[i], 1.0f));
			mn = glm::min(mn, wp);
			mx = glm::max(mx, wp);
		}
		anyMesh = true;
	}

	if (!anyMesh)
	{
		mn = glm::vec3(-kDefaultExtent);
		mx = glm::vec3( kDefaultExtent);
	}

	outMin = Vector3{ mn };
	outMax = Vector3{ mx };
}

void GameScene::ComputeDirectionalShadowParameters(
    SceneParameters &params,
    const Camera &camera,
    const Vector3 &shadowKeyToLight,
    uint32_t shadowCasterLightIndex)
{
	constexpr uint32_t CASCADE_COUNT = 4;
	constexpr uint32_t SHADOW_SIZE   = 4096u;
	constexpr float    kLambda       = 0.75f;

	const glm::vec3 lightDir = glm::normalize(-glm::vec3(shadowKeyToLight.x, shadowKeyToLight.y, shadowKeyToLight.z));
	const glm::vec3 up = glm::abs(lightDir.y) > 0.95f ? glm::vec3(0, 0, 1) : glm::vec3(0, 1, 0);

	const float camNear  = camera.ClipNear();
	const float camFar   = std::min(camera.ClipFar(), shadowMaxDistance);
	const float range    = camFar - camNear;
	const glm::mat4 invView = Vector::Inverse(camera.View());

	const bool csm = (shadowMode == SceneShadowMode::CSM2x2);
	const uint32_t cascadeCount = csm ? CASCADE_COUNT : 1u;

	float splits[CASCADE_COUNT + 1];
	splits[0] = camNear;
	if (csm)
	{
		for (uint32_t i = 1; i <= CASCADE_COUNT; ++i)
		{
			float p = static_cast<float>(i) / static_cast<float>(CASCADE_COUNT);
			float log  = camNear * std::pow(camFar / camNear, p);
			float lin  = camNear + range * p;
			splits[i] = kLambda * log + (1.0f - kLambda) * lin;
		}
	}
	else
	{
		splits[1] = camFar;
	}

	const glm::vec3 camPos   = glm::vec3(invView[3]);
	const glm::vec3 camFwd   = -glm::vec3(invView[2]);
	const glm::vec3 camRight =  glm::vec3(invView[0]);
	const glm::vec3 camUp    =  glm::vec3(invView[1]);

	const glm::mat4 proj = camera.Projection();
	const float tanHalfFovY = 1.0f / proj[1][1];
	const float aspect      = proj[1][1] / proj[0][0];

	for (uint32_t c = 0; c < cascadeCount; ++c)
	{
		float nearDist = splits[c];
		float farDist  = splits[c + 1];

		float nearH = tanHalfFovY * nearDist;
		float nearW = nearH * aspect;
		float farH  = tanHalfFovY * farDist;
		float farW  = farH * aspect;

		glm::vec3 nc = camPos + camFwd * nearDist;
		glm::vec3 fc = camPos + camFwd * farDist;

		glm::vec3 cornersWS[8] = {
			nc - camUp * nearH - camRight * nearW,
			nc + camUp * nearH - camRight * nearW,
			nc + camUp * nearH + camRight * nearW,
			nc - camUp * nearH + camRight * nearW,
			fc - camUp * farH  - camRight * farW,
			fc + camUp * farH  - camRight * farW,
			fc + camUp * farH  + camRight * farW,
			fc - camUp * farH  + camRight * farW,
		};

		glm::vec3 frustumCenter(0.0f);
		for (int j = 0; j < 8; ++j)
			frustumCenter += cornersWS[j];
		frustumCenter /= 8.0f;

		float radius = 0.0f;
		for (int j = 0; j < 8; ++j)
			radius = std::max(radius, glm::length(cornersWS[j] - frustumCenter));

		glm::vec3 eye = frustumCenter - lightDir * (radius + 50.0f);
		glm::mat4 cascadeLightView = glm::lookAt(eye, frustumCenter, up);
		glm::mat4 lightProj = glm::ortho(-radius, radius, -radius, radius, 0.1f, radius * 2.0f + 100.0f);

		glm::mat4 shadowVP = lightProj * cascadeLightView;

		glm::vec4 originClip = shadowVP * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
		float texelSizeNDC = 2.0f / static_cast<float>(SHADOW_SIZE);

		float offsetX = std::fmod(originClip.x, texelSizeNDC);
		float offsetY = std::fmod(originClip.y, texelSizeNDC);

		shadowVP[3][0] -= offsetX;
		shadowVP[3][1] -= offsetY;

		params.shadowCascadeViewProjection[c] = shadowVP;
		params.shadowCascadeSplits[c] = splits[c + 1];
	}

	for (uint32_t c = cascadeCount; c < CASCADE_COUNT; ++c)
	{
		params.shadowCascadeViewProjection[c] = params.shadowCascadeViewProjection[cascadeCount - 1];
		params.shadowCascadeSplits[c] = std::numeric_limits<float>::max();
	}

	params.shadowCascadeCount     = csm ? CASCADE_COUNT : 1u;
	params.shadowViewProjection   = params.shadowCascadeViewProjection[0];
	params.shadowCasterLightIndex = shadowCasterLightIndex;
	params.shadowEnabled          = 1u;
}

void GameScene::OnRender(const Camera &camera)
{
    float deltaTime = Time::DeltaTime;
	bool isDeferred = IsDeferred();

    if (!render2d)
	{
		render2d = new Render2D;
	}

	CommandBuffer *commandBuffer = Application::Reference().GetCurrentCommandBuffer();

    SceneParameters params{};
	params.view             = camera.View();
    params.viewProjection   = camera.ViewProjection();
	params.invViewProjection = Vector::Inverse(camera.ViewProjection());
	const Vector3 cameraWorldPos = camera.GetWorldPosition();
	params.cameraWorld       = Vector4{ cameraWorldPos, 1.0f };
	params.skyboxProjection = camera.Projection() * Matrix4(Vector::Matrix3(camera.View()));
	params.exposure         = settings.exposure;
	params.gamma            = settings.gamma;
	params.shadowEnablePcf  = shadowForwardPcfEnabled ? 1u : 0u;

	Vector3 shadowKeyToLight{};
	bool haveShadowCaster = false;
	uint32_t shadowCasterLightIndex = 0;
	UpdateLight(params, camera, shadowKeyToLight, haveShadowCaster, shadowCasterLightIndex);

	params.shadowEnabled = 0;
	params.shadowCascadeCount = 0;
	for (uint32_t c = 0; c < SceneParameters::MaxShadowCascades; c++)
	{
		params.shadowCascadeSplits[c] = FLT_MAX;
		params.shadowCascadeViewProjection[c] = params.shadowViewProjection;
	}

	const bool viewportOk = viewportSize.x > 1.0f && viewportSize.y > 1.0f;
	if (!(userShadowEnabled && haveShadowCaster && viewportOk))
	{
		shadowFrustumCenterSmoothInited = false;
	}
	if (userShadowEnabled && haveShadowCaster && viewportOk)
	{
		ComputeDirectionalShadowParameters(params, camera, shadowKeyToLight, shadowCasterLightIndex);
	}

	{
		auto skinnedView = registry.view<TransformComponent, MeshComponent>();
		for (auto e : skinnedView)
		{
			auto &mc = skinnedView.get<MeshComponent>(e);
			if (!mc.Mesh || !mc.Mesh->IsSkinned())
			{
				continue;
			}
			auto &tf = skinnedView.get<TransformComponent>(e);
			Mesh &m = *mc.Mesh;
			if (m.IsAnimated())
			{
				auto &clips = m.GetAnimation();
				uint32_t idx = m.GetAnimationState();
				if (!clips.empty() && idx < clips.size() && clips[idx].Duration > 1e-6f)
				{
					clips[idx].Ticks(deltaTime);
				}
			}
			m.CalculatedBoneTransform(tf.Transform());
			m.UpdateBoneTransforms();
		}
	}

	if (isDeferred && deferredTask)
	{
		Texture *d0 = nullptr;
		Texture *d1 = nullptr;
		Texture *d2 = nullptr;
		Texture *d3 = nullptr;
		if (params.shadowEnabled)
		{
			if (params.shadowCascadeCount > 0u)
			{
				for (uint32_t i = 0; i < 4u; ++i)
				{
					Texture *td = (shadowCascadeRT[i] && shadowCascadeRT[i]->GetDepthAttachment()) ? shadowCascadeRT[i]->GetDepthAttachment() : nullptr;
					if (i == 0u)
					{
						d0 = td;
					}
					else if (i == 1u)
					{
						d1 = td;
					}
					else if (i == 2u)
					{
						d2 = td;
					}
					else
					{
						d3 = td;
					}
				}
			}
			else if (shadowMapRT && shadowMapRT->GetDepthAttachment())
			{
				d0 = shadowMapRT->GetDepthAttachment();
			}
		}
		if (params.shadowEnabled && d0)
		{
			deferredTask->SetShadowMapForResolve(
			    d0,
			    d1,
			    d2,
			    d3,
			    params.shadowViewProjection,
			    params.shadowCascadeViewProjection,
			    params.shadowCascadeSplits,
			    params.shadowCascadeCount,
			    true,
			    params.shadowBias,
			    params.shadowStrength,
			    params.shadowCasterLightIndex);
		}
		else
		{
			deferredTask->SetShadowMapForResolve(nullptr, nullptr, nullptr, nullptr, {}, nullptr, nullptr, 0, false, 0.0f, 1.0f, 0u);
		}
	}
	else
	{
		Texture *sd0 = nullptr;
		Texture *sd1 = nullptr;
		Texture *sd2 = nullptr;
		Texture *sd3 = nullptr;
		if (params.shadowEnabled)
		{
			if (params.shadowCascadeCount > 0u)
			{
				for (uint32_t i = 0; i < 4u; ++i)
				{
					Texture *td = (shadowCascadeRT[i] && shadowCascadeRT[i]->GetDepthAttachment()) ? shadowCascadeRT[i]->GetDepthAttachment() : nullptr;
					if (i == 0u)
					{
						sd0 = td;
					}
					else if (i == 1u)
					{
						sd1 = td;
					}
					else if (i == 2u)
					{
						sd2 = td;
					}
					else
					{
						sd3 = td;
					}
				}
			}
			else if (shadowMapRT)
			{
				sd0 = shadowMapRT->GetDepthAttachment();
			}
		}
		if (meshletTask)
		{
			Texture *shadowRgb = (shadowMapRT && params.shadowEnabled) ? shadowMapRT->GetColorAttachment(0) : nullptr;
			meshletTask->SetShadowMaps(sd0, sd1, sd2, sd3, shadowRgb);
		}
	}

	if (postProcessTask && hdrRenderTarget)
	{
		postProcessTask->SetHdrSceneColor(hdrRenderTarget->GetColorAttachment(0));
		postProcessTask->SetHdrPickTexture(hdrRenderTarget->GetColorAttachment(1));
	}

	frameGraph->Run(commandBuffer, params, registry);

	{
		size_t size = shadowMode == SceneShadowMode::CSM2x2 ? 4 : 1;
		for (size_t i = 0; i < size; i++)
		{
			depthToRgbTasks[i]->Dispatch(commandBuffer, shadowCascadeRT[i]->GetDepthAttachment(), shadowMapLinearizeZNear, shadowMapLinearizeZFar);
		}
	}

	commandBuffer->BeginRenderTarget(renderTarget, nullptr);

    {
        auto view = registry.view<TransformComponent, SpriteRendererComponent, VideoPlayerComponent, ColorMixingComponent>();
        for (auto object : view)
        {
            auto [transform, sprite, videoPlayer, color] = view.get<TransformComponent, SpriteRendererComponent, VideoPlayerComponent, ColorMixingComponent>(object);
            auto picture = videoPlayer.GetPicture();
            if (picture)
            {
				Graphics::ReleaseResource(sprite.Sprite);
			    Graphics::CreateTexture(picture);
                color.Modified = true;
                transform.Scale = Vector3{ picture.GetDisplayAspectRatio(), 1.0f, 1.0f };
            }
        }
    }

	Render2DComponent(camera, commandBuffer);
	commandBuffer->EndRenderTarget();

	Texture *sceneColorForPost = renderTarget ? renderTarget->GetColorAttachment(0) : nullptr;

	if (isEditorScene && editorGridTask && editorGridTask->IsReady() && showEditorGrid && sceneColorForPost)
	{
		Texture *gridDepth = nullptr;
		if (isDeferred && gbufferRT)
		{
			gridDepth = gbufferRT->GetDepthAttachment();
		}
		else if (!isDeferred && hdrRenderTarget)
		{
			gridDepth = hdrRenderTarget->GetDepthAttachment();
		}
		if (gridDepth)
		{
			Matrix4 invVP = Vector::Inverse(camera.ViewProjection());
			Vector4 camWorld{ Vector::Inverse(camera.View())[3] };
			editorGridTask->ExecutePost(commandBuffer, sceneColorForPost, gridDepth, invVP, camera.ViewProjection(), camWorld);
			sceneColorForPost = editorGridTask->GetPostOutputTarget()->GetColorAttachment(0);
		}
	}

	if (showShadowMapDepthDebug)
	{
		commandBuffer->BeginRenderTarget(renderTarget, nullptr);
		depthToRgbTasks[0]->DrawPresentFullscreen(commandBuffer);
		commandBuffer->EndRenderTarget();
	}

	RenderOutline(commandBuffer, sceneColorForPost);
}

void GameScene::RenderOutline(CommandBuffer *commandBuffer, Texture *sceneColorForPost)
{
	if (!(outlineTask && outlineTask->GetEnabled()))
	{
		return;
	}

	Ref<Texture> objectIdTex = GetObjectIdPickTexture();
	if (sceneColorForPost && objectIdTex)
	{
		if (selectedObject && *selectedObject)
		{
			outlineTask->SetSelectedObjectId((uint32_t) (uint64_t) (*selectedObject));
			uint32_t subMesh = UINT32_MAX;
			if (selectedObject->HasComponent<MeshComponent>())
			{
				subMesh = selectedObject->GetComponent<MeshComponent>().SelectedDrawNodeIndex;
			}
			outlineTask->SetSelectedSubMesh(subMesh);
		}
		else
		{
			outlineTask->SetSelectedObjectId(0);
			outlineTask->SetSelectedSubMesh(UINT32_MAX);
		}
		outlineTask->Execute(commandBuffer, sceneColorForPost, objectIdTex);
	}
}

DeferredTask::NPRParams &GameScene::GetNPRParams()
{
	return deferredTask->GetNPRParams();
}

void GameScene::SetViewportSize(const Vector2 &size)
{
    viewportSize = size;

	if (isEditorScene && size.x > 1.0f && size.y > 1.0f)
	{
		editorCamera.SetViewportSize(size);
		editorOrthographicCamera.SetViewportSize(size);
	}

	bool isDeferred = IsDeferred();

	if (!frameGraph)
	{
		frameGraph = new FrameGraph;
	}

	frameGraph->Clear();
	
	uint32_t w = (uint32_t)size.x;
	uint32_t h = (uint32_t)size.y;

	constexpr uint32_t kShadowMapSize = 4096;
	const ClearValue shadowRtClears[] = {
	    { .color = { .float32 = { 0.0f, 0.0f, 0.0f, 1.0f } } },
	    { .depthStencil = { 1.0f, 0 } },
	};
	frameGraph->AddRenderTarget({
		RenderTargetCreateInfo{
		    .name         = "SceneHDR",
		    .width        = w,
		    .height       = h,
		    .colorFormats = { Format::R16G16B16A16_SFLOAT, Format::R32G32_UINT },
		    .depthFormat  = Format::Depth24Stencil8,
		    .clearValues  = sceneRTClearValues,
		},
		RenderTargetCreateInfo{
			.name         = "SceneOutput",
			.width        = w,
			.height       = h,
			.colorFormats = { Format::RGBA8, Format::R32G32_UINT},
			.depthFormat  = Format::Depth24Stencil8,
			.clearValues  = sceneRTClearValues
		},
		RenderTargetCreateInfo{
			.name         = "GBufferRT",
			.width        = w,
			.height       = h,
	        .colorFormats = {
				Format::R16G16B16A16_SFLOAT,
				Format::R16G16B16A16_SFLOAT,
				Format::R32G32_UINT,
				Format::R8G8B8A8_UNORM,
			},
			.depthFormat  = Format::Depth24Stencil8,
	        .clearValues  = gbufferClearValues
		},
	});

	{
		size_t size = shadowMode == SceneShadowMode::CSM2x2 ? 4 : 1;
		for (uint32_t i = 0; i < size; ++i)
		{
			frameGraph->AddRenderTarget({
			    RenderTargetCreateInfo{
			        .name         = std::string("ShadowRT") + std::to_string(i),
			        .width        = kShadowMapSize,
			        .height       = kShadowMapSize,
			        .colorFormats = { Format::RGBA8 },
			        .depthFormat  = Format::Depth24Stencil8,
			        .clearValues  = { shadowRtClears[0], shadowRtClears[1] },
			    },
			});
		}
	}

	if (!outlineTask)
	{
		outlineTask = new SelectionOutlineTask;
		outlineTask->Build(Graphics::GetAsyncComputeThread());
	}

	if (isEditorScene && !editorGridTask)
	{
		editorGridTask = new EditorGridTask;
		editorGridTask->Build(Graphics::GetAsyncComputeThread());
	}
	if (editorGridTask)
	{
		editorGridTask->SetViewportSize(size);
	}
	outlineTask->SetViewportSize(size);

	gbufferRT       = frameGraph->QueryRenderTarget("GBufferRT")->renderTarget;
	hdrRenderTarget = frameGraph->QueryRenderTarget("SceneHDR")->renderTarget;
	renderTarget    = frameGraph->QueryRenderTarget("SceneOutput")->renderTarget;
	if (postProcessTask)
	{
		postProcessTask->SetViewportSize(w, h);
	}
	{
		size_t size = shadowMode == SceneShadowMode::CSM2x2 ? 4 : 1;
		for (uint32_t i = 0; i < size; ++i)
		{
			shadowCascadeRT[i] = frameGraph->QueryRenderTarget(std::string("ShadowRT") + std::to_string(i))->renderTarget;
		}
		shadowMapRT = shadowCascadeRT[0];
	}

	if (deferredTask)
	{
		deferredTask->SetGBufferRenderTarget(gbufferRT);
	}

	RebuildFrameGraph();
}

Ref<RenderTarget> GameScene::GetRenderTarget() const
{
	if (outlineTask && outlineTask->IsReady() && outlineTask->GetEnabled())
	{
		return outlineTask->GetOutputTarget();
	}
	if (!IsDeferred() && isEditorScene && editorGridTask && editorGridTask->IsReady() && showEditorGrid && editorGridTask->GetPostOutputTarget()
	    && renderTarget && hdrRenderTarget && hdrRenderTarget->GetDepthAttachment())
	{
		return editorGridTask->GetPostOutputTarget();
	}
	if (IsDeferred() && editorGridTask && editorGridTask->IsReady() && showEditorGrid && isEditorScene && editorGridTask->GetPostOutputTarget())
	{
		return editorGridTask->GetPostOutputTarget();
	}
	return renderTarget;
}

Ref<Texture> GameScene::GetObjectIdPickTexture() const
{
	if (IsDeferred() && gbufferRT)
	{
		return gbufferRT->GetColorAttachment(2);
	}
	if (renderTarget)
	{
		return renderTarget->GetColorAttachment(1);
	}
	return {};
}

void GameScene::UpdateLight(SceneParameters &params, const Camera &camera, Vector3 &shadowKeyToLight, bool &haveShadowCaster, uint32_t &shadowCasterLightIndex)
{
	haveShadowCaster = false;
	shadowKeyToLight = Vector3{};
	shadowCasterLightIndex = 0;

	size_t li = 0;
	auto lightView = registry.view<TransformComponent, LightComponent>();
	for (auto entity : lightView)
	{
		if (li >= SL_ARRAY_LENGTH(params.lights))
		{
			break;
		}

		auto &transform = lightView.get<TransformComponent>(entity);
		auto &light = lightView.get<LightComponent>(entity);
		if (!light.Enabled)
		{
			continue;
		}

		const Vector3 dir = light.DirectionWorld(camera, transform);
		params.lights[li].direction = Vector4{ dir, 1.0f };
		const float k = light.Intensity;
		params.lights[li].radiance = Vector4{
		    light.Radiance.x * k,
		    light.Radiance.y * k,
		    light.Radiance.z * k,
		    light.Range
		};

		params.lights[li].position = Vector4{ transform.Position, static_cast<float>(light.LightType) };
		params.lights[li].spotParams = Vector4{
		    std::cos(light.InnerConeAngle * 3.14159265f / 180.0f),
		    std::cos(light.OuterConeAngle * 3.14159265f / 180.0f),
		    0.0f, 0.0f
		};

		if (!haveShadowCaster && light.CastShadows && light.LightType == LightComponent::Type::Directional)
		{
			haveShadowCaster = true;
			shadowKeyToLight = dir;
			shadowCasterLightIndex = (uint32_t)li;
		}
		li++;
	}
	params.lightCount = (uint32_t)li;

	if (li == 0)
	{
		const Vector3 d = LightComponent::DefaultDirectionalLightDirection();
		params.lights[0].direction  = Vector4{ d, 1.0f };
		params.lights[0].radiance   = Vector4{ 3.5f, 3.5f, 3.5f, 0.0f };
		params.lights[0].position   = Vector4{ 0.0f, 0.0f, 0.0f, static_cast<float>(LightComponent::Type::Directional) };
		params.lights[0].spotParams = Vector4{
		    std::cos(30.0f * 3.14159265f / 180.0f),
		    std::cos(45.0f * 3.14159265f / 180.0f),
		    0.0f, 0.0f
		};
		params.lightCount     = 1u;
		haveShadowCaster      = true;
		shadowKeyToLight      = d;
		shadowCasterLightIndex = 0u;
	}
}

}
