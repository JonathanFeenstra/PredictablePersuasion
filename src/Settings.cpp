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

#include "Settings.h"

#include "SimpleIni.h"

void Settings::Load()
{
	CSimpleIniA ini;

	ini.SetUnicode();
	ini.SetQuotes();
	ini.SetSpaces();
	ini.LoadFile(R"(.\Data\SKSE\Plugins\PredictablePersuasion.ini)");

	// [TopicFormats]
	applyTopicFormatting = ini.GetBoolValue("TopicFormats", "bApplyTopicFormatting", true);
	persuadeTopicFormat = ini.GetValue("TopicFormats", "sPersuadeTopicFormat", "{0} ({1} Level {3}");
	intimidateTopicFormat = ini.GetValue("TopicFormats", "sIntimidateTopicFormat", "{0} ({1})");
	bribeTopicFormat = ini.GetValue("TopicFormats", "sBribeTopicFormat", "{0} (Bribe with {1})");

	// [Subtitles]
	const auto showSubtitlesValue = ini.GetLongValue("Subtitles", "uShowSubtitles", static_cast<long>(SHOW_SUBTITLES::kForAllSpeechChecks));
	switch (showSubtitlesValue) {
	case 0:
		showSubtitles = SHOW_SUBTITLES::kNever;
		break;
	case 1:
		showSubtitles = SHOW_SUBTITLES::kOnlyForNoCheck;
		break;
	case 2:
		showSubtitles = SHOW_SUBTITLES::kForAllSpeechChecks;
		break;
	default:
		showSubtitles = SHOW_SUBTITLES::kForAllSpeechChecks;
		logger::error("Invalid value for uShowSubtitles: {}", showSubtitlesValue);
	}

	subtitleColor = ini.GetLongValue("Subtitles", "uSubtitleColor", 0xA3A3A3);
	persuadeSubtitleFormat = ini.GetValue("Subtitles", "sPersuadeSubtitleFormat", "{4}");
	intimidateSubtitleFormat = ini.GetValue("Subtitles", "sIntimidateSubtitleFormat", "{4}");
	bribeSubtitleFormat = ini.GetValue("Subtitles", "sBribeSubtitleFormat", "{4}");

	// [CheckResults]
	checkSuccessText = ini.GetValue("CheckResults", "sSuccessText", "Success");
	checkFailureText = ini.GetValue("CheckResults", "sFailureText", "Failure");
	noCheckText = ini.GetValue("CheckResults", "sNoCheckText", "No Check");

	// [TagRegex]
	try {
		persuadeTagRegex = ini.GetValue("TagRegex", "sPersuadeTagRegex", " (\\(Persuade\\))$");
		intimidateTagRegex = ini.GetValue("TagRegex", "sIntimidateTagRegex", " (\\(Intimidate\\))$");
		bribeTagRegex = ini.GetValue("TagRegex", "sBribeTagRegex", " (\\(\\d+ gold\\))$");
	} catch (const std::regex_error& e) {
		persuadeTagRegex = " (\\(Persuade\\))$";
		intimidateTagRegex = " (\\(Intimidate\\))$";
		bribeTagRegex = " (\\(\\d+ gold\\))$";
		logger::error("Failed to compile regex: {}", e.what());
	}

	// [TopicColors]
	applyTopicColors = ini.GetBoolValue("TopicColors", "bApplyTopicColors", true);
	successColor = ini.GetLongValue("TopicColors", "uSuccessColor", 0x00FF00);
	failureColorNew = ini.GetLongValue("TopicColors", "uFailureColorNew", 0xFF0000);
	failureColorOld = ini.GetLongValue("TopicColors", "uFailureColorOld", 0x600000);
	noCheckColorNew = ini.GetLongValue("TopicColors", "uNoCheckColorNew", 0xFFFF00);
	noCheckColorOld = ini.GetLongValue("TopicColors", "uNoCheckColorOld", 0x606000);
	regularColorNew = ini.GetLongValue("TopicColors", "uRegularColorNew", 0xFFFFFF);
	regularColorOld = ini.GetLongValue("TopicColors", "uRegularColorOld", 0x606060);

	// [Requirements]
	requirePerk = ini.GetBoolValue("Requirements", "bRequirePerk", false);
	requiredPerkFormID = ini.GetLongValue("Requirements", "uRequiredPerkFormID", 0x001090A2);
}