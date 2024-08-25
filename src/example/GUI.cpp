//
// Created by bzl on 23.08.2024.
//

#include "GUI.h"
#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_sdlrenderer2.h>

namespace BzlGame {

	GUI::GUI(Map *map, EditorState *state):  map_(map), state_(state) {
		panel_slider_ = state_->GetPanels().upper.h;
	}


	void GUI::Menubar() const {
		if(ImGui::BeginMenuBar()) {
		if (ImGui::BeginMenu("File"))
		{
			if(ImGui::MenuItem("Open.."))
			{
				map_->ClearMap();
				const int ntf_error = state_->OpenFileDialog();
				if(ntf_error == 1)
				{
					print("NTF ERROR 1");
					Event next_event = { .type = SDL_QUIT };
					SDL_PushEvent( &next_event );
				}
			}

			if(ImGui::MenuItem("Save")) {
				const int ntf_error = state_->SaveFileDialog();
				if(ntf_error == 1)
				{
					print("NTF ERROR 1");
					Event next_event = { .type = SDL_QUIT };
					SDL_PushEvent( &next_event );
				}
			}
			// if(ImGui::MenuItem("Save")){}
			ImGui::EndMenu();
		}
		ImGui::EndMenuBar();
	}
}



void GUI::RenderAll()
{
#ifdef BZ_IMGUI_ACTIVE
	const int MaxSize = state_->GetWindowSize().y;
	constexpr int MinSize = 0;

	ImGuiWindowFlags windowFlags = 0;
	windowFlags |= ImGuiWindowFlags_NoTitleBar;
	windowFlags |= ImGuiWindowFlags_MenuBar;
	//windowFlags |= ImGuiWindowFlags_NoCollapse;

	ImGui_ImplSDLRenderer2_NewFrame();
	ImGui_ImplSDL2_NewFrame();
	ImGui::NewFrame();
	//ImGui::ShowDemoWindow();
	//ImGui::ShowUserGuide();
	ImGui::Begin("bazpi tools", &imgui_window_active, windowFlags);

	if(imgui_window_active)
	{
		Menubar();

		if (ImGui::BeginListBox("Tilesets##setlistbox")) {
			for (size_t i = 0; i < map_->getTilesets().size(); ++i)
			{
				const bool is_selected = (i == state_->GetTilesetID()); // Beispiel: markiere das erste Element
				if (ImGui::Selectable(std::to_string(i).c_str(), is_selected))
				{
					state_->SetTileSetID(i);
				}
				if (is_selected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndListBox();
		}


		if(ImGui::Button("add##tileset")) {
			state_->OpenAssetFileDialog();
		};
		ImGui::SameLine();
		if(ImGui::Button("delete##tileset")) {
			map_->RemoveTileset(state_->GetTilesetID());
		}

		ImGui::Separator();

		// atlas panel slider
		ImGui::SliderInt("Atlas height##slider", &panel_slider_, MinSize, MaxSize);


		state_->SetPanelHeight(panel_slider_);

		ImGui::Checkbox("show/hide##atlas", &state_->refAtlasEnabled());
		ImGui::NewLine();

		// scaling slider
		int slider_zoom = state_->GetZoom();
		ImGui::SliderInt("Render scale##slider", &slider_zoom, 1, 3);
		state_->SetZoom(static_cast<u8>(slider_zoom));
		state_->SetScaler(slider_zoom * map_->tilesize());

		ImGui::NewLine();
		ImGui::Separator();

		if (ImGui::BeginCombo("Layer##Combo",std::to_string(state_->GetLayerID()).c_str())) {
			for (int i = 0; i < map_->getTileVec().size(); ++i)
			{
				const bool is_selected = (i == state_->GetLayerID());
				if (ImGui::Selectable(std::to_string(i).c_str(), is_selected))
				{
					state_->SetLayerID(i);
				}
			}
			ImGui::EndCombo();
		}

		//ImGui::NewLine();
		if(ImGui::Button("add##layer")) {
			map_->AddLayer();
			state_->SetLayerID(state_->GetLayerID()+1);
		}

		ImGui::SameLine();
		if(ImGui::Button("delete##layer" )) {
			map_->RemoveLayer(state_->GetLayerID());
			state_->SetLayerID(0);
		}
		ImGui::Separator();
		ImGui::NewLine();

		Point grid_mousepos = state_->GetGridMousepos();
		ImGui::Text("grid_mousepos: x: %d  y: %d", grid_mousepos.x, grid_mousepos.y);
		ImGui::Text("zoom: %d", state_->GetZoom());

		ImGui::NewLine();

		ImGui::Text("upper_panel:x %d / y %d | w %d h %d", state_->GetPanels().upper.x, state_->GetPanels().upper.y, state_->GetPanels().upper.w, state_->GetPanels().upper.h );
		ImGui::Text("lower_panel:x %d / y %d | w %d  h %d", state_->GetPanels().lower.x, state_->GetPanels().lower.y, state_->GetPanels().lower.w, state_->GetPanels().lower.h);

		ImGui::NewLine();

		Point windowsize = state_->GetWindowSize();
		ImGui::Text("WindowSize: w %d  h %d", windowsize.x, windowsize.y);

		// ImGui::NewLine();
		// if (ImGui::Button("save map")) {
		// 	SaveFileDialog();
		// }

		ImGui::End();
		ImGui::Render();

		SDL_SetRenderDrawColor(state_->GetRenderer(), static_cast <u8>(0.45f * 255), static_cast <u8>(0.55f * 255), static_cast <u8>(0.60f * 255), static_cast <u8>(1.00f * 255));

		ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData());
	}
#endif
}
}
