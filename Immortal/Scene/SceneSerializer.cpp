#include "SceneSerializer.h"

#include <cctype>

#include "EditorCamera.h"
#include "GameScene.h"
#include "FileSystem/FileSystem.h"
#include "Helper/json.h"
#include "Scene/Component.h"
#include "Scene/Object.h"
#include "Scene/VideoPlayerComponent.h"

namespace Immortal
{

namespace
{

using Json = nlohmann::json;

constexpr const char *kFormatVersion = "0.0.3";

static std::string MeshAsciiLower(std::string s)
{
	for (char &c : s)
	{
		c = (char)std::tolower((unsigned char)c);
	}
	return s;
}

static Ref<Mesh> CreateBuiltinPrimitiveMesh(const std::string &kindIn)
{
	const std::string k = MeshAsciiLower(kindIn);
	if (k == "plane")
	{
		return Mesh::CreatePlane();
	}
	if (k == "cube")
	{
		return Mesh::CreateCube();
	}
	if (k == "sphere")
	{
		return Mesh::CreateSphere();
	}
	if (k == "cylinder")
	{
		return Mesh::CreateCylinder();
	}
	if (k == "capsule")
	{
		return Mesh::CreateCapsule();
	}
	if (k == "cone")
	{
		return Mesh::CreateCone();
	}
	if (k == "torus")
	{
		return Mesh::CreateTorus();
	}
	return {};
}

static void EnsureMaterialTextureDefaults(MaterialComponent::Reference &ref)
{
	auto *preset = Graphics::Preset();
	if (!ref.Textures.Albedo)
	{
		ref.Textures.Albedo = preset->Textures.White;
	}
	if (!ref.Textures.Normal)
	{
		ref.Textures.Normal = preset->Textures.Normal;
	}
	if (!ref.Textures.Specular)
	{
		ref.Textures.Specular = ref.Textures.Albedo;
	}
	if (!ref.Textures.Metallic)
	{
		ref.Textures.Metallic = ref.Textures.Albedo;
	}
	if (!ref.Textures.Roughness)
	{
		ref.Textures.Roughness = ref.Textures.Albedo;
	}
	if (!ref.Textures.AmbientOcclusion)
	{
		ref.Textures.AmbientOcclusion = ref.Textures.Albedo;
	}
}

Json TryFind(const Json &j, const std::string &key)
{
	if (j.is_object() && j.contains(key))
	{
		return j.at(key);
	}
	return Json{};
}

void WriteVec3(Json &j, const Vector3 &v)
{
	j = Json::object({ { "x", v.x }, { "y", v.y }, { "z", v.z } });
}

void ReadVec3(const Json &j, Vector3 &v)
{
	if (!j.is_object())
	{
		return;
	}
	if (j.contains("x"))
	{
		j.at("x").get_to(v.x);
	}
	if (j.contains("y"))
	{
		j.at("y").get_to(v.y);
	}
	if (j.contains("z"))
	{
		j.at("z").get_to(v.z);
	}
}

void WriteVec4(Json &j, const Vector4 &v)
{
	j = Json::object({ { "x", v.x }, { "y", v.y }, { "z", v.z }, { "w", v.w } });
}

void ReadVec4(const Json &j, Vector4 &v)
{
	if (!j.is_object())
	{
		return;
	}
	if (j.contains("x"))
	{
		j.at("x").get_to(v.x);
	}
	if (j.contains("y"))
	{
		j.at("y").get_to(v.y);
	}
	if (j.contains("z"))
	{
		j.at("z").get_to(v.z);
	}
	if (j.contains("w"))
	{
		j.at("w").get_to(v.w);
	}
}

void WriteMatrix4(Json &j, const Matrix4 &m)
{
	Json arr = Json::array();
	for (int c = 0; c < 4; c++)
	{
		for (int r = 0; r < 4; r++)
		{
			arr.push_back(m[c][r]);
		}
	}
	j = std::move(arr);
}

bool ReadMatrix4(const Json &j, Matrix4 &m)
{
	if (!j.is_array() || j.size() != 16)
	{
		return false;
	}
	size_t k = 0;
	for (int c = 0; c < 4; c++)
	{
		for (int r = 0; r < 4; r++)
		{
			m[c][r] = j.at(k++).get<float>();
		}
	}
	return true;
}

void SerializeTag(Json &objectData, const TagComponent &c)
{
	objectData["Name"] = c.Tag;
}

void SerializeTransform(Json &objectData, const TransformComponent &t)
{
	Json tr = Json::object();
	WriteVec3(tr["Position"], t.Position);
	WriteVec3(tr["Rotation"], t.Rotation);
	WriteVec3(tr["Scale"], t.Scale);
	objectData["Transform"] = std::move(tr);
}

void SerializeLight(Json &objectData, const LightComponent &l)
{
	Json light = Json::object();
	WriteVec4(light["Radiance"], l.Radiance);
	light["Intensity"]     = l.Intensity;
	light["LightType"]     = static_cast<uint32_t>(l.LightType);
	light["Range"]         = l.Range;
	light["InnerConeAngle"] = l.InnerConeAngle;
	light["OuterConeAngle"] = l.OuterConeAngle;
	light["Enabled"]      = l.Enabled;
	light["CastShadows"]    = l.CastShadows;
	objectData["Light"]     = std::move(light);
}

void SerializeDirectionalLight(Json &objectData, const DirectionalLightComponent &d)
{
	Json j = Json::object();
	WriteVec3(j["Radiance"], d.Radiance);
	j["Intensity"]   = d.Intensity;
	j["CastShadows"] = d.CastShadows;
	j["SoftShadows"] = d.SoftShadows;
	j["LightSize"]   = d.LightSize;
	objectData["DirectionalLight"] = std::move(j);
}

void SerializeSpriteRenderer(Json &objectData, const SpriteRendererComponent &s)
{
	Json sprite = Json::object();
	WriteVec4(sprite["Color"], s.Color);
	sprite["TilingFactor"] = s.TilingFactor;
	objectData["SpriteRenderer"] = std::move(sprite);
}

void SerializeColorMixing(Json &objectData, const ColorMixingComponent &c)
{
	Json j = Json::object();
	j["Modified"]    = c.Modified;
	j["Initialized"] = c.Initialized;
	WriteVec4(j["RGBA"], c.RGBA);
	WriteVec4(j["HSL"], c.HSL);
	j["WhiteBalance"] = Json::object({ { "ColorTemperature", c.WhiteBalance.ColorTemperature }, { "Hue", c.WhiteBalance.Hue } });
	j["Gradation"]    = Json::object({ { "White", c.Gradation.White }, { "Black", c.Gradation.Black } });
	j["Exposure"]     = c.Exposure;
	j["Contrast"]     = c.Contrast;
	j["Hightlights"]  = c.Hightlights;
	j["Shadow"]       = c.Shadow;
	j["Vividness"]    = c.Vividness;
	objectData["ColorMixing"] = std::move(j);
}

void SerializeMesh(Json &objectData, const MeshComponent &mesh, const std::string &entityName)
{
	if (!mesh.Mesh)
	{
		return;
	}
	Json meshObject = Json::object();
	const std::string path = mesh.Mesh->Source();
	if (!path.empty())
	{
		meshObject["kind"]   = "file";
		meshObject["Source"] = path;
	}
	else
	{
		meshObject["kind"]           = "primitive";
		meshObject["primitiveKind"] = entityName;
	}
	meshObject["SelectedDrawNodeIndex"] = mesh.SelectedDrawNodeIndex;
	if (!mesh.SubmeshLocalTransform.empty())
	{
		Json arr = Json::array();
		for (const Matrix4 &mat : mesh.SubmeshLocalTransform)
		{
			Json mjson;
			WriteMatrix4(mjson, mat);
			arr.push_back(std::move(mjson));
		}
		meshObject["SubmeshLocalTransform"] = std::move(arr);
	}
	objectData["Mesh"] = std::move(meshObject);
}

void SerializeMaterial(Json &objectData, const MaterialComponent &material)
{
	Json materialObject = Json::array();
	for (const auto &ref : material.References)
	{
		Json j = Json::object();
		j["Name"]      = ref.Name;
		j["Metallic"]  = ref.Metallic;
		j["Roughness"] = ref.Roughness;
		j["Opacity"]   = ref.Opacity;
		WriteVec4(j["AlbedoColor"], ref.AlbedoColor);
		WriteVec4(j["Specular"], ref.Specular);
		WriteVec4(j["Ambient"], ref.Ambient);
		WriteVec4(j["Emissive"], ref.Emissive);
		Json paths = Json::object();
		paths["Diffuse"]            = std::string(ref.Pathes.Diffuse);
		paths["Normal"]             = std::string(ref.Pathes.Normal);
		paths["Specular"]          = std::string(ref.Pathes.Specular);
		paths["Metallic"]          = std::string(ref.Pathes.Metallic);
		paths["Roughness"]        = std::string(ref.Pathes.Roughness);
		paths["AmbientOcclusion"] = std::string(ref.Pathes.AmbientOcclusion);
		j["Paths"] = std::move(paths);
		materialObject.push_back(std::move(j));
	}
	objectData["Material"] = std::move(materialObject);
}

void SerializeScript(Json &objectData, const ScriptComponent &script)
{
	Json scriptObject = Json::object();
	scriptObject["Source"]    = script.path;
	scriptObject["ClassName"] = script.className;
	objectData["Script"]      = std::move(scriptObject);
}

void SerializeVideoPlayer(Json &objectData, const VideoPlayerComponent &vp)
{
	Json j    = Json::object();
	j["Source"] = std::string(vp.GetSource());
	j["Speed"]  = vp.speed;
	objectData["VideoPlayer"] = std::move(j);
}

void SerializeCamera(Json &objectData, const CameraComponent &camera)
{
	Json cameraObject = Json::object();
	cameraObject["Primary"] = camera.Primary;
	const SceneCamera &sc = camera.Camera;
	cameraObject["ProjectionType"] = static_cast<int>(sc.GetType());
	cameraObject["PerspectiveVerticalFOV"] = sc.PerspectiveVerticalFOV();
	cameraObject["PerspectiveNear"]        = sc.PerspectiveNearClip();
	cameraObject["PerspectiveFar"]         = sc.PerspectiveFarClip();
	cameraObject["OrthographicSize"]       = sc.OrthographicSize();
	cameraObject["OrthographicNear"]       = sc.OrthographicNearClip();
	cameraObject["OrthographicFar"]        = sc.OrthographicFarClip();
	objectData["Camera"] = std::move(cameraObject);
}

void SerializeId(Json &objectData, const IDComponent &id)
{
	objectData["ID"] = Json::object({ { "uid", id.uid } });
}

void DeserializeTransform(const Json &j, TransformComponent &t)
{
	const Json tr = TryFind(j, "Transform");
	if (tr.is_null() || !tr.is_object())
	{
		return;
	}
	ReadVec3(TryFind(tr, "Position"), t.Position);
	ReadVec3(TryFind(tr, "Rotation"), t.Rotation);
	ReadVec3(TryFind(tr, "Scale"), t.Scale);
}

void DeserializeLight(const Json &j, LightComponent &l)
{
	const Json light = TryFind(j, "Light");
	if (light.is_null() || !light.is_object())
	{
		return;
	}
	l.Enabled = light.value("Enabled", l.Enabled);
	if (light.contains("Radiance"))
	{
		ReadVec4(light["Radiance"], l.Radiance);
	}
	l.LightType = static_cast<LightComponent::Type>(light.value("LightType", static_cast<uint32_t>(l.LightType)));
	l.Intensity         = light.value("Intensity", l.Intensity);
	l.Range            = light.value("Range", l.Range);
	l.InnerConeAngle   = light.value("InnerConeAngle", l.InnerConeAngle);
	l.OuterConeAngle   = light.value("OuterConeAngle", l.OuterConeAngle);
	l.CastShadows      = light.value("CastShadows", l.CastShadows);
}

void DeserializeDirectionalLight(const Json &j, DirectionalLightComponent &d)
{
	const Json block = TryFind(j, "DirectionalLight");
	if (block.is_null() || !block.is_object())
	{
		return;
	}
	ReadVec3(TryFind(block, "Radiance"), d.Radiance);
	d.Intensity   = block.value("Intensity", d.Intensity);
	d.CastShadows = block.value("CastShadows", d.CastShadows);
	d.SoftShadows = block.value("SoftShadows", d.SoftShadows);
	d.LightSize   = block.value("LightSize", d.LightSize);
}

void DeserializeSpriteRenderer(Object &object, const Json &j)
{
	const Json sprite = TryFind(j, "SpriteRenderer");
	if (sprite.is_null() || !sprite.is_object())
	{
		return;
	}
	auto &s = object.AddComponent<SpriteRendererComponent>();
	ReadVec4(TryFind(sprite, "Color"), s.Color);
	s.TilingFactor = sprite.value("TilingFactor", s.TilingFactor);
	if (sprite.contains("Source") && sprite["Source"].is_string())
	{
		const std::string src = sprite["Source"].get<std::string>();
		if (!src.empty())
		{
			s.Sprite = Graphics::CreateTexture(src);
			if (s.Sprite)
			{
				s.Result = Graphics::CreateTexture(Format::RGBA8, s.Sprite->GetWidth(), s.Sprite->GetHeight());
			}
		}
	}
	object.AddComponent<ColorMixingComponent>();
}

void DeserializeColorMixing(const Json &j, ColorMixingComponent &c)
{
	const Json block = TryFind(j, "ColorMixing");
	if (block.is_null() || !block.is_object())
	{
		return;
	}
	c.Modified    = block.value("Modified", c.Modified);
	c.Initialized = block.value("Initialized", c.Initialized);
	ReadVec4(TryFind(block, "RGBA"), c.RGBA);
	ReadVec4(TryFind(block, "HSL"), c.HSL);
	const Json wb = TryFind(block, "WhiteBalance");
	if (wb.is_object())
	{
		c.WhiteBalance.ColorTemperature = wb.value("ColorTemperature", c.WhiteBalance.ColorTemperature);
		c.WhiteBalance.Hue              = wb.value("Hue", c.WhiteBalance.Hue);
	}
	const Json gr = TryFind(block, "Gradation");
	if (gr.is_object())
	{
		c.Gradation.White = gr.value("White", c.Gradation.White);
		c.Gradation.Black = gr.value("Black", c.Gradation.Black);
	}
	c.Exposure    = block.value("Exposure", c.Exposure);
	c.Contrast    = block.value("Contrast", c.Contrast);
	c.Hightlights = block.value("Hightlights", c.Hightlights);
	c.Shadow      = block.value("Shadow", c.Shadow);
	c.Vividness   = block.value("Vividness", c.Vividness);
}

void DeserializeMesh(const Json &j, MeshComponent &meshComponent, const std::string &entityName)
{
	const Json meshObject = TryFind(j, "Mesh");
	if (meshObject.is_null() || !meshObject.is_object())
	{
		return;
	}
	const std::string kind = meshObject.value("kind", std::string{});
	std::string       src;
	if (meshObject.contains("Source") && meshObject["Source"].is_string())
	{
		src = meshObject["Source"].get<std::string>();
	}
	const std::string primitiveHint = meshObject.value("primitiveKind", entityName);

	Ref<Mesh> loaded;
	if (kind == "primitive")
	{
		loaded = CreateBuiltinPrimitiveMesh(primitiveHint);
	}
	else if (!src.empty())
	{
		loaded = new Mesh{Graphics::GetAsyncComputeThread(), nullptr, src};
	}
	else if (kind.empty())
	{
		// Legacy scenes: "Source" was present but empty for editor primitives.
		loaded = CreateBuiltinPrimitiveMesh(primitiveHint);
	}

	if (!loaded)
	{
		return;
	}
	meshComponent.Mesh                  = loaded;
	meshComponent.SelectedDrawNodeIndex = meshObject.value("SelectedDrawNodeIndex", meshComponent.SelectedDrawNodeIndex);
	const Json arr = TryFind(meshObject, "SubmeshLocalTransform");
	if (arr.is_array() && meshComponent.Mesh)
	{
		const size_t n = meshComponent.Mesh->NodeList().size();
		meshComponent.EnsureSubmeshLocalCount(n);
		for (size_t i = 0; i < arr.size() && i < meshComponent.SubmeshLocalTransform.size(); i++)
		{
			ReadMatrix4(arr.at(i), meshComponent.SubmeshLocalTransform[i]);
		}
	}
}

void DeserializeMaterial(const Json &j, MaterialComponent &material, MeshComponent &meshComponent)
{
	const Json materialObject = TryFind(j, "Material");
	if (!materialObject.is_array() || !meshComponent.Mesh)
	{
		return;
	}
	material.References.resize(meshComponent.Mesh->Size());
	meshComponent.Mesh->PopulateMaterialComponent(material);

	auto loadPath = [](Ref<Texture> &texture, const std::string &path) {
		if (path.empty())
		{
			return;
		}
		Ref<Texture> loaded = Graphics::CreateTexture(path);
		if (loaded)
		{
			texture = std::move(loaded);
		}
	};

	for (size_t i = 0; i < materialObject.size() && i < material.References.size(); i++)
	{
		auto &ref     = material.References[i];
		const Json &m = materialObject.at(i);
		ref.Name = m.value("Name", ref.Name);
		if (m.contains("AlbedoColor"))
		{
			ReadVec4(m["AlbedoColor"], ref.AlbedoColor);
		}
		else if (m.contains("Albedo"))
		{
			ReadVec4(m["Albedo"], ref.AlbedoColor);
		}
		if (m.contains("Specular"))
		{
			ReadVec4(m["Specular"], ref.Specular);
		}
		if (m.contains("Ambient"))
		{
			ReadVec4(m["Ambient"], ref.Ambient);
		}
		if (m.contains("Emissive"))
		{
			ReadVec4(m["Emissive"], ref.Emissive);
		}
		ref.Metallic  = m.value("Metallic", m.value("Metalness", ref.Metallic));
		ref.Roughness = m.value("Roughness", ref.Roughness);
		ref.Opacity   = m.value("Opacity", ref.Opacity);

		Json paths = TryFind(m, "Paths");
		if (!paths.is_object())
		{
			paths = TryFind(m, "Pathes");
		}
		if (paths.is_object())
		{
			if (paths.contains("Diffuse"))
			{
				ref.Pathes.Diffuse = paths["Diffuse"].get<std::string>();
			}
			if (paths.contains("Normal"))
			{
				ref.Pathes.Normal = paths["Normal"].get<std::string>();
			}
			if (paths.contains("Specular"))
			{
				ref.Pathes.Specular = paths["Specular"].get<std::string>();
			}
			if (paths.contains("Metallic"))
			{
				ref.Pathes.Metallic = paths["Metallic"].get<std::string>();
			}
			if (paths.contains("Roughness"))
			{
				ref.Pathes.Roughness = paths["Roughness"].get<std::string>();
			}
			if (paths.contains("AmbientOcclusion"))
			{
				ref.Pathes.AmbientOcclusion = paths["AmbientOcclusion"].get<std::string>();
			}
			loadPath(ref.Textures.Albedo, std::string(ref.Pathes.Diffuse));
			loadPath(ref.Textures.Normal, std::string(ref.Pathes.Normal));
			loadPath(ref.Textures.Specular, std::string(ref.Pathes.Specular));
			loadPath(ref.Textures.Metallic, std::string(ref.Pathes.Metallic));
			loadPath(ref.Textures.Roughness, std::string(ref.Pathes.Roughness));
			loadPath(ref.Textures.AmbientOcclusion, std::string(ref.Pathes.AmbientOcclusion));
		}

		const Json textures = TryFind(m, "Textures");
		if (textures.is_object())
		{
			if (textures.contains("Albedo"))
			{
				loadPath(ref.Textures.Albedo, textures["Albedo"].get<std::string>());
			}
			if (textures.contains("Normal"))
			{
				loadPath(ref.Textures.Normal, textures["Normal"].get<std::string>());
			}
			if (textures.contains("Metalness"))
			{
				loadPath(ref.Textures.Metallic, textures["Metalness"].get<std::string>());
			}
			if (textures.contains("Metallic"))
			{
				loadPath(ref.Textures.Metallic, textures["Metallic"].get<std::string>());
			}
			if (textures.contains("Roughness"))
			{
				loadPath(ref.Textures.Roughness, textures["Roughness"].get<std::string>());
			}
		}
	}

	for (auto &ref : material.References)
	{
		EnsureMaterialTextureDefaults(ref);
	}
}

void DeserializeScript(Scene *scene, Object &object, const Json &j)
{
	const Json scriptObject = TryFind(j, "Script");
	if (scriptObject.is_null() || !scriptObject.is_object() || !scriptObject.contains("Source"))
	{
		return;
	}
	auto &script = object.AddComponent<ScriptComponent>(scriptObject["Source"].get<std::string>());
	if (scriptObject.contains("ClassName"))
	{
		script.className = scriptObject["ClassName"].get<std::string>();
	}
	script.Init((int)object, scene);
}

void DeserializeCamera(const Json &j, CameraComponent &camera)
{
	const Json cameraObject = TryFind(j, "Camera");
	if (cameraObject.is_null() || !cameraObject.is_object())
	{
		return;
	}
	camera.Primary = cameraObject.value("Primary", camera.Primary);
	const int proj = cameraObject.value("ProjectionType", static_cast<int>(camera.Camera.GetType()));
	camera.Camera.SetProjectionType(static_cast<Camera::ProjectionType>(proj));
	camera.Camera.SetPerspectiveVerticalFOV(cameraObject.value("PerspectiveVerticalFOV", camera.Camera.PerspectiveVerticalFOV()));
	camera.Camera.PerspectiveNearClip() = cameraObject.value("PerspectiveNear", camera.Camera.PerspectiveNearClip());
	camera.Camera.PerspectiveFarClip()  = cameraObject.value("PerspectiveFar", camera.Camera.PerspectiveFarClip());
	camera.Camera.SetOrthographicSize(cameraObject.value("OrthographicSize", camera.Camera.OrthographicSize()));
	camera.Camera.OrthographicNearClip() = cameraObject.value("OrthographicNear", camera.Camera.OrthographicNearClip());
	camera.Camera.OrthographicFarClip()  = cameraObject.value("OrthographicFar", camera.Camera.OrthographicFarClip());
	camera.Camera.RefreshCameraClipPlanes();
}

void DeserializeVideoPlayer(Object &object, const Json &j)
{
	const Json block = TryFind(j, "VideoPlayer");
	if (block.is_null() || !block.is_object() || !block.contains("Source"))
	{
		return;
	}
	const std::string src = block["Source"].get<std::string>();
	if (src.empty())
	{
		return;
	}
	object.AddComponent<VideoPlayerComponent>(String{ src });
	auto &vp = object.GetComponent<VideoPlayerComponent>();
	if (block.contains("Speed"))
	{
		vp.speed = block["Speed"].get<double>();
	}
}

void DeserializeId(const Json &j, IDComponent &id)
{
	const Json block = TryFind(j, "ID");
	if (block.is_null() || !block.is_object())
	{
		return;
	}
	id.uid = block.value("uid", id.uid);
}

void SerializeEditorView(Json &data, EditorCamera *c)
{
	if (!c)
	{
		return;
	}

	Vector3 focal;
	float   dist, pitchDeg, yawDeg, fovDeg;
	c->ExportOrbitSnapshot(focal, dist, pitchDeg, yawDeg, fovDeg);
	Json ev = Json::object();
	WriteVec3(ev["FocalPoint"], focal);
	ev["Distance"] = dist;
	ev["Pitch"]    = pitchDeg;
	ev["Yaw"]      = yawDeg;
	ev["FOV"]      = fovDeg;
	ev["zNear"]    = c->ClipNear();
	ev["zFar"]     = c->ClipFar();
	data["EditorView"] = std::move(ev);
}

void DeserializeEditorView(const Json &root, EditorCamera *editor)
{
	if (!editor)
	{
		return;
	}
	const Json ev = TryFind(root, "EditorView");
	if (ev.is_null() || !ev.is_object())
	{
		return;
	}
	Vector3 focal;
	float   dist, pitchDeg, yawDeg, fovDeg;
	editor->ExportOrbitSnapshot(focal, dist, pitchDeg, yawDeg, fovDeg);
	ReadVec3(TryFind(ev, "FocalPoint"), focal   );
	dist     = ev.value("Distance",     dist    );
	pitchDeg = ev.value("Pitch",        pitchDeg);
	yawDeg   = ev.value("Yaw",          yawDeg  );
	fovDeg   = ev.value("FOV",          fovDeg  );

	float zNear = ev.value("zNear", 0.1f);
	float zFar  = ev.value("zFar",  1000.0f);
	editor->ImportOrbitSnapshot(focal, dist, pitchDeg, yawDeg, fovDeg, zNear, zFar);
}

void SyncSceneCameraProjections(Scene *scene)
{
	const Vector2 &vp = scene->GetViewportSize();
	if (vp.x <= 0.0f || vp.y <= 0.0f)
	{
		return;
	}
	for (const auto entity : scene->Registry().view<CameraComponent>())
	{
		auto &cc = scene->Registry().get<CameraComponent>(entity);
		cc.Camera.SetViewportSize(vp.x, vp.y);
	}
}

} // namespace

SceneSerializer::SceneSerializer():
	ICLASS
{

}

void SceneSerializer::Serialize(Scene *scene, const std::string &path)
{
	using nlohmann::json;

	Stream stream{ path, Stream::Mode::Write };
	if (!stream.Writable())
	{
		LOG::INFO("Failed to serialize!");
		return;
	}

	json data;
	data["version"] = kFormatVersion;
	auto &objects  = data["Objects"];
	for (const auto primitive : scene->Registry().view<TagComponent>())
	{
		Object object{ primitive, scene };
		if (!object)
		{
			continue;
		}

		json objectData;
		SerializeTag(objectData, object.GetComponent<TagComponent>());

		if (object.HasComponent<TransformComponent>())
		{
			SerializeTransform(objectData, object.GetComponent<TransformComponent>());
		}
		if (object.HasComponent<IDComponent>())
		{
			SerializeId(objectData, object.GetComponent<IDComponent>());
		}
		if (object.HasComponent<LightComponent>())
		{
			SerializeLight(objectData, object.GetComponent<LightComponent>());
		}
		if (object.HasComponent<DirectionalLightComponent>())
		{
			SerializeDirectionalLight(objectData, object.GetComponent<DirectionalLightComponent>());
		}
		if (object.HasComponent<SpriteRendererComponent>())
		{
			SerializeSpriteRenderer(objectData, object.GetComponent<SpriteRendererComponent>());
		}
		if (object.HasComponent<ColorMixingComponent>())
		{
			SerializeColorMixing(objectData, object.GetComponent<ColorMixingComponent>());
		}
		if (object.HasComponent<MeshComponent>())
		{
			SerializeMesh(objectData, object.GetComponent<MeshComponent>(), object.GetComponent<TagComponent>().Tag);
		}
		if (object.HasComponent<MaterialComponent>())
		{
			SerializeMaterial(objectData, object.GetComponent<MaterialComponent>());
		}
		if (object.HasComponent<ScriptComponent>())
		{
			SerializeScript(objectData, object.GetComponent<ScriptComponent>());
		}
		if (object.HasComponent<VideoPlayerComponent>())
		{
			SerializeVideoPlayer(objectData, object.GetComponent<VideoPlayerComponent>());
		}
		if (object.HasComponent<CameraComponent>())
		{
			SerializeCamera(objectData, object.GetComponent<CameraComponent>());
		}

		objects.emplace_back(std::move(objectData));
	}

	EditorCamera *editorView = nullptr;
	if (auto *gs = dynamic_cast<GameScene *>(scene))
	{
		editorView = gs->IsEditorScene() ? &gs->GetEditorCamera() : nullptr;
	}
	SerializeEditorView(data, editorView);

	stream.Write(data.dump());
}

bool SceneSerializer::Deserialize(Scene *scene, const std::string &filepath)
{
	Json root;
	try
	{
		root = JSON{}.Parse(filepath);
	}
	catch (const std::exception &e)
	{
		CLOG_ERROR("Failed to load {} for {}", filepath, e.what());
		return false;
	}

	if (!root.is_object() || !root.contains("Objects") || !root["Objects"].is_array())
	{
		return false;
	}

	for (const auto &data : root["Objects"])
	{
		if (!data.is_object() || !data.contains("Name"))
		{
			continue;
		}
		Object object = scene->CreateObject(data["Name"].get<std::string>());

		DeserializeTransform(data, object.GetComponent<TransformComponent>());
		DeserializeId(data, object.GetComponent<IDComponent>());

		if (!TryFind(data, "Light").is_null())
		{
			DeserializeLight(data, object.AddComponent<LightComponent>());
		}
		if (!TryFind(data, "DirectionalLight").is_null())
		{
			DeserializeDirectionalLight(data, object.AddComponent<DirectionalLightComponent>());
		}
		DeserializeSpriteRenderer(object, data);
		if (!TryFind(data, "ColorMixing").is_null())
		{
			if (!object.HasComponent<ColorMixingComponent>())
			{
				object.AddComponent<ColorMixingComponent>();
			}
			DeserializeColorMixing(data, object.GetComponent<ColorMixingComponent>());
		}
		else if (object.HasComponent<ColorMixingComponent>())
		{
			DeserializeColorMixing(data, object.GetComponent<ColorMixingComponent>());
		}

		if (!TryFind(data, "Mesh").is_null())
		{
			auto &meshComponent = object.AddComponent<MeshComponent>();
			DeserializeMesh(data, meshComponent, data["Name"].get<std::string>());
		}
		if (!TryFind(data, "Material").is_null() && object.HasComponent<MeshComponent>())
		{
			auto &material = object.AddComponent<MaterialComponent>();
			DeserializeMaterial(data, material, object.GetComponent<MeshComponent>());
		}

		DeserializeScript(scene, object, data);
		DeserializeVideoPlayer(object, data);

		if (!TryFind(data, "Camera").is_null())
		{
			DeserializeCamera(data, object.AddComponent<CameraComponent>());
		}
	}

	EditorCamera *editorView = nullptr;
	if (auto *gs = dynamic_cast<GameScene *>(scene))
	{
		editorView = gs->IsEditorScene() ? &gs->GetEditorCamera() : nullptr;
	}
	DeserializeEditorView(root, editorView);
	SyncSceneCameraProjections(scene);

	return true;
}

}
