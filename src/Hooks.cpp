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

#include "Hooks.h"

#include "Events.h"
#include "Settings.h"
#include "StringUtil.h"

namespace Hooks
{
	void Install() noexcept
	{
		DialogueMenuEx::Install();
	}

	void DialogueMenuEx::Install() noexcept
	{
		REL::Relocation<uintptr_t> vtbl(RE::VTABLE_DialogueMenu[0]);
		_ProcessMessageFn = vtbl.write_vfunc(0x4, &ProcessMessageEx);
		if (Settings::applyTextColors) {
			Events::MenuOpenCloseEventSink::Install(&successTopics, &failureTopics, &noCheckTopics);
		}
	}

	RE::UI_MESSAGE_RESULTS DialogueMenuEx::ProcessMessageEx(RE::UIMessage& a_message) noexcept
	{
		static std::unordered_map<RE::FormID, std::string> cache;
		switch (*a_message.type) {
		case RE::UI_MESSAGE_TYPE::kShow:
		case RE::UI_MESSAGE_TYPE::kUpdate:
			if (const auto dialogueList = RE::MenuTopicManager::GetSingleton()->dialogueList) {
				for (auto it = dialogueList->begin(); it != dialogueList->end(); ++it) {
					const auto dialogue = *it;
					if (!dialogue)
						continue;
					const auto formID = dialogue->parentTopic->formID;
					auto where = cache.find(formID);
					if (where != cache.end()) {
						dialogue->topicText = where->second;
						continue;
					}
					processTopic(dialogue);
					cache[formID] = dialogue->topicText;
				}
			}
			break;
		case RE::UI_MESSAGE_TYPE::kHide:
			cache.clear();
			if (Settings::applyTextColors) {
				successTopics.clear();
				failureTopics.clear();
			}
			break;
		}

		return _ProcessMessageFn(this, a_message);
	}

	void DialogueMenuEx::processTopic(RE::MenuTopicManager::Dialogue* a_dialogue) noexcept
	{
		const auto speechCheckData = getSpeechCheckData(a_dialogue);
		SPEECH_CHECK_TYPE checkType;
		std::set<std::string>* resultSet;
		std::string resultText;
		if (speechCheckData.checkType == SPEECH_CHECK_TYPE::kNone) {
			resultSet = &noCheckTopics;
			resultText = Settings::noCheckString;
			checkType = speechCheckData.tagType;
		} else {
			checkType = speechCheckData.checkType;
			if (speechCheckData.passesCheck) {
				resultSet = &successTopics;
				resultText = Settings::checkSuccessString;
			} else {
				resultSet = &failureTopics;
				resultText = Settings::checkFailureString;
			}
		}
		
		if (Settings::applyFormatting) {
			switch (checkType) {
			case SPEECH_CHECK_TYPE::kNone:
				return;  // neither the tag nor the response conditions indicate a speech check, so a regular topic is assumed
			case SPEECH_CHECK_TYPE::kPersuade:
				a_dialogue->topicText = std::vformat(Settings::persuadeFormat, std::make_format_args(speechCheckData.mainText, speechCheckData.tagText, resultText, speechCheckData.requiredSpeechLevel));
				break;
			case SPEECH_CHECK_TYPE::kIntimidate:
				a_dialogue->topicText = std::vformat(Settings::intimidateFormat, std::make_format_args(speechCheckData.mainText, speechCheckData.tagText, resultText));
				break;
			case SPEECH_CHECK_TYPE::kBribe:
				a_dialogue->topicText = std::vformat(Settings::bribeFormat, std::make_format_args(speechCheckData.mainText, speechCheckData.tagText, resultText));
				break;
			}
		}

		if (Settings::applyTextColors)
		{
			resultSet->insert(a_dialogue->topicText.c_str());
		}
	}

	DialogueMenuEx::SpeechCheckData DialogueMenuEx::getSpeechCheckData(const RE::MenuTopicManager::Dialogue* a_dialogue) noexcept
	{
		SpeechCheckData result{ {}, {}, SPEECH_CHECK_TYPE::kNone, SPEECH_CHECK_TYPE::kNone, false, 0.0F };
		const auto topic = a_dialogue->parentTopic;
		if (!topic)
			return result;

		hydrateTextData(result, a_dialogue);
		hydrateCheckData(result, topic);
		return result;
	}

