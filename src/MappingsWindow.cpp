#define _USE_MATH_DEFINES
#include <cmath>
#include "Canvas.h"
#include "MappingsWindow.h"
#include "ScenesPanel.h"
#include "Timeline.h"

namespace MappingsWindow {

	bool addingMapping = false;
	bool isTrigger = false;
	std::size_t audioIndex = 0;
	static const float blinkDuration = 1.0f; // Duration of one blink cycle (in seconds)
	static float blinkAlpha = 1.0f; // Alpha value for blinking effect

	static bool isMappingSelected = false; // Flag to track if a mapping is selected
	static int selectedMappingIndex = -1; // Index of the selected mapping
	std::shared_ptr<Mapping> selectedMapping = nullptr; // Pointer to the selected mapping

	void renderMappingsPopup() {
		if (ImGui::BeginPopup("MappingsPopUp")) {
			if (ImGui::Selectable("Sync")) {
				addingMapping = true;
				isTrigger = false;
				ScenesPanel::mappingIndex = -1;
			}

			if (ImGui::Selectable("Trigger")) {
				addingMapping = true;
				isTrigger = true;
			}

			ImGui::EndPopup();
		}
	}

	void showMappingsWindow(TimelineTrack* selectedTrack, const std::string& parameter, std::size_t p_index) {

		audioIndex = p_index;

		ImGuiIO& io = ImGui::GetIO();

		float scale = (isTrigger && ScenesPanel::showAnimateWindow) ? 0.5f : 1.0f;
		float space = (isTrigger && ScenesPanel::showAnimateWindow) ? 5.0f : 0.0f;

		ImGui::SetNextWindowPos(ImVec2(0, io.DisplaySize.y - Timeline::timelineFixedHeight + 5), ImGuiCond_Always);
		ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x * scale - space, Timeline::timelineFixedHeight - 5), ImGuiCond_Always);
		ImGui::SetNextWindowBgAlpha(1.0f);
		ImGui::Begin((parameter + " Mappings").c_str(), nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);

        const char* addMapBtnLabel = addingMapping ? "Adding Mapping" : "Add Mapping";

		ImGui::PushStyleVar(ImGuiStyleVar_Alpha, blinkAlpha);

        if (ImGui::Button(addMapBtnLabel)) {
			if(!addingMapping)
				ImGui::OpenPopup("MappingsPopUp");
			else {
				addingMapping = false;
				isTrigger = false;
				ScenesPanel::showAnimateWindow = false;
			}
        }

		ImGui::PopStyleVar(); // Restore button alpha

		renderMappingsPopup();

		ImGui::NewLine();

		if(selectedTrack->mappings[p_index].size() > 0){
			std::vector<std::shared_ptr<Mapping>>& cur_mappings = selectedTrack->mappings[p_index];
			for (std::size_t i = 0; i < cur_mappings.size(); ++i) {
				ImGui::SameLine();
				bool is_selected = (i == selectedMappingIndex);
				std::string label = "Mapping " + std::to_string(i + 1);
				if (ImGui::Selectable(label.c_str(), is_selected, 0, {100, 0})) {
					isMappingSelected = true;
					selectedMappingIndex = i; 
					selectedMapping = cur_mappings[i];
					ScenesPanel::mappingIndex = static_cast<int>(selectedMapping->getGraphicParameter());
				}
			}
		} else {
			ImGui::Text("No mappings for %s.", parameter.c_str());
		}

		ImGui::Separator();

		if (isMappingSelected) {
			ImGui::Indent(10.0f);
			const std::string& mapTypeName = selectedTrack->mappings[p_index][selectedMappingIndex]->getMapTypeName();
			const std::string& objectName = selectedTrack->mappings[p_index][selectedMappingIndex]->getMappedObject()->getId();
			const std::string& parameterName = ScenesPanel::parameters[static_cast<std::size_t>(selectedTrack->mappings[p_index][selectedMappingIndex]->getGraphicParameter())];
			ImGui::Text((mapTypeName + " : " + objectName + " -> " + parameterName).c_str());
			selectedTrack->mappings[p_index][selectedMappingIndex]->showMappingParametersUI();
			ImGui::Unindent(10.0f);
		}

		if (addingMapping) {
			// Calculate blink alpha (0.0 = transparent, 1.0 = opaque)
			float t = ImGui::GetTime();  // seconds since app start
			blinkAlpha = 0.5f + 0.5f * sinf(t / blinkDuration * 2.0f * M_PI);

			if (Canvas::selectedObject) {
				if (!ScenesPanel::showAnimateWindow)
					ImGui::Text("Select a parameter.");
				else
					ImGui::Text("Select an animation.");
			}
			else {
				ImGui::Text("Select an object to map to.");
			}
		}
		else {
			blinkAlpha = 1.0f; // Reset blink alpha when not adding a mapping
		}

		ImGui::End();
	}
}