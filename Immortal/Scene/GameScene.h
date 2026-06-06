#pragma once

#include "Scene.h"
#include "EditorCamera.h"
#include "Render/OrthographicCamera.h"
#include "Render/FrameGraph.h"
#include "Render/MeshletTask.h"
#include "Render/MeshletShadowTask.h"
#include "Render/DeferredTask.h"
#include "Render/PostProcessTask.h"
#include "Render/IBLTask.h"
#include "Render/SkyboxTask.h"
#include "Render/AtmosphereTask.h"
#include "Render/SelectionOutlineTask.h"
#include "Render/EditorGridTask.h"
#include "Render/DepthToRgbTask.h"

namespace Immortal
{

enum class SceneRenderMode : uint32_t
{
	Forward  = 0,
	Deferred = 1,
};

enum class SceneLightingModel : uint32_t
{
	Unlit = 0,
	Phong = 1,
	PBR   = 2,
	NPR   = 3,
};

enum class SceneShadowMode : uint32_t
{
	ShadowMapping = 0,
	CSM2x2        = 1,
};

enum class SceneCameraType : uint8_t
{
	EditorPerspective  = 0,
	EditorOrthographic = 1,
};

class IMMORTAL_API GameScene : public Scene
{
public:
	GameScene(const String &name = "Untitled", bool isEditorScene = false);

	~GameScene() override;

	void OnGuiRender() override;

	void OnRenderRuntime() override;

	void OnRenderEditor();

	void OnRender(const Camera &camera);

	void RenderOutline(CommandBuffer *commandBuffer, Texture *sceneColorForPost);

	void SetViewportSize(const Vector2 &size) override;

	Ref<RenderTarget> GetRenderTarget() const override;

	Ref<Texture> GetObjectIdPickTexture() const;

	void SetCameraType(SceneCameraType type);

	SceneCameraType GetCameraType() const { return editorHostCameraType; }

	EditorCamera &GetEditorCamera() { return editorCamera; }

	const EditorCamera &GetEditorCamera() const { return editorCamera; }

	OrthographicCamera &GetEditorOrthographicCamera() { return editorOrthographicCamera; }

	const OrthographicCamera &GetEditorOrthographicCamera() const { return editorOrthographicCamera; }

	Camera *GetEditorPrimaryCamera();

	const Camera *GetEditorPrimaryCamera() const;

	void InitRenderTask();

	void RebuildFrameGraph();

	IBLTask *GetIBLTask() const { return iblTask; }
	SkyboxTask *GetSkyboxTask() const { return skyboxTask; }
	AtmosphereTask *GetAtmosphereTask() const { return atmosphereTask; }

	DeferredTask::NPRParams &GetNPRParams();

	SelectionOutlineTask *GetOutlineTask() const { return outlineTask; }

	EditorGridTask *GetEditorGridTask() const { return editorGridTask; }

	void SetShowEditorGrid(bool value) { showEditorGrid = value; }

	bool GetShowEditorGrid() const { return showEditorGrid; }

	void SetShowShadowMapDepthDebug(bool value) { showShadowMapDepthDebug = value; }

	bool GetShowShadowMapDepthDebug() const { return showShadowMapDepthDebug; }

	bool IsDeferred() const
	{
		return renderMode == SceneRenderMode::Deferred;
	}

private:
	void UpdateLight(SceneParameters &params, const Camera &camera, Vector3 &shadowKeyToLight, bool &haveShadowCaster, uint32_t &shadowCasterLightIndex);

	void ComputeDirectionalShadowParameters(
	    SceneParameters &params,
	    const Camera &camera,
	    const Vector3 &shadowKeyToLight,
	    uint32_t shadowCasterLightIndex);

	void ComputeSceneWorldAABB(Vector3 &outMin, Vector3 &outMax) const;

protected:
	Ref<FrameGraph> frameGraph;

	std::vector<ClearValue> sceneRTClearValues = {
	    { .color = { .float32 = { 0.0f, 0.0f, 0.0f, 0.0f } } },
	    { .color = { .uint32  = { 0u, 0u, 0u, 0u } } },
	    { .depthStencil = { .depth = 1.0f, .stencil = 0 } },
	};

	Ref<RenderTarget> shadowMapRT;

	Ref<RenderTarget> shadowCascadeRT[4]{};

	float shadowMapLinearizeZNear = 0.0f;
	float shadowMapLinearizeZFar = 1.0f;

	Ref<RenderTarget> gbufferRT;

	Ref<RenderTarget> hdrRenderTarget;

	Ref<PostProcessTask> postProcessTask;

	SceneRenderMode renderMode = SceneRenderMode::Forward;
	SceneLightingModel lightingModel = SceneLightingModel::Phong;
	SceneShadowMode shadowMode = SceneShadowMode::ShadowMapping;
	bool userShadowEnabled = true;
	bool shadowForwardPcfEnabled = true;
	float shadowMaxDistance = 200.0f;

	Vector3 shadowFrustumCenterSmoothed[SceneParameters::MaxShadowCascades]{};
	bool shadowFrustumCenterSmoothInited = false;

	Ref<MeshletTask> meshletTask;
	Ref<MeshletShadowTask> meshletShadowTask;

	Ref<DeferredTask> deferredTask;

	ClearValue shadowClearValue[1] = {
	    { .depthStencil = { .depth = 1.0f, .stencil = 0 } },
	};

	std::vector<ClearValue> gbufferClearValues = {
	    { .color = { .float32 = { 0.0f, 0.0f, 0.0f, 0.0f } } },
	    { .color = { .float32 = { 0.5f, 0.5f, 1.0f, 1.0f } } },
	    { .color = { .uint32  = { 0u, 0u, 0u, 0u } } },
	    { .color = { .float32 = { 0.0f, 0.0f, 0.0f, 0.0f } } },
	    { .depthStencil = { .depth = 1.0f, .stencil = 0 } },
	};

	Ref<IBLTask> iblTask;

	Ref<SkyboxTask> skyboxTask;

	Ref<AtmosphereTask> atmosphereTask;

	Ref<SelectionOutlineTask> outlineTask;

	Ref<EditorGridTask> editorGridTask;

	Ref<DepthToRgbTask> depthToRgbTasks[4];

	bool showEditorGrid = true;

	bool showShadowMapDepthDebug = false;

private:
	EditorCamera editorCamera;

	OrthographicCamera editorOrthographicCamera;

	SceneCameraType editorHostCameraType = SceneCameraType::EditorPerspective;
};

}
