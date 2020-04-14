
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <ctime>
#include <iomanip>
#include <vector>
#include <memory>

enum Level
{
    NONE = 0,
    FATAL = 1,
    ERROR = 2,
    WARNING = 3,
    INFO = 4,
    DEBUG = 5,
    VERBOSE = 6
};

class LogSink
{
protected: 
	Level m_minLevel;
public:
	LogSink(const Level minLevel) : m_minLevel(minLevel) {}
	virtual ~LogSink() {}
	virtual void write(const Level level, const std::string &str) = 0;
};

class ConsoleSink : public LogSink
{
public:
	ConsoleSink(const Level minLevel) : LogSink(minLevel) {}

	virtual void write(const Level level, const std::string &str)
	{
		if (level < m_minLevel)
			return;
		if (level == Level::WARNING)
			std::cout << "[WARNING]: ";
        else if (level == Level::INFO)
			std::cout << "[INGO]: ";
        else if (level == Level::DEBUG)
			std::cout << "[DEBUG]: ";
        else if (level == Level::VERBOSE)
			std::cout << "[VERBOSE]: ";
		else if (level == Level::ERROR)
			std::cerr << "[ERROR]: ";
        
		std::cout << str << std::endl;
	}
};

class FileSink : public LogSink
	{
	protected: 
		std::ofstream m_file;

	public:
		FileSink(const Level minLevel, const std::string &fileName)
			: LogSink(minLevel)
		{
			m_file.open(fileName.c_str(), std::ios::out);
			if (m_file.fail())
				std::cerr << "Failed to open file: " << fileName << "\n";
		}

		virtual ~FileSink()
		{
			m_file.close();
		}

		virtual void write(const Level level, const std::string &str)
		{
			if (level < m_minLevel)
				return;

			// print time
			time_t t = time(0);   // get time now
			struct tm * now = localtime(&t);
			m_file << "[" << (now->tm_year + 1900) << '-' 
					<< std::setfill('0') << std::setw(2)
					<< (now->tm_mon + 1) << '-'
					<< std::setfill('0') << std::setw(2)
					<< now->tm_mday << " "
					<< std::setfill('0') << std::setw(2) 
					<< now->tm_hour << ":" 
					<< std::setfill('0') << std::setw(2) 
					<< now->tm_min << ":" 
					<< std::setfill('0') 
					<< std::setw(2) << now->tm_sec << "] ";

			// print level
			if (level == Level::DEBUG)
				m_file << "Debug:   ";
			else if (level == Level::INFO)
				m_file << "Info:    ";
			else if (level == Level::WARNING)
				m_file << "Warning: ";
			else if (level == Level::ERROR)
				m_file << "Error:   ";

			m_file << str << std::endl;
			m_file.flush();
		}
	};

class Logger
{
public: 
	Logger() {}
	~Logger() 
	{ 
		m_sinks.clear();  
	}

protected:
	std::vector<std::unique_ptr<LogSink>> m_sinks;

public:

	void addSink(std::unique_ptr<LogSink> sink)
	{
		m_sinks.push_back(std::move(sink));
	}
	void write(const Level level, const std::string &str)
	{
		for (unsigned int i = 0; i < m_sinks.size(); i++)
			m_sinks[i]->write(level, str);
	}
};

class LogStream
{
 public:
	LogStream(Logger *logger, const Level level) : m_logger(logger), m_level(level) {}
	template <typename T>
	LogStream& operator<<(T const & value)
	{
		m_buffer << value;
		return *this;
	}
	~LogStream() { m_logger->write(m_level, m_buffer.str()); }
protected:
	Level m_level;
	Logger *m_logger;
	std::ostringstream m_buffer;
};

extern Logger logger;

#define LOG_DEBUG LogStream(&logger, Level::DEBUG)
#define LOG_INFO LogStream(&logger, Level::INFO)
#define LOG_WARN LogStream(&logger, Level::WARN)
#define LOG_ERR LogStream(&logger, Level::ERR)
#define INIT_LOGGING Logger logger;