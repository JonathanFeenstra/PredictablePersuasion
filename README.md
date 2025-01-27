# Predictable Persuasion
SKSE plugin for Skyrim Special Edition to display more info about persuasion/intimidation/bribe dialogue topics, such as the required speech level and the calculated outcome (success or failure), without requiring compatibility patches for mods that add new speech challenges.
The way the dialogue topic is formatted, can be customized in the plugin's [INI file](./config/PredictablePersuasion.ini).
This plugin is made using [CommonLibSSE NG](https://github.com/CharmedBaryon/CommonLibSSE-NG) to support Skyrim SE, AE, and VR.

## User requirements

* [SKSE64](https://www.nexusmods.com/skyrimspecialedition/mods/30379) or [SKSE VR](https://www.nexusmods.com/skyrimspecialedition/mods/30457)
* [Address Library for SKSE plugins](https://www.nexusmods.com/skyrimspecialedition/mods/32444) or [VR Address Library for SKSEVR](https://www.nexusmods.com/skyrimspecialedition/mods/58101)
* Adjust the settings in the INI file so the tag regex patterns match your game's language and the text colors match your UI mods