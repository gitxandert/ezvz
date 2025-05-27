#pragma once
#include <string>
#include "TimelineTrack.h"

namespace MappingsWindow {
	extern bool addingMapping;
	extern bool isTrigger;
	extern std::size_t audioIndex;
	extern Mapping* selectedMapping;
	extern void showMappingsWindow(TimelineTrack* selectedTrack, const std::string& parameter, std::size_t p_index);
}