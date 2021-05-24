#pragma once

#include "Immortal.h"

namespace Immortal {

	class SceneHierarchyPanel
	{
	public:
		SceneHierarchyPanel();
		SceneHierarchyPanel(const Ref<Scene> &scene);
		~SceneHierarchyPanel();

		void SetContext(const Ref<Scene> &scene);

		void OnGuiRender();

		Entity &SelectedEntity() { return mSelectedEntity; }
		const Entity &SelectedEntity() const { return mSelectedEntity; }

	private:
		void DrawEntityNode(Entity &e);
		void DrawComponents(Entity &e);
	private:
		Ref<Scene> mContext;
		Entity mSelectedEntity;
	};

}

