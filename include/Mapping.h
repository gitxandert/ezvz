#pragma once
#include <algorithm>
#include "GraphicObject.h"
#include "Canvas.h"
#include "imgui.h"

enum class AudioParameter : std::size_t {
	Envelope = 0,
	ZCR,
	SpCentroid,
	SpFlatness,
	SpRolloff,
	SpContrast,
	SpBandwidth,
	SpEntropy,
	SpFlux,
	SpSkewness,
	SpKurtosis,
	COUNT
};

const std::string AudioParameterNames[] = {
	"Envelope",
	"Zero Crossing Rate",
	"Spectral Centroid",
	"Spectral Flatness",
	"Spectral Rolloff",
	"Spectral Contrast",
	"Spectral Bandwidth",
	"Spectral Entropy",
	"Spectral Flux",
	"Spectral Skewness",
	"Spectral Kurtosis"
};

const enum class MapType : int {
	Sync = 0,
	Trigger = 1,
	COUNT
};

const std::string mapTypeNames[] = {
	"Sync",
	"Trigger"
};

namespace MappingRanges {
	const glm::vec2 inputRanges[] = {
		{ 0.0f, 1.0f } //Envelope
		, { 0.0f, 1.0f } //ZCR
	};

	inline const glm::vec2 outputRanges(GraphicParameter px, bool py, float& input_drag_speed_, float& output_drag_speed_) {
		switch (px) {
		case GraphicParameter::Position: { //Position
			input_drag_speed_ = 0.01f;

			if (!py) {
				output_drag_speed_ = (Canvas::screenW * 3.0f / 100.0f);
				return { -Canvas::screenW, Canvas::screenW * 2.0f };
			}
			else {
				output_drag_speed_ = (Canvas::screenH * 3.0f / 100.0f);
				return { -Canvas::screenH, Canvas::screenH * 2.0f };
			}
		}
		case GraphicParameter::Rotation: {//Rotation
			input_drag_speed_ = 0.01f;
			output_drag_speed_ = 72.0f;
			return { -360.0f, 360.0f };
		}
		case GraphicParameter::Size: { //Size
			input_drag_speed_ = 0.01f;

			if (!py) {
				output_drag_speed_ = Canvas::screenW / 100.0f;
				return { 0.0f, Canvas::screenW };
			}
			else {
				output_drag_speed_ = Canvas::screenH / 100.0f;
				return { 0.0f, Canvas::screenH };
			}
		}
		case GraphicParameter::Hue_Sat: [[fallthrough]];
		case GraphicParameter::Brightness: [[fallthrough]];
		case GraphicParameter::Alpha: {//Color
			input_drag_speed_ = 0.01f;
			output_drag_speed_ = 0.01f;
			return { 0.0f, 1.0f };
		}
		default: return { 0.0f, 1.0f };
		}
	}
}

class Mapping {
public:
	Mapping(std::shared_ptr<GraphicObject> obj, AudioParameter ap, GraphicParameter gp, MapType mp, bool gpy=false)
		: mapped_object(obj)
		, a_param_(ap)
		, g_param_(gp)
		, map_type_(mp)
		, g_param_y_(gpy)
	{
	}

	~Mapping() = default;

	const MapType& getMapType() const { return map_type_; }
	const std::string& getMapTypeName() const { return mapTypeNames[static_cast<int>(map_type_)]; }

	virtual void showMappingParametersUI() = 0;
	virtual void mapParameter(float) = 0;

	void updateMappedObject(float value) {
		if (auto obj = mapped_object.lock()) {
			switch (g_param_) {
			case GraphicParameter::Position: [[fallthrough]];
			case GraphicParameter::Size: [[fallthrough]];
			case GraphicParameter::Hue_Sat: {
				glm::vec2 param_value = obj->getParameterValue(static_cast<int>(g_param_));
				if (!g_param_y_)
					param_value.x = value;
				else
					param_value.y = value;
				obj->setParameter(static_cast<int>(g_param_), param_value);
				break;
			}
			default: {
				obj->setParameter(static_cast<int>(g_param_), { value, NULL });
				break;
			}
			}

			obj->setNewMapBools(static_cast<int>(g_param_), g_param_y_);
		}
	}

	const bool& getGParamY() const { return g_param_y_; }
	const AudioParameter& getAudioParameter() const { return a_param_; }
	const GraphicParameter& getGraphicParameter() const { return g_param_; }
	std::shared_ptr<GraphicObject> getMappedObject() const {
		return mapped_object.lock();
	}

	bool isMapped() const { return !mapped_object.expired(); }

private:
	std::weak_ptr<GraphicObject> mapped_object;
	AudioParameter a_param_;
	GraphicParameter g_param_;
	bool g_param_y_ = false;

	MapType map_type_;
};

