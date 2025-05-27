#pragma once
#include <string>
#include "TimelineTrack.h"
#include "Mapping.h"

namespace MappingsWindow {
	extern bool addingMapping;
	extern bool isTrigger;
	extern std::size_t audioIndex;
	extern std::shared_ptr<Mapping> selectedMapping;
	extern void showMappingsWindow(TimelineTrack* selectedTrack, const std::string& parameter, std::size_t p_index);
}