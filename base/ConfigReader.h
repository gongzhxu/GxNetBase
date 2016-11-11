#ifndef _CONFIG_READER_H_
#define _CONFIG_READER_H_

#include <map>
#include <mutex>
#include <string>

typedef std::map<std::string, std::string> ConfigMap;

class ConfigReader
{
public:
	ConfigReader(const char * filename);
	~ConfigReader();

    int GetNameInt(const char * name, int defvalue);
    int GetNameInt(int id, const char * name, int defvalue);

    std::string GetNameStr(const char * name);
    std::string GetNameStr(int id, const char * name);
    int SetConfigValue(const char * name, const char * value);
private:
    void _LoadFile();
    int _WriteFIle();

    void _ParseLine(char * line);
    char * _TrimSpace(char * name);

    ConfigMap _cfgMap;
    std::string _cfgFile;
    std::mutex _mutex;
};

#endif /* _CONFIG_READER_H_ */
