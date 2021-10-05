#pragma once

#include "Immortal.h"

namespace Immortal {

	class SceneHierarchyPanel
	{
	public:
		SceneHierarchyPanel();
		SceneHierarchyPanel(const std::shared_ptr<Scene> &scene);
		~SceneHierarchyPanel();

		void SetContext(const std::shared_ptr<Scene> &scene);

		void OnGuiRender();

		Entity &SelectedEntity() { return mSelectedEntity; }
		const Entity &SelectedEntity() const { return mSelectedEntity; }

	private:
		void DrawEntityNode(Entity &e);
		void DrawComponents(Entity &e);
	private:
		std::shared_ptr<Scene> mContext;
		Entity mSelectedEntity;
	};

}

