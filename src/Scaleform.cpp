/*
Copyright (C) 2025 Jonathan Feenstra

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

#include "Scaleform.h"

#include "Settings.h"

namespace Scaleform
{
	void InstallHooks(const std::unordered_map<std::string, TopicDisplayData>* a_topicDisplayData) noexcept
	{
		const auto ui = RE::UI::GetSingleton();
		if (!ui) {
			logger::error("Failed to get UI");
			return;
		}

		const auto dialogueMenu = ui->GetMenu<RE::DialogueMenu>().get();
		if (!dialogueMenu) {
			logger::error("Failed to get DialogueMenu");
			return;
		}

		RE::GFxValue topicList;
		if (!dialogueMenu->uiMovie->GetVariable(&topicList, "_root.DialogueMenu_mc.TopicListHolder.List_mc")) {
			logger::error("Failed to get TopicList");
			return;
		}

		if (Settings::applyTopicColors) {
			SetEntryTextFunctionHandler::Install(dialogueMenu, topicList, a_topicDisplayData);
		}

		if (Settings::showSubtitles != Settings::SHOW_SUBTITLES::kNever) {
			DoSetSelectedIndexFunctionHandler::Install(dialogueMenu, topicList, a_topicDisplayData);
		}
	}

	void SetEntryTextFunctionHandler::Install(
		const RE::DialogueMenu* a_dialogueMenu,
		RE::GFxValue a_topicList,
		const std::unordered_map<std::string, TopicDisplayData>* a_topicDisplayData) noexcept
	{
		auto handler = RE::make_gptr<Scaleform::SetEntryTextFunctionHandler>();
		handler->topicDisplayData = a_topicDisplayData;

		RE::GFxValue setEntryText;
		a_dialogueMenu->uiMovie->CreateFunction(&setEntryText, handler.get());
		a_topicList.SetMember("SetEntryText", setEntryText);
	}

	// replaces: https://github.com/Mardoxx/skyrimui/blob/425aa8a31de31fb11fe78ee6cec799f4ba31af03/src/dialoguemenu/DialogueCenteredList.as#L23-L29
	void SetEntryTextFunctionHandler::Call(Params& a_params)
	{
		if (a_params.argCount < 2) {
			logger::error("SetEntry: Expected 2 arguments, found {}", a_params.argCount);
			return;
		}

		auto thisPtr = a_params.thisPtr;
		auto& aEntryClip = a_params.args[0];
		auto& aEntryObject = a_params.args[1];

		// custom implementation of BSScrollingList.SetEntryText
		RE::GFxValue textField;
		if (!aEntryClip.GetMember("textField", &textField) || textField.IsUndefined())
			return;

		RE::GFxValue textOption;
		thisPtr->Invoke("__get__textOption", &textOption);
		switch (textOption.GetUInt()) {
		case 1:
			// Shared.BSScrollingList.TEXT_OPTION_SHRINK_TO_FIT
			textField.SetMember("textAutoSize", RE::GFxValue("shrink"));
			break;
		case 2:
			// Shared.BSScrollingList.TEXT_OPTION_MULTILINE
			textField.SetMember("verticalAutoSize", RE::GFxValue("top"));
			break;
		}

		RE::GFxValue text;
		if (!aEntryObject.GetMember("text", &text) || text.IsUndefined()) {
			RE::GFxValue arg = RE::GFxValue(" ");
			textField.Invoke("SetText", nullptr, &arg, RE::UPInt(1));
		} else {
			textField.Invoke("SetText", nullptr, &text, RE::UPInt(1));
		}

		// the rest of the original function is replaced by custom logic to determine the text color
		RE::GFxValue topicIsNew;
		const auto newTopic = !aEntryObject.GetMember("topicIsNew", &topicIsNew) || topicIsNew.GetBool();
		colorText(textField, newTopic);
	}

	void SetEntryTextFunctionHandler::colorText(RE::GFxValue& a_textField, bool a_topicIsNew) noexcept
	{
		RE::GFxValue text;
		a_textField.GetMember("text", &text);
		const auto textStr = std::string(text.GetString());
		const auto where = topicDisplayData->find(textStr);
		if (where == topicDisplayData->end())
			return;

		const auto& displayData = where->second;
		RE::GFxValue textColor(a_topicIsNew ? displayData.newColor : displayData.oldColor);
		a_textField.SetMember("textColor", textColor);
	}

	void DoSetSelectedIndexFunctionHandler::Install(
		const RE::DialogueMenu* a_dialogueMenu,
		RE::GFxValue a_topicList,
		const std::unordered_map<std::string, TopicDisplayData>* a_topicDisplayData) noexcept
	{
		auto handler = RE::make_gptr<Scaleform::DoSetSelectedIndexFunctionHandler>();
		handler->topicDisplayData = a_topicDisplayData;
		handler->topicList = a_topicList;
		if (!a_dialogueMenu->uiMovie->GetVariable(&handler->dialogueMenu, "_root.DialogueMenu_mc")) {
			logger::error("Failed to get DialogueMenu_mc");
			return;
		}

		RE::GFxValue doSetSelectedIndexOriginal;
		handler->topicList.GetMember("doSetSelectedIndex", &doSetSelectedIndexOriginal);
		handler->topicList.SetMember("doSetSelectedIndexOriginal", doSetSelectedIndexOriginal);

		RE::GFxValue doSetSelectedIndexNew;
		a_dialogueMenu->uiMovie->CreateFunction(&doSetSelectedIndexNew, handler.get());
		handler->topicList.SetMember("doSetSelectedIndex", doSetSelectedIndexNew);
	}

	// replaces: https://github.com/Mardoxx/skyrimui/blob/425aa8a31de31fb11fe78ee6cec799f4ba31af03/src/common/Shared/BSScrollingList.as#L159-L182
	void DoSetSelectedIndexFunctionHandler::Call(Params& a_params)
	{
		topicList.Invoke("doSetSelectedIndexOriginal", nullptr, a_params.args, a_params.argCount);

		// new part of the function
		RE::GFxValue selectedEntry;
		a_params.thisPtr->Invoke("__get__selectedEntry", &selectedEntry);
		RE::GFxValue text;
		selectedEntry.GetMember("text", &text);
		const auto textStr = std::string(text.GetString());
		const auto where = topicDisplayData->find(textStr);
		if (where == topicDisplayData->end())
			return;
		
		const auto& displayData = where->second;
		RE::GFxValue subtitle(displayData.subtitle);
		dialogueMenu.Invoke("ShowDialogueText", nullptr, &subtitle, RE::UPInt(1));
	}
}