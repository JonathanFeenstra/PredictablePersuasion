#pragma once

#include "Scaleform.h"

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

		// https://stackoverflow.com/a/21439212
		typedef std::tuple<RE::FormID, RE::BSString> cache_key_t;

		struct cache_key_hash
		{
			size_t
			operator()(cache_key_t const& key) const
			{
				size_t seed = 0;
				const auto& formID = get<0>(key);
				const auto& text = get<1>(key);
				seed ^= std::hash<RE::FormID>()(formID) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
				seed ^= std::hash<const char*>()(text.c_str()) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
				return seed;
			}
		};

		enum class SPEECH_CHECK_TYPE
		{
			kPersuade,
			kIntimidate,
			kBribe,
			kNone,
		};

		struct SpeechCheckData final
		{
			std::string mainText;
			std::string tagText;
			SPEECH_CHECK_TYPE tagType;
			SPEECH_CHECK_TYPE checkType;
			bool passesCheck;
			float requiredSpeechLevel;  // only applicable for persuasion (bribes and intimidation are more complicated: https://en.uesp.net/wiki/Skyrim:Speech#Bribe_Formula)
			std::string predictedResponseText;
		};

		static inline std::unordered_map<std::string, Scaleform::TopicDisplayData> topicDisplayData;

		static void processTopic(RE::MenuTopicManager::Dialogue* a_dialogue) noexcept;

		static SpeechCheckData getSpeechCheckData(const RE::MenuTopicManager::Dialogue* a_dialogue) noexcept;
		static std::string applyFormat(
			const std::string& a_format,
			const SpeechCheckData* a_speechCheckData,
			const std::string& a_resultText,
			const float a_playerSpeechLevel) noexcept;

		static void hydrateTextData(SpeechCheckData& a_speechCheckData, const RE::MenuTopicManager::Dialogue* a_dialogue) noexcept;
		static void hydrateCheckData(SpeechCheckData& a_speechCheckData, const RE::TESTopic* a_topic) noexcept;

		static bool evaluateSpeechCheck(const RE::TESConditionItem* a_conditionItem, bool a_checkForAmuletOfArticulation) noexcept;
		static std::string getResponseText(RE::TESTopicInfo* a_responseInfo, RE::TESObjectREFR* a_speaker) noexcept;
	};
}