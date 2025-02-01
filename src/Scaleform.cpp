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
		DialogueMenuUI::Install(a_topicDisplayData);
		const auto dialogueMenuUI = DialogueMenuUI::GetSingleton();

		if (Settings::applyTopicColors) {
			SetEntryTextFunctionHandler::Install(dialogueMenuUI);
		}

		if (Settings::showSubtitles != Settings::SHOW_SUBTITLES::kNever) {
			ShowDialogueTextFunctionHandler::Install(dialogueMenuUI);
			DoSetSelectedIndexFunctionHandler::Install(dialogueMenuUI);
			if (dialogueMenuUI->UsesBetterDialogueControls()) {
				// Better Dialogue Controls and mods based on it decouple mouse highlighting from the selected item:
				// See: https://github.com/fabd/skyrimui/commit/e5f0d8d719acd2d2545357d4415882f54084d74d
				MoveSelectionUpFunctionHandler::Install(dialogueMenuUI);
				MoveSelectionDownFunctionHandler::Install(dialogueMenuUI);
			}
		}
	}

	DialogueMenuUI* DialogueMenuUI::GetSingleton() noexcept
	{
		static DialogueMenuUI singleton;
		return &singleton;
	}

	void DialogueMenuUI::Install(const std::unordered_map<std::string, TopicDisplayData>* a_topicDisplayData) noexcept
	{
		const auto singleton = GetSingleton();
		singleton->topicDisplayData = a_topicDisplayData;

		RE::GFxValue subtitleColor;
		if (!singleton->getSubtitleText().GetMember("textColor", &subtitleColor)) {
			logger::error("Failed to get default subtitle color");
			return;
		}

		singleton->defaultSubtitleColor = subtitleColor.GetNumber();
	}

	void DialogueMenuUI::CopyOriginalTopicListFunction(const char* a_functionName) noexcept
	{
		auto topicList = getTopicList();
		RE::GFxValue originalFunction;
		topicList.GetMember(a_functionName, &originalFunction);
		const auto originalFunctionName = std::string(a_functionName) + "Original";
		topicList.SetMember(originalFunctionName.c_str(), originalFunction);
	}

	void DialogueMenuUI::ReplaceFunction(const char* a_functionName, RE::GFxFunctionHandler* a_newHandler) noexcept
	{
		RE::GFxValue newFunction;
		getDialogueMenu()->uiMovie->CreateFunction(&newFunction, a_newHandler);
		getDialogueMenu_mc().SetMember(a_functionName, newFunction);
	}

	void DialogueMenuUI::ReplaceTopicListFunction(const char* a_functionName, RE::GFxFunctionHandler* a_newHandler) noexcept
	{
		RE::GFxValue newFunction;
		getDialogueMenu()->uiMovie->CreateFunction(&newFunction, a_newHandler);
		getTopicList().SetMember(a_functionName, newFunction);
	}

	void DialogueMenuUI::ColorText(RE::GFxValue a_textField, bool a_topicIsNew) noexcept
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

	bool DialogueMenuUI::IsTopicListShown() noexcept
	{
		RE::GFxValue eMenuState;
		if (!getDialogueMenu_mc().GetMember("eMenuState", &eMenuState))
			return false;
		return eMenuState.GetNumber() == 1;  // eMenuState == TOPIC_LIST_SHOWN
	}

	bool DialogueMenuUI::UsesBetterDialogueControls() noexcept
	{
		return getDialogueMenu_mc().HasMember("iHighlightedIndex");
	}

	void DialogueMenuUI::ShowGameSubtitle(const RE::GFxValue a_strText) noexcept
	{
		auto subtitleText = getSubtitleText();
		subtitleText.SetMember("textColor", defaultSubtitleColor);
		subtitleText.Invoke("SetText", nullptr, &a_strText, RE::UPInt(1));
		isGameSubtitle = true;
	}

	void DialogueMenuUI::ShowModSubtitle() noexcept
	{
		RE::GFxValue text;
		getHiglightedEntry().GetMember("text", &text);
		const auto textStr = std::string(text.GetString());
		if (textStr.empty())
			return;

		const auto where = topicDisplayData->find(textStr);
		if (where == topicDisplayData->end())
			return;

		auto subtitleText = getSubtitleText();
		const auto& displayData = where->second;
		if (displayData.subtitle.empty()) {
			// prevent hiding real subtitles by overwriting them with empty strings
			if (!isGameSubtitle) {
				RE::GFxValue currentSubtitle;
				if (subtitleText.GetMember("text", &currentSubtitle)) {
					std::string currentSubtitleStr(currentSubtitle.GetString());
					if (!currentSubtitleStr.empty() && currentSubtitleStr != " ") {
						return;
					}
				}
			}
		}

		RE::GFxValue subtitle(displayData.subtitle);
		subtitleText.SetMember("textColor", Settings::subtitleColor);
		subtitleText.Invoke("SetText", nullptr, &subtitle, RE::UPInt(1));
		isGameSubtitle = false;
	}

	RE::DialogueMenu* DialogueMenuUI::getDialogueMenu() noexcept
	{
		const auto ui = RE::UI::GetSingleton();
		if (!ui) {
			logger::error("Failed to get UI");
		}

		const auto dialogueMenu = ui->GetMenu<RE::DialogueMenu>().get();
		if (!dialogueMenu) {
			logger::error("Failed to get DialogueMenu");
		}

		return dialogueMenu;
	}

	RE::GFxValue DialogueMenuUI::getDialogueMenu_mc() noexcept
	{
		RE::GFxValue dialogueMenu_mc;
		if (!getDialogueMenu()->uiMovie->GetVariable(&dialogueMenu_mc, "_root.DialogueMenu_mc")) {
			logger::error("Failed to get DialogueMenu_mc");
		}

		return dialogueMenu_mc;
	}

	RE::GFxValue DialogueMenuUI::getTopicList() noexcept
	{
		RE::GFxValue topicList;
		if (!getDialogueMenu()->uiMovie->GetVariable(&topicList, "_root.DialogueMenu_mc.TopicListHolder.List_mc")) {
			logger::error("Failed to get TopicList");
		}

		return topicList;
	}

	RE::GFxValue DialogueMenuUI::getSubtitleText() noexcept
	{
		RE::GFxValue subtitleText;
		if (!getDialogueMenu_mc().GetMember("SubtitleText", &subtitleText)) {
			logger::error("Failed to get SubtitleText");
		}

		return subtitleText;
	}
	
	RE::GFxValue DialogueMenuUI::getHiglightedEntry() noexcept
	{
		auto topicList = getTopicList();
		RE::GFxValue highlightedEntry;
		RE::GFxValue iHighlightedIndex;
		if (topicList.GetMember("iHighlightedIndex", &iHighlightedIndex) && iHighlightedIndex.GetNumber() != -1) {
			RE::GFxValue entriesA;
			topicList.GetMember("EntriesA", &entriesA);
			entriesA.GetElement(iHighlightedIndex.GetNumber(), &highlightedEntry);
		} else {
			topicList.Invoke("__get__selectedEntry", &highlightedEntry);
		}

		return highlightedEntry;
	}

	void SetEntryTextFunctionHandler::Install(DialogueMenuUI* a_dialogueMenuUI) noexcept
	{
		auto handler = RE::make_gptr<Scaleform::SetEntryTextFunctionHandler>();
		handler->dialogueMenuUI = a_dialogueMenuUI;
		constexpr auto functionName = "SetEntryText";
		a_dialogueMenuUI->CopyOriginalTopicListFunction(functionName);
		a_dialogueMenuUI->ReplaceTopicListFunction(functionName, handler.get());
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
		dialogueMenuUI->ColorText(textField, newTopic);
	}

	void ShowDialogueTextFunctionHandler::Install(DialogueMenuUI* a_dialogueMenuUI) noexcept
	{
		auto handler = RE::make_gptr<Scaleform::ShowDialogueTextFunctionHandler>();
		handler->dialogueMenuUI = a_dialogueMenuUI;
		constexpr auto functionName = "ShowDialogueText";
		a_dialogueMenuUI->ReplaceFunction(functionName, handler.get());
	}

	// replaces: https://github.com/Mardoxx/skyrimui/blob/425aa8a31de31fb11fe78ee6cec799f4ba31af03/src/dialoguemenu/DialogueMenu.as#L116-L119
	void ShowDialogueTextFunctionHandler::Call(Params& a_params)
	{
		if (a_params.argCount < 1) {
			logger::error("ShowDialogueText: Expected 1 argument, found {}", a_params.argCount);
			return;
		}

		const auto& astrText = a_params.args[0];
		dialogueMenuUI->ShowGameSubtitle(astrText);
	}

	void DoSetSelectedIndexFunctionHandler::Install(DialogueMenuUI* a_dialogueMenuUI) noexcept
	{
		auto handler = RE::make_gptr<Scaleform::DoSetSelectedIndexFunctionHandler>();
		handler->dialogueMenuUI = a_dialogueMenuUI;
		constexpr auto functionName = "doSetSelectedIndex";
		a_dialogueMenuUI->CopyOriginalTopicListFunction(functionName);
		a_dialogueMenuUI->ReplaceTopicListFunction(functionName, handler.get());
	}

	// replaces: https://github.com/Mardoxx/skyrimui/blob/425aa8a31de31fb11fe78ee6cec799f4ba31af03/src/common/Shared/BSScrollingList.as#L159-L182
	void DoSetSelectedIndexFunctionHandler::Call(Params& a_params)
	{
		a_params.thisPtr->Invoke("doSetSelectedIndexOriginal", nullptr, a_params.args, a_params.argCount);

		// new part of the function
		if (!dialogueMenuUI->IsTopicListShown())
			return;

		dialogueMenuUI->ShowModSubtitle();
	}

	void MoveSelectionUpFunctionHandler::Install(DialogueMenuUI* a_dialogueMenuUI) noexcept
	{
		auto handler = RE::make_gptr<Scaleform::MoveSelectionUpFunctionHandler>();
		handler->dialogueMenuUI = a_dialogueMenuUI;
		constexpr auto functionName = "moveSelectionUp";
		a_dialogueMenuUI->CopyOriginalTopicListFunction(functionName);
		a_dialogueMenuUI->ReplaceTopicListFunction(functionName, handler.get());
	}

	// replaces: https://github.com/fabd/skyrimui/blob/ba35b0b559939e9b53179599f96757a46f168357/src/common/Shared/BSScrollingList.as#L443-L451
	void MoveSelectionUpFunctionHandler::Call(Params& a_params)
	{
		a_params.thisPtr->Invoke("moveSelectionUpOriginal", nullptr, a_params.args, a_params.argCount);

		// new part of the function
		if (!dialogueMenuUI->IsTopicListShown())
			return;

		dialogueMenuUI->ShowModSubtitle();
	}

	void MoveSelectionDownFunctionHandler::Install(DialogueMenuUI* a_dialogueMenuUI) noexcept
	{
		auto handler = RE::make_gptr<Scaleform::MoveSelectionDownFunctionHandler>();
		handler->dialogueMenuUI = a_dialogueMenuUI;
		constexpr auto functionName = "moveSelectionDown";
		a_dialogueMenuUI->CopyOriginalTopicListFunction(functionName);
		a_dialogueMenuUI->ReplaceTopicListFunction(functionName, handler.get());
	}

	// replaces: https://github.com/fabd/skyrimui/blob/ba35b0b559939e9b53179599f96757a46f168357/src/common/Shared/BSScrollingList.as#L453-L461
	void MoveSelectionDownFunctionHandler::Call(Params& a_params)
	{
		a_params.thisPtr->Invoke("moveSelectionDownOriginal", nullptr, a_params.args, a_params.argCount);

		// new part of the function
		if (!dialogueMenuUI->IsTopicListShown())
			return;

		dialogueMenuUI->ShowModSubtitle();
	}
}