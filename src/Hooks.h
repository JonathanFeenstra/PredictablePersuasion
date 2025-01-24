#pragma once

namespace Hooks
{
	void Install() noexcept;

	class DialogueMenuEx final : public RE::DialogueMenu
	{
	public:
		static void Install() noexcept;
		RE::UI_MESSAGE_RESULTS ProcessMessageEx(RE::UIMessage& a_message) noexcept;

	private:
		using ProcessMessageFn = decltype(&RE::DialogueMenu::ProcessMessage);

		static inline REL::Relocation<ProcessMessageFn> _ProcessMessageFn;

		enum class SPEECH_CHECK_TYPE
		{
			kPersuade,
			kIntimidate,
			kBribe,
			kNone,
		};

		struct SpeechCheckData
		{
			std::string mainText;
			std::string tagText;
			SPEECH_CHECK_TYPE tagType;
			SPEECH_CHECK_TYPE checkType;
			bool passesCheck;
			float requiredSpeechLevel;  // only applicable for persuasion (bribes and intimidation are more complicated: https://en.uesp.net/wiki/Skyrim:Speech#Bribe_Formula)
		};

		// the ActionScript 2 code of the dialogue menu only has access to the text of the topics, so the topics that should be colored are stored
		static inline std::set<std::string> successTopics;
		static inline std::set<std::string> failureTopics;
		static inline std::set<std::string> noCheckTopics;

		static void processTopic(RE::MenuTopicManager::Dialogue* a_dialogue) noexcept;

		static SpeechCheckData getSpeechCheckData(const RE::MenuTopicManager::Dialogue* a_dialogue) noexcept;
		static void hydrateTextData(SpeechCheckData& a_speechCheckData, const RE::MenuTopicManager::Dialogue* a_dialogue) noexcept;
		static void hydrateCheckData(SpeechCheckData& a_speechCheckData, const RE::TESTopic* a_topic) noexcept;

		static bool evaluateSpeechCheck(const RE::TESConditionItem* a_conditionItem, bool a_checkForAmuletOfArticulation) noexcept;
	};
}