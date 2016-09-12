#ifndef _CONFIGFILEREADER_H_
#define _CONFIGFILEREADER_H_

#include <map>
#include <mutex>
#include <string>

typedef std::map<std::string, std::string> ConfigMap;

class ConfigFileReader
{
public:
	ConfigFileReader(const char * filename);
	~ConfigFileReader();

    char * GetConfigName(const char * name);
    char * GetConfigName(const char * name, int id);
    int SetConfigValue(const char * name, const char * value);
private:
    void _LoadFile(const char * filename);
    int _WriteFIle(const char *filename = nullptr);
    void _ParseLine(char * line);
    char * _TrimSpace(char * name);

    bool _load_ok;
    ConfigMap _config_map;
    std::string _config_file;
    std::mutex _mutex;
};



#endif /* CONFIGFILEREADER_H_ */
