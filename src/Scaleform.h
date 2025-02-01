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

	class DialogueMenuUI final
	{
	public:
		static void InstallHooks(const std::unordered_map<std::string, TopicDisplayData>* a_topicDisplayData) noexcept;
		static DialogueMenuUI* GetSingleton() noexcept;

		void CopyOriginalTopicListFunction(const char* a_functionName) noexcept;
		void ReplaceFunction(const char* a_functionName, RE::GFxFunctionHandler* a_newHandler) noexcept;
		void ReplaceTopicListFunction(const char* a_functionName, RE::GFxFunctionHandler* a_newHandler) noexcept;

		void ColorText(RE::GFxValue a_textField, bool a_topicIsNew) noexcept;

		bool IsTopicListShown() noexcept;

		void ShowGameSubtitle(const RE::GFxValue a_strText) noexcept;
		void ShowModSubtitle() noexcept;

		DialogueMenuUI(const DialogueMenuUI&) = delete;
		DialogueMenuUI(DialogueMenuUI&&) = delete;
		DialogueMenuUI& operator=(const DialogueMenuUI&) = delete;
		DialogueMenuUI& operator=(DialogueMenuUI&&) = delete;

	private:
		DialogueMenuUI() {};

		RE::GFxValue getHiglightedEntry() noexcept;

		const std::unordered_map<std::string, TopicDisplayData>* topicDisplayData;
		RE::GPtr<RE::GFxMovieView> dialogueMenuUIMovie;
		RE::GFxValue dialogueMenu_mc;
		RE::GFxValue subtitleText;
		RE::GFxValue defaultSubtitleColor;
		RE::GFxValue topicList;
		bool usesBetterDialogueControls;
		bool isGameSubtitle;
	};

	class SetEntryTextFunctionHandler final : public RE::GFxFunctionHandler
	{
	public:
		static void Install(DialogueMenuUI* a_dialogueMenuUI) noexcept;

		void Call(Params& a_params) override;

	private:
		DialogueMenuUI* dialogueMenuUI;
	};

	class ShowDialogueTextFunctionHandler final : public RE::GFxFunctionHandler
	{
	public:
		static void Install(DialogueMenuUI* a_dialogueMenuUI) noexcept;

		void Call(Params& a_params) override;

	private:
		DialogueMenuUI* dialogueMenuUI;
	};

	class DoSetSelectedIndexFunctionHandler final : public RE::GFxFunctionHandler
	{
	public:
		static void Install(DialogueMenuUI* a_dialogueMenuUI) noexcept;

		void Call(Params& a_params) override;

	private:
		DialogueMenuUI* dialogueMenuUI;
	};

	class MoveSelectionUpFunctionHandler final : public RE::GFxFunctionHandler
	{
	public:
		static void Install(DialogueMenuUI* a_dialogueMenuUI) noexcept;
		void Call(Params& a_params) override;

	private:
		DialogueMenuUI* dialogueMenuUI;
	};

	class MoveSelectionDownFunctionHandler final : public RE::GFxFunctionHandler
	{
	public:
		static void Install(DialogueMenuUI* a_dialogueMenuUI) noexcept;
		void Call(Params& a_params) override;

	private:
		DialogueMenuUI* dialogueMenuUI;
	};
}