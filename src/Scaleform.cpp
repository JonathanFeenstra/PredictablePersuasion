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

		RE::GFxValue dialogueMenu_mc;
		if (!dialogueMenu->uiMovie->GetVariable(&dialogueMenu_mc, "_root.DialogueMenu_mc")) {
			logger::error("Failed to get DialogueMenu_mc");
			return;
		}

		RE::GFxValue subtitleText;
		if (!dialogueMenu_mc.GetMember("SubtitleText", &subtitleText)) {
			logger::error("Failed to get SubtitleText");
			return;
		}

		if (Settings::applyTopicColors) {
			SetEntryTextFunctionHandler::Install(dialogueMenu, topicList, a_topicDisplayData);
		}

		if (Settings::showSubtitles != Settings::SHOW_SUBTITLES::kNever) {
			ShowDialogueTextFunctionHandler::Install(dialogueMenu, dialogueMenu_mc, subtitleText);
			DoSetSelectedIndexFunctionHandler::Install(dialogueMenu, dialogueMenu_mc, subtitleText, topicList, a_topicDisplayData);
			if (topicList.HasMember("iHighlightedIndex")) {
				// Better Dialogue Controls and mods based on it decouple mouse highlighting from the selected item:
				// See: https://github.com/fabd/skyrimui/commit/e5f0d8d719acd2d2545357d4415882f54084d74d
				MoveSelectionUpFunctionHandler::Install(dialogueMenu, dialogueMenu_mc, subtitleText, topicList, a_topicDisplayData);
				MoveSelectionDownFunctionHandler::Install(dialogueMenu, dialogueMenu_mc, subtitleText, topicList, a_topicDisplayData);
			}
		}
	}

	void ShowModSubtitle(
		RE::GFxValue a_dialogueMenu_mc,
		RE::GFxValue a_topicList,
		RE::GFxValue a_subtitleText,
		const std::unordered_map<std::string, TopicDisplayData>* a_topicDisplayData) noexcept
	{
		if (!IsTopicListShown(a_dialogueMenu_mc))
			return;

		RE::GFxValue highlightedEntry = GetHiglightedEntry(a_topicList);
		RE::GFxValue text;
		highlightedEntry.GetMember("text", &text);
		const auto textStr = std::string(text.GetString());
		if (textStr.empty())
			return;

		const auto where = a_topicDisplayData->find(textStr);
		if (where == a_topicDisplayData->end())
			return;

		const auto& displayData = where->second;
		if (displayData.subtitle.empty()) {
			// prevent hiding game subtitles by overwriting them with empty strings
			RE::GFxValue isGameSubtitle;
			if (a_dialogueMenu_mc.GetMember("bIsGameSubtitle", &isGameSubtitle) && isGameSubtitle.GetBool()) {
				RE::GFxValue currentSubtitle;
				if (a_subtitleText.GetMember("text", &currentSubtitle)) {
					std::string currentSubtitleStr(currentSubtitle.GetString());
					if (!currentSubtitleStr.empty() && currentSubtitleStr != " ") {
						return;
					}
				}
			}
		}

		RE::GFxValue subtitle(displayData.subtitle);
		a_subtitleText.SetMember("textColor", Settings::subtitleColor);
		a_subtitleText.Invoke("SetText", nullptr, &subtitle, RE::UPInt(1));
		a_dialogueMenu_mc.SetMember("bIsGameSubtitle", false);
	}

	bool IsTopicListShown(RE::GFxValue a_dialogueMenu_mc) noexcept
	{
		RE::GFxValue eMenuState;
		if (!a_dialogueMenu_mc.GetMember("eMenuState", &eMenuState))
			return false;
		return eMenuState.GetNumber() == 1;  // eMenuState == TOPIC_LIST_SHOWN
	}

	RE::GFxValue GetHiglightedEntry(RE::GFxValue a_topicList) noexcept
	{
		RE::GFxValue highlightedEntry;
		RE::GFxValue iHighlightedIndex;
		if (a_topicList.GetMember("iHighlightedIndex", &iHighlightedIndex) && iHighlightedIndex.GetNumber() != -1) {
			RE::GFxValue entriesA;
			a_topicList.GetMember("EntriesA", &entriesA);
			entriesA.GetElement(iHighlightedIndex.GetNumber(), &highlightedEntry);
		} else {
			a_topicList.Invoke("__get__selectedEntry", &highlightedEntry);
		}

		return highlightedEntry;
	}

	void SetEntryTextFunctionHandler::Install(
		const RE::DialogueMenu* a_dialogueMenu,
		RE::GFxValue a_topicList,
		const std::unordered_map<std::string, TopicDisplayData>* a_topicDisplayData) noexcept
	{
		auto handler = RE::make_gptr<Scaleform::SetEntryTextFunctionHandler>();
		handler->topicDisplayData = a_topicDisplayData;

		RE::GFxValue setEntryTextOriginal;
		a_topicList.GetMember("SetEntryText", &setEntryTextOriginal);
		a_topicList.SetMember("SetEntryTextOriginal", setEntryTextOriginal);

		RE::GFxValue setEntryTextNew;
		a_dialogueMenu->uiMovie->CreateFunction(&setEntryTextNew, handler.get());
		a_topicList.SetMember("SetEntryText", setEntryTextNew);
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

		thisPtr->Invoke("SetEntryTextOriginal", nullptr, a_params.args, a_params.argCount);

		// new part of the function
		RE::GFxValue textField;
		if (!aEntryClip.GetMember("textField", &textField) || textField.IsUndefined())
			return;

		RE::GFxValue topicIsNew;
		const auto newTopic = !aEntryObject.GetMember("topicIsNew", &topicIsNew) || topicIsNew.GetBool();
		colorText(textField, newTopic);
	}

	void SetEntryTextFunctionHandler::colorText(RE::GFxValue a_textField, bool a_topicIsNew) noexcept
	{
		RE::GFxValue text;
		a_textField.GetMember("text", &text);
		const auto textStr = std::string(text.GetString());
		const auto where = topicDisplayData->find(textStr);
		if (where == topicDisplayData->end())
			return;

		const auto& displayData = where->second;
		a_textField.SetMember("textColor", a_topicIsNew ? displayData.newColor : displayData.oldColor);
	}

	void ShowDialogueTextFunctionHandler::Install(const RE::DialogueMenu* a_dialogueMenu, RE::GFxValue a_dialogueMenu_mc, RE::GFxValue a_subtitleText) noexcept
	{
		auto handler = RE::make_gptr<Scaleform::ShowDialogueTextFunctionHandler>();
		handler->dialogueMenu_mc = a_dialogueMenu_mc;
		handler->subtitleText = a_subtitleText;

		if (!handler->subtitleText.GetMember("textColor", &handler->defaultSubtitleColor)) {
			logger::error("Failed to get default subtitle color");
			return;
		}

		a_dialogueMenu_mc.SetMember("bIsGameSubtitle", false);

		RE::GFxValue showDialogueText;
		a_dialogueMenu->uiMovie->CreateFunction(&showDialogueText, handler.get());
		a_dialogueMenu_mc.SetMember("ShowDialogueText", showDialogueText);
	}

	// replaces: https://github.com/Mardoxx/skyrimui/blob/425aa8a31de31fb11fe78ee6cec799f4ba31af03/src/dialoguemenu/DialogueMenu.as#L116-L119
	void ShowDialogueTextFunctionHandler::Call(Params& a_params)
	{
		if (a_params.argCount < 1) {
			logger::error("ShowDialogueText: Expected 1 argument, found {}", a_params.argCount);
			return;
		}

		const auto& astrText = a_params.args[0];

		subtitleText.SetMember("textColor", defaultSubtitleColor);
		subtitleText.Invoke("SetText", nullptr, &astrText, RE::UPInt(1));
		dialogueMenu_mc.SetMember("bIsGameSubtitle", true);
	}

	void DoSetSelectedIndexFunctionHandler::Install(
		const RE::DialogueMenu* a_dialogueMenu,
		RE::GFxValue a_dialogueMenu_mc,
		RE::GFxValue a_subtitleText,
		RE::GFxValue a_topicList,
		const std::unordered_map<std::string, TopicDisplayData>* a_topicDisplayData) noexcept
	{
		auto handler = RE::make_gptr<Scaleform::DoSetSelectedIndexFunctionHandler>();
		handler->topicDisplayData = a_topicDisplayData;
		handler->dialogueMenu_mc = a_dialogueMenu_mc;
		handler->subtitleText = a_subtitleText;
		handler->topicList = a_topicList;

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
		a_params.thisPtr->Invoke("doSetSelectedIndexOriginal", nullptr, a_params.args, a_params.argCount);
		ShowModSubtitle(dialogueMenu_mc, topicList, subtitleText, topicDisplayData);
	}

	void MoveSelectionUpFunctionHandler::Install(
		const RE::DialogueMenu* a_dialogueMenu,
		RE::GFxValue a_dialogueMenu_mc,
		RE::GFxValue a_subtitleText,
		RE::GFxValue a_topicList,
		const std::unordered_map<std::string, TopicDisplayData>* a_topicDisplayData) noexcept
	{
		auto handler = RE::make_gptr<Scaleform::MoveSelectionUpFunctionHandler>();
		handler->topicDisplayData = a_topicDisplayData;
		handler->dialogueMenu_mc = a_dialogueMenu_mc;
		handler->subtitleText = a_subtitleText;
		handler->topicList = a_topicList;

		RE::GFxValue moveSelectionUpOriginal;
		a_topicList.GetMember("moveSelectionUp", &moveSelectionUpOriginal);
		a_topicList.SetMember("moveSelectionUpOriginal", moveSelectionUpOriginal);

		RE::GFxValue moveSelectionUpNew;
		a_dialogueMenu->uiMovie->CreateFunction(&moveSelectionUpNew, handler.get());
		a_topicList.SetMember("moveSelectionUp", moveSelectionUpNew);
	}

	// replaces: https://github.com/fabd/skyrimui/blob/ba35b0b559939e9b53179599f96757a46f168357/src/common/Shared/BSScrollingList.as#L443-L451
	void MoveSelectionUpFunctionHandler::Call(Params& a_params)
	{
		a_params.thisPtr->Invoke("moveSelectionUpOriginal", nullptr, a_params.args, a_params.argCount);
		ShowModSubtitle(dialogueMenu_mc, topicList, subtitleText, topicDisplayData);
	}

	void MoveSelectionDownFunctionHandler::Install(
		const RE::DialogueMenu* a_dialogueMenu,
		RE::GFxValue a_dialogueMenu_mc,
		RE::GFxValue a_subtitleText,
		RE::GFxValue a_topicList,
		const std::unordered_map<std::string, TopicDisplayData>* a_topicDisplayData) noexcept
	{
		auto handler = RE::make_gptr<Scaleform::MoveSelectionDownFunctionHandler>();
		handler->topicDisplayData = a_topicDisplayData;
		handler->dialogueMenu_mc = a_dialogueMenu_mc;
		handler->subtitleText = a_subtitleText;
		handler->topicList = a_topicList;
		
		RE::GFxValue moveSelectionDownOriginal;
		a_topicList.GetMember("moveSelectionDown", &moveSelectionDownOriginal);
		a_topicList.SetMember("moveSelectionDownOriginal", moveSelectionDownOriginal);

		RE::GFxValue moveSelectionDownNew;
		a_dialogueMenu->uiMovie->CreateFunction(&moveSelectionDownNew, handler.get());
		a_topicList.SetMember("moveSelectionDown", moveSelectionDownNew);
	}

	// replaces: https://github.com/fabd/skyrimui/blob/ba35b0b559939e9b53179599f96757a46f168357/src/common/Shared/BSScrollingList.as#L453-L461
	void MoveSelectionDownFunctionHandler::Call(Params& a_params)
	{
		a_params.thisPtr->Invoke("moveSelectionDownOriginal", nullptr, a_params.args, a_params.argCount);
		ShowModSubtitle(dialogueMenu_mc, topicList, subtitleText, topicDisplayData);
	}
}