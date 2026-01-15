#pragma once

#include "Scaleform.h"

#include <RE/B/BSTEvent.h>
#include <RE/M/MenuOpenCloseEvent.h>

#include <string>
#include <unordered_map>

namespace Events
{
	class MenuOpenCloseEventSink final : public RE::BSTEventSink<RE::MenuOpenCloseEvent>
	{
	public:
		static void Install(const std::unordered_map<std::string, Scaleform::TopicDisplayData>* a_topicDisplayData) noexcept;

		static MenuOpenCloseEventSink* GetSingleton() noexcept;
		RE::BSEventNotifyControl ProcessEvent(const RE::MenuOpenCloseEvent* a_event, RE::BSTEventSource<RE::MenuOpenCloseEvent>*) override;

		MenuOpenCloseEventSink(const MenuOpenCloseEventSink&) = delete;
		MenuOpenCloseEventSink(MenuOpenCloseEventSink&&) = delete;
		void operator=(const MenuOpenCloseEventSink&) = delete;
		void operator=(MenuOpenCloseEventSink&&) = delete;

	private:
		MenuOpenCloseEventSink() : topicDisplayData(nullptr) {};

		const std::unordered_map<std::string, Scaleform::TopicDisplayData>* topicDisplayData;
	};
}