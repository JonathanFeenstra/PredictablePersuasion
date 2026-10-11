#pragma once

#include <RE/D/DialogueMenu.h>
#include <RE/G/GFxFunctionHandler.h>
#include <RE/G/GFxValue.h>

#include <cstdint>
#include <string>
#include <unordered_map>

namespace Scaleform
{
	// the ActionScript 2 code of the dialogue menu only has access to the text of the topics, so additional data needs to be passed
	struct TopicDisplayData final
	{
		std::uint32_t oldColor{ 0 };
		std::uint32_t newColor{ 0 };
		bool hasSpeechCheck{ false };
		std::string subtitle;
	};

	void InstallHooks(
		const std::unordered_map<std::string, TopicDisplayData>* a_topicDisplayData,
		bool a_smartTalkSetEntryTextCustomFunctionEnabled) noexcept;

	void ShowModSubtitle(
		RE::GFxValue a_dialogueMenu_mc,
		RE::GFxValue a_topicList,
		RE::GFxValue a_subtitleText,
		const std::unordered_map<std::string, TopicDisplayData>* a_topicDisplayData) noexcept;

	bool IsTopicListShown(RE::GFxValue a_dialogueMenu_mc) noexcept;
	RE::GFxValue GetHiglightedEntry(RE::GFxValue a_topicList) noexcept;

	void ColorText(
		RE::GFxValue a_textField,
		bool a_topicIsNew,
		const std::unordered_map<std::string, TopicDisplayData>* a_topicDisplayData,
		bool a_isSmartTalkQuestLine) noexcept;

	class SetEntryTextFunctionHandler final : public RE::GFxFunctionHandler
	{
	public:
		static void Install(
			const RE::DialogueMenu* a_dialogueMenu,
			RE::GFxValue a_topicList,
			const std::unordered_map<std::string, TopicDisplayData>* a_topicDisplayData) noexcept;

		void Call(Params& a_params) override;

	private:
		const std::unordered_map<std::string, TopicDisplayData>* topicDisplayData{ nullptr };
	};

	class SmartTalkSetEntryTextCustomFunctionHandler final : public RE::GFxFunctionHandler
	{
	public:
		static void Install(
			const RE::DialogueMenu* a_dialogueMenu,
			RE::GFxValue a_topicList,
			const std::unordered_map<std::string, TopicDisplayData>* a_topicDisplayData) noexcept;

		void Call(Params& a_params) override;

	private:
		const std::unordered_map<std::string, TopicDisplayData>* topicDisplayData{ nullptr };
	};

	class ShowDialogueTextFunctionHandler final : public RE::GFxFunctionHandler
	{
	public:
		static void Install(const RE::DialogueMenu* a_dialogueMenu, RE::GFxValue a_dialogueMenu_mc, RE::GFxValue a_subtitleText) noexcept;

		void Call(Params& a_params) override;

	private:
		RE::GFxValue dialogueMenu_mc;
		RE::GFxValue subtitleText;
		RE::GFxValue defaultSubtitleColor;
	};

	class DoSetSelectedIndexFunctionHandler final : public RE::GFxFunctionHandler
	{
	public:
		static void Install(
			const RE::DialogueMenu* a_dialogueMenu,
			RE::GFxValue a_dialogueMenu_mc,
			RE::GFxValue a_subtitleText,
			RE::GFxValue a_topicList,
			const std::unordered_map<std::string, TopicDisplayData>* a_topicDisplayData) noexcept;

		void Call(Params& a_params) override;

	private:
		const std::unordered_map<std::string, TopicDisplayData>* topicDisplayData{ nullptr };

		RE::GFxValue dialogueMenu_mc;
		RE::GFxValue subtitleText;
		RE::GFxValue topicList;
	};

	class MoveSelectionUpFunctionHandler final : public RE::GFxFunctionHandler
	{
	public:
		static void Install(
			const RE::DialogueMenu* a_dialogueMenu,
			RE::GFxValue a_dialogueMenu_mc,
			RE::GFxValue a_subtitleText,
			RE::GFxValue a_topicList,
			const std::unordered_map<std::string, TopicDisplayData>* a_topicDisplayData) noexcept;

		void Call(Params& a_params) override;

	private:
		const std::unordered_map<std::string, TopicDisplayData>* topicDisplayData{ nullptr };

		RE::GFxValue dialogueMenu_mc;
		RE::GFxValue subtitleText;
		RE::GFxValue topicList;
	};

	class MoveSelectionDownFunctionHandler final : public RE::GFxFunctionHandler
	{
	public:
		static void Install(
			const RE::DialogueMenu* a_dialogueMenu,
			RE::GFxValue a_dialogueMenu_mc,
			RE::GFxValue a_subtitleText,
			RE::GFxValue a_topicList,
			const std::unordered_map<std::string, TopicDisplayData>* a_topicDisplayData) noexcept;

		void Call(Params& a_params) override;

	private:
		const std::unordered_map<std::string, TopicDisplayData>* topicDisplayData{ nullptr };

		RE::GFxValue dialogueMenu_mc;
		RE::GFxValue subtitleText;
		RE::GFxValue topicList;
	};
}