	void DialogueMenuEx::hydrateTextData(DialogueMenuEx::SpeechCheckData& a_speechCheckData, const RE::MenuTopicManager::Dialogue* a_dialogue) noexcept
	{
		const std::string topicText(a_dialogue->topicText.c_str(), a_dialogue->topicText.size());
		std::smatch tagMatch;
		try {
			if (std::regex_search(topicText, tagMatch, Settings::persuadeTagRegex)) {
				a_speechCheckData.tagType = SPEECH_CHECK_TYPE::kPersuade;
			} else if (std::regex_search(topicText, tagMatch, Settings::intimidateTagRegex)) {
				a_speechCheckData.tagType = SPEECH_CHECK_TYPE::kIntimidate;
			} else if (std::regex_search(topicText, tagMatch, Settings::bribeTagRegex) && StringUtil::LowerCaseContains(a_dialogue->parentTopic->fullName, "<bribecost>")) {
				a_speechCheckData.tagType = SPEECH_CHECK_TYPE::kBribe;
			}
		} catch (const std::regex_error& e) {
			logger::error("Failed to match regex: {}", e.what());
		}

		a_speechCheckData.mainText = topicText.substr(0, topicText.size() - tagMatch.length());
		a_speechCheckData.tagText = tagMatch.str(1);
	}

	void DialogueMenuEx::hydrateCheckData(DialogueMenuEx::SpeechCheckData& a_speechCheckData, const RE::TESTopic* a_topic) noexcept
	{
		// based on: https://github.com/Scrabx3/Dynamic-Dialogue-Replacer/blob/3ffe893f741a9e1530c9bcb5577465b6e9ccad0b/src/Hooks/Hooks.cpp#L96-L105
		auto infoPtr = a_topic->topicInfos;
		for (auto i = a_topic->numTopicInfos; i > 0; --i) {
			if (!infoPtr)
				return;
			if (const auto responseInfo = *infoPtr) {
				auto conditionItem = responseInfo->objConditions.head;

				while (conditionItem) {
					const auto data = conditionItem->data;
					const auto function = data.functionData.function;
					// evaluating the full responseInfo.objConditions doesn't always return the correct result, so only evaluate the speech checks
					if (function == RE::FUNCTION_DATA::FunctionID::kGetActorValue) {
						const auto actorValue = static_cast<RE::ActorValue>(reinterpret_cast<intptr_t>(data.functionData.params[0]));
						if (actorValue == RE::ActorValue::kSpeech && data.flags.opCode == RE::CONDITION_ITEM_DATA::OpCode::kGreaterThanOrEqualTo) {
							a_speechCheckData.checkType = SPEECH_CHECK_TYPE::kPersuade;
							a_speechCheckData.requiredSpeechLevel = data.comparisonValue.g ? data.comparisonValue.g->value : data.comparisonValue.f;
							a_speechCheckData.passesCheck = evaluateSpeechCheck(conditionItem, true);
							return;
						}
					} else if (function == RE::FUNCTION_DATA::FunctionID::kGetBribeSuccess) {
						a_speechCheckData.checkType = SPEECH_CHECK_TYPE::kBribe;
						a_speechCheckData.passesCheck = evaluateSpeechCheck(conditionItem, false);
						return;
					} else if (function == RE::FUNCTION_DATA::FunctionID::kGetIntimidateSuccess) {
						a_speechCheckData.checkType = SPEECH_CHECK_TYPE::kIntimidate;
						a_speechCheckData.passesCheck = evaluateSpeechCheck(conditionItem, false);
						return;
					}
					conditionItem = conditionItem->next;
				}
			}
			++infoPtr;
		}
	}

	bool DialogueMenuEx::evaluateSpeechCheck(const RE::TESConditionItem* a_conditionItem, bool a_checkForAmuletOfArticulation) noexcept
	{
		const auto speaker = RE::MenuTopicManager::GetSingleton()->speaker.get().get();
		const auto player = RE::PlayerCharacter::GetSingleton();
		auto checkParams = RE::ConditionCheckParams(speaker, player);
		const auto result = a_conditionItem->IsTrue(checkParams);
		if (result)
			return true;
		// persuasion checks are usually followed by checking if the Amulet of Articulation is equipped
		if (!a_checkForAmuletOfArticulation || !a_conditionItem->data.flags.isOR || !a_conditionItem->next || a_conditionItem->next->data.functionData.function != RE::FUNCTION_DATA::FunctionID::kGetEquipped)
			return false;
		// the following forms should be the same here:
		//const auto formToCheck = std::bit_cast<RE::TESForm*>(a_conditionItem->next->data.functionData.params[0]);
		//const auto amuletOfArticulationFormList = RE::TESForm::LookupByEditorID("TGAmuletOfArticulationList");
		return a_conditionItem->next->IsTrue(checkParams);
	}
}