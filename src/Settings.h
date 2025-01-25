#pragma once

class Settings final
{
public:
	static void Load();

	// [TopicFormats]
	static inline bool applyTopicFormatting;
	static inline std::string persuadeTopicFormat;
	static inline std::string intimidateTopicFormat;
	static inline std::string bribeTopicFormat;

	// [Subtitles]
	enum class SHOW_SUBTITLES : uint8_t
	{
		kNever = 0,
		kOnlyForNoCheck = 1,
		kForAllSpeechChecks = 2,
	};

	static inline SHOW_SUBTITLES showSubtitles;
	static inline std::string persuadeSubtitleFormat;
	static inline std::string intimidateSubtitleFormat;
	static inline std::string bribeSubtitleFormat;

	// [CheckResults]
	static inline std::string checkSuccessText;
	static inline std::string checkFailureText;
	static inline std::string noCheckText;

	// [TagRegex]
	static inline std::regex persuadeTagRegex;
	static inline std::regex intimidateTagRegex;
	static inline std::regex bribeTagRegex;

	// [TopicColors]
	static inline bool applyTopicColors;
	static inline std::uint32_t successColor;
	static inline std::uint32_t failureColorNew;
	static inline std::uint32_t failureColorOld;
	static inline std::uint32_t noCheckColorNew;
	static inline std::uint32_t noCheckColorOld;
	static inline std::uint32_t regularColorNew;
	static inline std::uint32_t regularColorOld;

	// Prevent instantiation
	Settings() = default;
	Settings(const Settings&) = delete;
	Settings(Settings&&) = delete;
	void operator=(Settings const&) = delete;
	void operator=(Settings&&) = delete;
	~Settings() = default;
};