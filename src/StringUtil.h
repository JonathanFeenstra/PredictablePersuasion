#pragma once

#include <string_view>

namespace StringUtil
{
	bool LowerCaseContains(const std::string_view a_hayStack, const std::string_view a_lowerCaseNeedle) noexcept;
}