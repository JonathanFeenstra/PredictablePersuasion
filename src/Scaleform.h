#pragma once

namespace Scaleform
{
	class SetEntryTextFunctionHandler final : public RE::GFxFunctionHandler
	{
	public:
		static void Install(
			const std::set<std::string>* a_successTopics,
			const std::set<std::string>* a_failureTopics,
			const std::set<std::string>* a_noCheckTopics) noexcept;

		void Call(Params& a_params) override;

	private:
		const std::set<std::string>* successTopics;
		const std::set<std::string>* failureTopics;
		const std::set<std::string>* noCheckTopics;

		void colorText(RE::GFxValue& a_textField, bool a_faded) noexcept;
	};
}