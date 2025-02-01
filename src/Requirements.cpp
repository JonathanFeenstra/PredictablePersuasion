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

#include "Requirements.h"

#include "Settings.h"

namespace Requirements
{
	bool AreRequirementsMet() noexcept
	{
		if (!Settings::requirePerk) {
			return true;
		}

		const auto player = RE::PlayerCharacter::GetSingleton();
		if (!player) {
			return false;
		}

		if (const auto requiredPerkForm = RE::TESForm::LookupByID(Settings::requiredPerkFormID)) {
			const auto requiredPerk = requiredPerkForm->As<RE::BGSPerk>();
			if (!requiredPerk) {
				logger::error("Form ID {} is not a perk", Settings::requiredPerkFormID);
				return false;
			}

			return player->HasPerk(requiredPerk);
		}

		logger::error("Failed to find form with Form ID {}", Settings::requiredPerkFormID);
		return false;
	}
}