#pragma once
#include "Immortal.h"

#include "Panel/SceneHierarchyPanel.h"

namespace Immortal {

	class EditorLayer : public Layer
	{
	public:
		EditorLayer();
		virtual ~EditorLayer() = default;

		virtual void OnAttach() override;
		virtual void OnDetach() override;

		void OnUpdate() override;
		virtual void OnGuiRender() override;
		void OnEvent(Event& e) override;

	public:
		bool OnResize(WindowResizeEvent &e);
		bool OnKeyPressed(KeyPressedEvent &e);
		bool OnMouseButtonPressed(MouseButtonPressedEvent &e);

	private:
		void OnPlay();
		void OnStop();

	private:
		bool NewScene();
		bool LoadScene();
		bool SaveSceneAs();
		bool LoadObject(const Ref<Scene> &scene);

	private:
		enum class SceneState
		{
			Edit,
			Play,
			Pause
		};
		SceneState mState = SceneState::Edit;
		EditorCamera mEditorCamera;

	private:
		Immortal::OrthographicCameraController mCameraController;

		Ref<VertexArray> mSquareVA;
		Ref<Shader> mFlatColorShader;
		Ref<Framebuffer> mFramebuffer;
		Ref<Texture2D> mTexture;

		float mBrightness{ 0 };
		float mLuminance{ 0 };
		float mSaturation{ 0 };
		float mHue{ 0 };
		Vector::Vector4 mRGBA{ 0.0f, 0.0f, 0.0f, 0.0f };


		Vector::Vector2 mViewportSize{ 1280.0f, 720.0f };
		Vector::Vector2 mViewportBounds[2];

		bool mViewportFocused{ false };
		bool mViewportHovered{ false };

		Ref<Scene> mActiveScene;
		std::vector<Entity> mImageLayer;

		Entity mCameraObject;
		Entity mCameraObject2;
		bool mPrimaryCamera = true;

		SceneHierarchyPanel mSceneHierarchyPanel;

		Ref<Texture2D> mPlayButtonTexture;
		Ref<Texture2D> mPauseButtonTexture;
		Ref<Texture2D> mStopButtonTexture;

		int mGuizmoType = ImGuizmo::OPERATION::INVALID;

	/* WorkSpace Related */
	public:
		static const std::string &WorkSpace() NOEXCEPT { return sWorkspace; }

	private:
		static std::string sWorkspace;
	};

    struct AppLog
    {
        ImGuiTextBuffer     Buf;
        ImGuiTextFilter     Filter;
        ImVector<int>       LineOffsets; // Index to lines offset. We maintain this with AddLog() calls.
        bool                AutoScroll;  // Keep scrolling if already at the bottom.

        AppLog()
        {
            AutoScroll = true;
            Clear();
        }

        void    Clear()
        {
            Buf.clear();
            LineOffsets.clear();
            LineOffsets.push_back(0);
        }

        void    AddLog(const char* fmt, ...) IM_FMTARGS(2)
        {
            int old_size = Buf.size();
            va_list args;
            va_start(args, fmt);
            Buf.appendfv(fmt, args);
            va_end(args);
            for (int new_size = Buf.size(); old_size < new_size; old_size++)
                if (Buf[old_size] == '\n')
                    LineOffsets.push_back(old_size + 1);
        }

        void    Draw(const char* title, bool* p_open = NULL)
        {
            if (!ImGui::Begin(title, p_open))
            {
                ImGui::End();
                return;
            }

            // Options menu
            if (ImGui::BeginPopup(U8("选项")))
            {
                ImGui::Checkbox(U8("自动滚动"), &AutoScroll);
                ImGui::EndPopup();
            }

            // Main window
            if (ImGui::Button(U8("选项")))
                ImGui::OpenPopup(U8("选项"));
            ImGui::SameLine();
            bool clear = ImGui::Button(U8("清理"));
            ImGui::SameLine();
            bool copy = ImGui::Button(U8("复制"));
            ImGui::SameLine();
            Filter.Draw(U8("过滤"), -100.0f);

            ImGui::Separator();
            ImGui::BeginChild("scrolling", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

            if (clear)
                Clear();
            if (copy)
                ImGui::LogToClipboard();

            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
            const char* buf = Buf.begin();
            const char* buf_end = Buf.end();
            if (Filter.IsActive())
            {
                // In this example we don't use the clipper when Filter is enabled.
                // This is because we don't have a random access on the result on our filter.
                // A real application processing logs with ten of thousands of entries may want to store the result of
                // search/filter.. especially if the filtering function is not trivial (e.g. reg-exp).
                for (int line_no = 0; line_no < LineOffsets.Size; line_no++)
                {
                    const char* line_start = buf + LineOffsets[line_no];
                    const char* line_end = (line_no + 1 < LineOffsets.Size) ? (buf + LineOffsets[line_no + 1] - 1) : buf_end;
                    if (Filter.PassFilter(line_start, line_end))
                        ImGui::TextUnformatted(line_start, line_end);
                }
            }
            else
            {
                // The simplest and easy way to display the entire buffer:
                //   ImGui::TextUnformatted(buf_begin, buf_end);
                // And it'll just work. TextUnformatted() has specialization for large blob of text and will fast-forward
                // to skip non-visible lines. Here we instead demonstrate using the clipper to only process lines that are
                // within the visible area.
                // If you have tens of thousands of items and their processing cost is non-negligible, coarse clipping them
                // on your side is recommended. Using ImGuiListClipper requires
                // - A) random access into your data
                // - B) items all being the  same height,
                // both of which we can handle since we an array pointing to the beginning of each line of text.
                // When using the filter (in the block of code above) we don't have random access into the data to display
                // anymore, which is why we don't use the clipper. Storing or skimming through the search result would make
                // it possible (and would be recommended if you want to search through tens of thousands of entries).
                ImGuiListClipper clipper;
                clipper.Begin(LineOffsets.Size);
                while (clipper.Step())
                {
                    for (int line_no = clipper.DisplayStart; line_no < clipper.DisplayEnd; line_no++)
                    {
                        const char* line_start = buf + LineOffsets[line_no];
                        const char* line_end = (line_no + 1 < LineOffsets.Size) ? (buf + LineOffsets[line_no + 1] - 1) : buf_end;
                        ImGui::TextUnformatted(line_start, line_end);
                    }
                }
                clipper.End();
            }
            ImGui::PopStyleVar();

            if (AutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
                ImGui::SetScrollHereY(1.0f);

            ImGui::EndChild();
            ImGui::End();
        }
    };
}
