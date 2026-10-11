/*
Copyright (C) 2025-2026 Jonathan Feenstra

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.

See EXCEPTIONS for additional permissions.
*/

#include "Events.h"

#include "Requirements.h"
#include "Scaleform.h"
#include "SmartTalkSettings.hpp"

#include <RE/B/BSTEvent.h>
#include <RE/D/DialogueMenu.h>
#include <RE/M/MenuOpenCloseEvent.h>
#include <RE/U/UI.h>
#include <SKSE/Logger.h>

#include <filesystem>
#include <string>
#include <unordered_map>

namespace Events
{
	void MenuOpenCloseEventSink::Install(const std::unordered_map<std::string, Scaleform::TopicDisplayData>* a_topicDisplayData) noexcept
	{
		const auto singleton = GetSingleton();
		singleton->topicDisplayData = a_topicDisplayData;

		if (std::filesystem::exists("Data/SKSE/Plugins/SmartTalk.dll")) {
			if (!SmartTalkSettings::ReadSettings()) {
				logger::warn("Failed to load Smart Talk settings file. Default settings will be used.");
			}

			singleton->smartTalkSetEntryTextCustomFunctionEnabled = !SmartTalkSettings::sQuestUISetEntryTextCustomFunction.empty();
		} else {
			singleton->smartTalkSetEntryTextCustomFunctionEnabled = false;
		}

		if (const auto ui = RE::UI::GetSingleton()) {
			ui->AddEventSink(singleton);
		}
	}

	MenuOpenCloseEventSink* MenuOpenCloseEventSink::GetSingleton() noexcept
	{
		static MenuOpenCloseEventSink singleton;
		return &singleton;
	}

	RE::BSEventNotifyControl MenuOpenCloseEventSink::ProcessEvent(const RE::MenuOpenCloseEvent* a_event, RE::BSTEventSource<RE::MenuOpenCloseEvent>*)
	{
		if (a_event->menuName == RE::DialogueMenu::MENU_NAME && a_event->opening && Requirements::AreRequirementsMet()) {
			Scaleform::InstallHooks(topicDisplayData, smartTalkSetEntryTextCustomFunctionEnabled);
		}
		return RE::BSEventNotifyControl::kContinue;
	}
}