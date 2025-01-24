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
	void SetEntryTextFunctionHandler::Install(
		const std::set<std::string>* a_successTopics,
		const std::set<std::string>* a_failureTopics,
		const std::set<std::string>* a_noCheckTopics) noexcept
	{
		const auto ui = RE::UI::GetSingleton();
		if (!ui) {
			logger::error("Failed to get UI");
			return;
		}

		const auto menu = ui->GetMenu<RE::DialogueMenu>();
		if (!menu) {
			logger::error("Failed to get DialogueMenu");
			return;
		}

		RE::GFxValue topicList;
		if (!menu->uiMovie->GetVariable(&topicList, "_root.DialogueMenu_mc.TopicListHolder.List_mc")) {
			logger::error("Failed to get TopicList");
			return;
		}

		RE::GFxValue setEntryText;
		auto setEntryTextFunction = RE::make_gptr<Scaleform::SetEntryTextFunctionHandler>();
		setEntryTextFunction->successTopics = a_successTopics;
		setEntryTextFunction->failureTopics = a_failureTopics;
		setEntryTextFunction->noCheckTopics = a_noCheckTopics;

		menu->uiMovie->CreateFunction(&setEntryText, setEntryTextFunction.get());
		topicList.SetMember("SetEntryText", setEntryText);
	}

	// replaces: https://github.com/Mardoxx/skyrimui/blob/425aa8a31de31fb11fe78ee6cec799f4ba31af03/src/dialoguemenu/DialogueCenteredList.as#L23-L29
	void SetEntryTextFunctionHandler::Call(Params& a_params)
	{
		if (a_params.argCount < 2) {
			logger::error("SetEntry: Expected at least 2 arguments, found {}", a_params.argCount);
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
		const auto textStr = text.GetString();
		RE::GFxValue textColor;
		if (successTopics->find(textStr) != successTopics->end()) {
			textColor = RE::GFxValue(Settings::successColor);
		} else if (failureTopics->find(textStr) != failureTopics->end()) {
			textColor = RE::GFxValue(a_topicIsNew ? Settings::failureColorNew : Settings::failureColorOld);
		} else if (noCheckTopics->find(textStr) != noCheckTopics->end()) {
			textColor = RE::GFxValue(a_topicIsNew ? Settings::noCheckColorNew : Settings::noCheckColorOld);
		} else {
			textColor = RE::GFxValue(a_topicIsNew ? Settings::regularColorNew : Settings::regularColorOld);
		}

		a_textField.SetMember("textColor", textColor);
	}
}