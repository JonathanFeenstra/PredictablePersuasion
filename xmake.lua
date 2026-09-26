-- include subprojects
includes("lib/commonlibsse-ng")

-- set project constants
set_project("Predictable Persuasion")
set_version("1.0.10")
set_license("GPL-3.0-or-later WITH Modding Exception AND GPL-3.0 Linking Exception (with Corresponding Source)")
set_languages("c++23")
set_warnings("allextra")

-- add common rules
add_rules("mode.debug", "mode.releasedbg")
add_rules("plugin.vsxmake.autoupdate")

-- set configs
set_config("rex_ini", true)

-- define targets
target("PredictablePersuasion")
    add_rules("commonlibsse-ng.plugin", {
        name = "PredictablePersuasion",
        author = "Jonathan Feenstra",
        description = "SKSE plugin for Skyrim Special Edition to display more info about persuasion/intimidation/bribe dialogue topics"
    })
	
    -- add src files
    add_files("src/**.cpp")
    add_headerfiles("src/**.h")
    add_includedirs("src")
    set_pcxxheader("src/PCH.h")

    -- add extra files
	add_extrafiles(".clang-format")
	
	add_installfiles("config/(**)")
	
