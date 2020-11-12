#pragma once

class CLog
{
private:
    CLog() {}
public:

    template<typename...Args>
	static void Log(const char * format, Args...args)
    {
        printf(format, args...);
    }
};