class SyncMapping : public Mapping {
public:
	SyncMapping(std::shared_ptr<GraphicObject> obj, AudioParameter ap, GraphicParameter gp, MapType mp, bool gpy=false)
		: Mapping(obj, ap, gp, mp, gpy)
	{
		input_range_ = MappingRanges::inputRanges[static_cast<int>(ap)];
		map_input_ = input_range_;
		output_range_ = MappingRanges::outputRanges(gp, gpy, input_drag_speed_, output_drag_speed_);
		map_output_ = output_range_;
	}

	void showMappingParametersUI() override
	{
		ImGui::SetNextItemWidth(100.0f);
		if (ImGui::DragFloat("##InputRange", &map_input_.x, input_drag_speed_, input_range_.x, input_range_.y)) {
			if (map_input_.x > input_range_.y) {
				map_input_.x = input_range_.y;
			}
			else if (map_input_.x > map_input_.y) {
				map_input_.x = map_input_.y;
			}
		}
		ImGui::SameLine();
		ImGui::SetNextItemWidth(100.0f);
		if (ImGui::DragFloat("Input", &map_input_.y, input_drag_speed_, input_range_.x, input_range_.y)) {
			if (map_input_.y > input_range_.y) {
				map_input_.y = input_range_.y;
			}
			else if (map_input_.y < map_input_.x) {
				map_input_.y = map_input_.x;
			}
		}

		ImGui::SetNextItemWidth(100.0f);
		ImGui::DragFloat("##OutputRange", &map_output_.x, output_drag_speed_, output_range_.x, output_range_.y);
		ImGui::SameLine();
		ImGui::SetNextItemWidth(100.0f);
		ImGui::DragFloat("Output", &map_output_.y, output_drag_speed_, output_range_.x, output_range_.y);
		if (map_output_.x < output_range_.x) {
			map_output_.x = output_range_.x;
		}
		else if (map_output_.x > output_range_.y) {
			map_output_.x = output_range_.y;
		}

		if(map_output_.y < output_range_.x) {
			map_output_.y = output_range_.x;
		}
		else if (map_output_.y > output_range_.y) {
			map_output_.y = output_range_.y;
		}
	}

	float convertValue(float value) const {
		if(map_input_.y - map_input_.x == 0.0f) {
			return map_output_.y; // Avoid division by zero
		}
		float percent = (value - map_input_.x) / (map_input_.y - map_input_.x);
		return map_output_.x + percent * (map_output_.y - map_output_.x);
	}

	void mapParameter(float value) override {
		float min_input_ = std::min(map_input_.x, map_input_.y);
		float max_input_ = std::max(map_input_.x, map_input_.y);
		if (value >= min_input_ && value <= max_input_) {
			float converted_value = convertValue(value);
			updateMappedObject(converted_value);
		}
	}

private:
	glm::vec2 input_range_{};
	glm::vec2 output_range_{};
	float input_drag_speed_ = 0.0f;
	float output_drag_speed_ = 0.0f;
	glm::vec2 map_input_{};
	glm::vec2 map_output_{};
};

class TriggerMapping : public Mapping {
public:
	TriggerMapping(std::shared_ptr<GraphicObject> obj, AudioParameter ap, GraphicParameter gp, std::size_t animation_index, MapType mp)
		: Mapping(obj, ap, gp, mp)
		, animation_index_(animation_index)
	{
		input_range_ = MappingRanges::inputRanges[static_cast<int>(ap)];
		threshold_ = input_range_.x;
	}

	void mapParameter(float value) override {
		bool shouldTrigger = false;
		if (isGreaterThan_) {
			if (value >= threshold_)
				shouldTrigger = true;
		}
		else {
			if (value <= threshold_)
				shouldTrigger = true;
		}

		if (shouldTrigger) {
			if (!hasReachedThreshold_) {
				hasReachedThreshold_ = true;
				if (auto obj = getMappedObject()) {
					std::cout << "animation_index_ = " << animation_index_ << '\n';
					obj->getAnimations(static_cast<std::size_t>(getGraphicParameter()))[animation_index_]->trigger();
				}
			}
		}
		else {
			hasReachedThreshold_ = false;
		}
	};

	void showMappingParametersUI() override
	{
		ImGui::Indent(10.0f);
		ImGui::Text("Threshold");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(100.0f);
		ImGui::DragFloat("##Threshold", &threshold_, input_drag_speed_, input_range_.y / 1000.0f, input_range_.y);

		ImGui::SameLine();
		const char* greater_than_label = isGreaterThan_ ? ">" : "<";
		if(ImGui::Button(greater_than_label)) {
			isGreaterThan_ = !isGreaterThan_;
		}
		ImGui::Unindent(10.0f);
	}

private:
	std::size_t animation_index_ = 0;
	glm::vec2 input_range_{};
	float input_drag_speed_ = 0.01f;
	float threshold_ = 0.0f;
	bool isGreaterThan_ = true;
	bool hasReachedThreshold_ = false;

};