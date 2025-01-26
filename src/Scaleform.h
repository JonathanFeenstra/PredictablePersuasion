#pragma once

namespace Scaleform
{
	// the ActionScript 2 code of the dialogue menu only has access to the text of the topics, so additional data needs to be passed
	struct TopicDisplayData final
	{
		std::uint32_t oldColor;
		std::uint32_t newColor;
		std::string subtitle;
	};

	void InstallHooks(const std::unordered_map<std::string, TopicDisplayData>* a_topicDisplayData) noexcept;

	class SetEntryTextFunctionHandler final : public RE::GFxFunctionHandler
	{
	public:
		static void Install(
			const RE::DialogueMenu* a_dialogueMenu,
			RE::GFxValue a_topicList,
			const std::unordered_map<std::string, TopicDisplayData>* a_topicDisplayData) noexcept;

		void Call(Params& a_params) override;

	private:
		const std::unordered_map<std::string, TopicDisplayData>* topicDisplayData;

		void colorText(RE::GFxValue a_textField, bool a_topicIsNew) noexcept;
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
		const std::unordered_map<std::string, TopicDisplayData>* topicDisplayData;

		RE::GFxValue dialogueMenu_mc;
		RE::GFxValue subtitleText;
		RE::GFxValue topicList;
	};
}