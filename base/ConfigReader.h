#ifndef _CONFIG_READER_H_
#define _CONFIG_READER_H_

#include <map>
#include <mutex>
#include <string>

typedef std::map<std::string, std::string> ConfigMap;

/*
    configure reader
 */
class ConfigReader
{
public:
	ConfigReader(const char * filename);
	~ConfigReader();

    int GetNameInt(const char * name, int defvalue = 0);
    int GetNameInt(int id, const char * name, int defvalue = 0);

    std::string GetNameStr(const char * name, const char * defvalue = "");
    std::string GetNameStr(int id, const char * name, const char * defvalue = "");
    int SetConfigValue(const char * name, const char * value);
private:
    void _LoadFile();
    int _WriteFIle();

    void _ParseLine(char * line);
    char * _TrimSpace(char * name);

    ConfigMap cfgMap_;
    std::string cfgFile_;
    std::mutex mutex_;
};

#endif /* _CONFIG_READER_H_ */
