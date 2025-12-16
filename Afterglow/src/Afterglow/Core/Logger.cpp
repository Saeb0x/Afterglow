#include "agpch.h"
#include "Logger.h"

namespace Afterglow
{
	Logger::~Logger()
	{
		if (m_OutputFileStream.is_open())
			m_OutputFileStream.close();
	}

	Logger& Logger::GetInstance()
	{
		static Logger instance;
		return instance;
	}

	void Logger::SetLogFile(const std::string& filepath)
	{
		std::lock_guard<std::mutex> lock(m_Mutex);

		if (m_OutputFileStream.is_open())
			m_OutputFileStream.close();

		std::filesystem::path path(filepath);
		if (path.has_parent_path())
			std::filesystem::create_directories(path.parent_path());

		m_OutputFileStream.open(filepath, std::ios::out | std::ios::app);
		if (!m_OutputFileStream.is_open())
			fmt::print(stderr, fg(fmt::color::red), "Failed to open log file: {}\n", filepath);
	}

	void Logger::EnableAllCategories()
	{
		std::lock_guard<std::mutex> lock(m_Mutex);
		b_AllCategoriesEnabled = true;
		m_EnabledCategories.clear();
	}

	void Logger::DisableAllCategories()
	{
		std::lock_guard<std::mutex> lock(m_Mutex);
		b_AllCategoriesEnabled = false;
		m_EnabledCategories.clear();
	}

	void Logger::EnableCategory(const char* category)
	{
		std::lock_guard<std::mutex> lock(m_Mutex);
		b_AllCategoriesEnabled = false;
		m_EnabledCategories.insert(category);
	}

	void Logger::DisableCategory(const char* category)
	{
		std::lock_guard<std::mutex> lock(m_Mutex);
		m_EnabledCategories.erase(category);
	}

	bool Logger::IsCategoryEnabled(const char* category) const
	{
		if (b_AllCategoriesEnabled)
			return true;

		return m_EnabledCategories.find(category) != m_EnabledCategories.end();
	}

	std::string Logger::FormatLogMessage(const char* category, LogVerbosity verbosity, const char* file, uint32_t line, const std::string& logMessage)
	{
		auto now = std::chrono::system_clock::now();
		auto time = std::chrono::system_clock::to_time_t(now);
		auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

		std::filesystem::path filepath(file);
		std::string filename = filepath.filename().string();

		// Thread-safe time conversion.
		std::tm localTime = {};
#ifdef _WIN32
		localtime_s(&localTime, &time);
#else
		localtime_r(&time, &localTime);
#endif

		// Format: [HH:MM:SS.mmm] [CATEGORY] [VERBOSITY] [FILE:LINE] LogMessage.
		return fmt::format("[{:02d}:{:02d}:{:02d}.{:03d}] [{}] [{}] [{}:{}] {}",
			localTime.tm_hour,
			localTime.tm_min,
			localTime.tm_sec,
			ms.count(),
			category,
			GetVerbosityString(verbosity),
			filename,
			line,
			logMessage);
	}

	void Logger::PrintToConsole(LogVerbosity verbosity, const std::string& logMessageFormatted)
	{
		fmt::color color = GetVerbosityColor(verbosity);

		if (verbosity == LogVerbosity::Error)
			fmt::print(stderr, fg(color), "{}\n", logMessageFormatted);
		else
			fmt::print(fg(color), "{}\n", logMessageFormatted);
	}

	const char* Logger::GetVerbosityString(LogVerbosity verbosity)
	{
		switch (verbosity)
		{
		case LogVerbosity::Trace:		return "TRACE";
		case LogVerbosity::Info:		return "INFO";
		case LogVerbosity::Warning:		return "WARNING";
		case LogVerbosity::Error:		return "ERROR";
		default:						return "UNKNOWN";
		}
	}

	fmt::color Logger::GetVerbosityColor(LogVerbosity verbosity)
	{
		switch (verbosity)
		{
		case LogVerbosity::Trace:		return fmt::color::gray;
		case LogVerbosity::Info:		return fmt::color::green;
		case LogVerbosity::Warning:		return fmt::color::yellow;
		case LogVerbosity::Error:		return fmt::color::red;
		default:						return fmt::color::white;
		}
	}
}