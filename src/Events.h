#pragma once

namespace Events
{
	class MenuOpenCloseEventSink final : public RE::BSTEventSink<RE::MenuOpenCloseEvent>
	{
	public:
		static void Install(
			const std::set<std::string>* a_successTopics,
			const std::set<std::string>* a_failureTopics,
			const std::set<std::string>* a_noCheckTopics) noexcept;

		static MenuOpenCloseEventSink* GetSingleton() noexcept;
		RE::BSEventNotifyControl ProcessEvent(const RE::MenuOpenCloseEvent* a_event, RE::BSTEventSource<RE::MenuOpenCloseEvent>*) override;

		MenuOpenCloseEventSink(const MenuOpenCloseEventSink&) = delete;
		MenuOpenCloseEventSink(MenuOpenCloseEventSink&&) = delete;
		void operator=(const MenuOpenCloseEventSink&) = delete;
		void operator=(MenuOpenCloseEventSink&&) = delete;

	private:
		MenuOpenCloseEventSink() {};

		const std::set<std::string>* successTopics;
		const std::set<std::string>* failureTopics;
		const std::set<std::string>* noCheckTopics;
	};
}