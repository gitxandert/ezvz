#include "imgui.h"
#include <vector>


inline ImVec4 CLEAR_COL{ 0.1f, 0.1f, 0.1f, 1.0f };

enum class StyleType {
	Volcano,
	Ocean
};

enum class StyleColor {
	DarkGray,
	Gray,
	LightGray,
	DarkSand,
	Sand,
	LightSand,
	LightRed,
	Red,
	DarkRed, 
	LightOcean,
	Ocean,
	DarkOcean,
	White,
	Black
};

inline ImVec4 getColor(StyleColor color) {
	switch (color) {
	// Volcano
	case StyleColor::LightGray:
		return ImVec4(0.50f, 0.50f, 0.50f, 1.00f);

	case StyleColor::Gray:
		return ImVec4(0.35f, 0.35f, 0.35f, 1.00f);

	case StyleColor::DarkGray:
		return ImVec4(0.20f, 0.20f, 0.20f, 1.00f);

	case StyleColor::LightRed:
		return ImVec4(0.90f, 0.30f, 0.30f, 1.00f);

	case StyleColor::Red:
		return ImVec4(0.80f, 0.20f, 0.20f, 1.00f);

	case StyleColor::DarkRed:
		return ImVec4(0.50f, 0.10f, 0.10f, 1.00f);

	// Ocean
	case StyleColor::LightSand:   return ImVec4(0.83f, 0.73f, 0.53f, 1.00f);  // sun-bleached beach  
	case StyleColor::Sand:        return ImVec4(0.73f, 0.63f, 0.40f, 1.00f);  // classic sand  
	case StyleColor::DarkSand:    return ImVec4(0.60f, 0.50f, 0.35f, 1.00f);  // damp/driftwood  

	case StyleColor::LightOcean:  return ImVec4(0.50f, 0.75f, 0.85f, 1.00f);  // shallow, sunlit water  
	case StyleColor::Ocean:       return ImVec4(0.25f, 0.55f, 0.70f, 1.00f);  // mid-depth  
	case StyleColor::DarkOcean:   return ImVec4(0.10f, 0.35f, 0.50f, 1.00f);  // deeper blue-green  

	case StyleColor::White:
		return ImVec4(1.00f, 1.00f, 1.00f, 1.00f);

	case StyleColor::Black:
		return ImVec4(0.00f, 0.00f, 0.00f, 1.00f);

	default:
		return ImVec4(1.00f, 1.00f, 1.00f, 1.00f); // Default to white
	}
}

inline void setStyle(StyleType colorStyle = StyleType::Volcano) {
	ImGuiStyle& style = ImGui::GetStyle();

	style.WindowRounding = 4.0f;
	style.TabRounding = 4.0f;
	style.FramePadding = ImVec2(8, 6);

	ImVec4 c_dark, c_norm, c_light, bw_dark, bw_norm, bw_light, textCol;

	switch (colorStyle) {
	case StyleType::Volcano: {
		c_dark = getColor(StyleColor::DarkRed);
		c_norm = getColor(StyleColor::Red);
		c_light = getColor(StyleColor::LightRed);

		bw_dark = getColor(StyleColor::DarkGray);
		bw_norm = getColor(StyleColor::Gray);
		bw_light = getColor(StyleColor::LightGray);

		style.Colors[ImGuiCol_MenuBarBg] = bw_norm;

		CLEAR_COL = bw_norm;

		textCol = ImVec4(1.00f, 0.70f, 0.40f, 1.00f);

		break;
	}
	case StyleType::Ocean: {
		c_dark = getColor(StyleColor::DarkOcean);
		c_norm = getColor(StyleColor::Ocean);
		c_light = getColor(StyleColor::LightOcean);

		bw_dark = getColor(StyleColor::DarkSand);
		bw_norm = getColor(StyleColor::Sand);
		bw_light = getColor(StyleColor::LightSand);

		style.Colors[ImGuiCol_MenuBarBg] = ImVec4(bw_dark.x * 0.80f, bw_dark.y * 0.80f, bw_dark.z * 0.80f, bw_dark.w * 0.80f);

		CLEAR_COL = ImVec4(bw_dark.x * 0.80f, bw_dark.y * 0.80f, bw_dark.z * 0.80f, bw_dark.w * 0.80f);

		textCol = ImVec4(0.00f, 1.00f, 1.00f, 1.0f);

		break;
	}
	default: {
		c_dark = getColor(StyleColor::DarkRed);
		c_norm = getColor(StyleColor::Red);
		c_light = getColor(StyleColor::LightRed);

		bw_dark = getColor(StyleColor::DarkGray);
		bw_norm = getColor(StyleColor::Gray);
		bw_light = getColor(StyleColor::LightGray);

		break;
	}
	}

	// Separator
	style.Colors[ImGuiCol_Separator] = c_norm;

	// Border
	style.Colors[ImGuiCol_Border] = ImVec4(c_norm.x, c_norm.y, c_norm.z, c_norm.w * 0.50f);

	// Scrollbar
	style.Colors[ImGuiCol_ScrollbarBg] = c_dark;
	style.Colors[ImGuiCol_ScrollbarGrab] = c_norm;
	style.Colors[ImGuiCol_ScrollbarGrabActive] = c_light;
	style.Colors[ImGuiCol_ScrollbarGrabHovered] = c_light;

	// Window
	style.Colors[ImGuiCol_WindowBg] = bw_dark;

	// Text
	style.Colors[ImGuiCol_Text] = textCol;

	// Title
	style.Colors[ImGuiCol_TitleBg] = c_dark;
	style.Colors[ImGuiCol_TitleBgActive] = c_norm;
	style.Colors[ImGuiCol_TitleBgCollapsed] = c_dark;

	// Popup
	style.Colors[ImGuiCol_PopupBg] = c_norm;

	// Selectable
	style.Colors[ImGuiCol_Header] = c_dark;
	style.Colors[ImGuiCol_HeaderActive] = c_norm;
	style.Colors[ImGuiCol_HeaderHovered] = c_light;

	// Frame
	style.Colors[ImGuiCol_FrameBg] = bw_norm;
	style.Colors[ImGuiCol_FrameBgHovered] = bw_light;
	style.Colors[ImGuiCol_FrameBgActive] = bw_light;

	// Button
	style.Colors[ImGuiCol_Button] = c_dark;
	style.Colors[ImGuiCol_ButtonHovered] = c_light;
	style.Colors[ImGuiCol_ButtonActive] = c_norm;

	// Slider
	style.Colors[ImGuiCol_SliderGrab] = c_dark;
	style.Colors[ImGuiCol_SliderGrabActive] = c_norm;

}