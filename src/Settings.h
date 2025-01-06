#pragma once

class Settings final
{
public:
	static void Load();

	// [Formats]
	static inline std::string persuadeFormat;
	static inline std::string intimidateFormat;
	static inline std::string bribeFormat;

	// [CheckResults]
	static inline std::string checkSuccessString;
	static inline std::string checkFailureString;
	static inline std::string noCheckString;

	// [TagRegex]
	static inline std::regex persuadeTagRegex;
	static inline std::regex intimidateTagRegex;
	static inline std::regex bribeTagRegex;

	// Prevent instantiation
	Settings() = default;
	Settings(const Settings&) = delete;
	Settings(Settings&&) = delete;
	void operator=(Settings const&) = delete;
	void operator=(Settings&&) = delete;
	~Settings() = default;
};