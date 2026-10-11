// Adapted from: https://github.com/Seb263/SkyrimSE_SmartTalk/blob/master/src/SettingsIni.hpp
#pragma once

#include <filesystem>
#include <string>

#include <SKSE/Logger.h>
#include <SimpleIni.h>

namespace SmartTalkSettings
{
	inline std::string sQuestUISetEntryTextCustomFunction = "SetEntryTextCustom";

	static bool ReadSettings()
	{
		const std::string path = "Data/SKSE/Plugins/SmartTalk.ini";
		const std::string pathCustomUI = "Data/SKSE/Plugins/SmartTalk_CustomUI.ini";

		std::wstring wpath_str(path.begin(), path.end());
		const wchar_t* wpath = wpath_str.c_str();

		bool readStatus = false;

		logger::info("Trying to read INI file at path: {}", path);

		if (std::filesystem::exists(path)) {
			CSimpleIniA ini;
			ini.SetUnicode();

			if (ini.LoadFile(wpath) >= 0) {
				sQuestUISetEntryTextCustomFunction = ini.GetValue("QuestUI", "sSetEntryTextCustomFunction", sQuestUISetEntryTextCustomFunction.c_str());
				readStatus = true;
			} else {
				logger::error("Failed to load INI file at: {}", path);
			}
		} else {
			logger::warn("INI file does not exist at {}", path);
		}

		if (std::filesystem::exists(pathCustomUI)) {
			logger::info("Custom UI INI found at {}, overriding QuestUI settings", pathCustomUI);

			std::wstring wpathCustom_str(pathCustomUI.begin(), pathCustomUI.end());
			const wchar_t* wpathCustom = wpathCustom_str.c_str();
			CSimpleIniA iniCustom;
			iniCustom.SetUnicode();

			if (iniCustom.LoadFile(wpathCustom) >= 0) {
				sQuestUISetEntryTextCustomFunction = iniCustom.GetValue("QuestUI", "sSetEntryTextCustomFunction", sQuestUISetEntryTextCustomFunction.c_str());
			} else {
				logger::error("Failed to load Custom UI INI at {}", pathCustomUI);
			}
		}

		return readStatus;
	}
}
