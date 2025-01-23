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

	// [Formats]
	persuadeFormat = ini.GetValue("Formats", "sPersuadeFormat", "{0} ({1} Level {3}");
	intimidateFormat = ini.GetValue("Formats", "sIntimidateFormat", "{0} ({1})");
	bribeFormat = ini.GetValue("Formats", "sBribeFormat", "{0} (Bribe with {1})");

	// [CheckResults]
	checkSuccessString = ini.GetValue("CheckResults", "sSuccess", "Success");
	checkFailureString = ini.GetValue("CheckResults", "sFailure", "Failure");
	noCheckString = ini.GetValue("CheckResults", "sNoCheck", "No Check");

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

	// [TextColors]
	applyTextColors = ini.GetBoolValue("TextColors", "bApplyTextColors", true);
	successColor = ini.GetLongValue("TextColors", "uSuccessColor", 0x00FF00);
	failureColorNew = ini.GetLongValue("TextColors", "uFailureColorNew", 0xFF0000);
	failureColorOld = ini.GetLongValue("TextColors", "uFailureColorOld", 0x600000);
	regularColorNew = ini.GetLongValue("TextColors", "uRegularColorNew", 0xFFFFFF);
	regularColorOld = ini.GetLongValue("TextColors", "uRegularColorOld", 0x606060);
}