#include "ConfigReader.h"

#include <stdio.h>
#include <string.h>

#include "BaseUtil.h"

ConfigReader::ConfigReader(const char * filename):
    _cfgFile(filename)
{
    std::unique_lock<std::mutex> lock(_mutex);
    _LoadFile();
}

ConfigReader::~ConfigReader()
{
}

int ConfigReader::GetNameInt(const char * name, int defvalue)
{
    int value = defvalue;
    std::string strValue = GetNameStr(name);
    if(!strValue.empty())
    {
        value = atoi(strValue.c_str());
    }

    return value;
}

int ConfigReader::GetNameInt(int id, const char * name, int defvalue)
{
    int value = defvalue;
    std::string strValue = GetNameStr(id, name);
    if(!strValue.empty())
    {
        value = atoi(strValue.c_str());
    }

    return value;
}

std::string ConfigReader::GetNameStr(const char * name, const char * defvalue)
{
	std::string value = defvalue;
	{
        std::unique_lock<std::mutex> lock(_mutex);
        ConfigMap::iterator it = _cfgMap.find(name);
        if (it != _cfgMap.end())
        {
            value = (char*)it->second.c_str();
        }
	}

	return value;
}

std::string ConfigReader::GetNameStr( int id, const char * name, const char * defvalue)
{
    std::string strName;
    base::sprintfex(strName, "%s%d", name, id);
    return GetNameStr(strName.c_str(), defvalue);
}

int ConfigReader::SetConfigValue(const char* name, const char * value)
{
    std::unique_lock<std::mutex> lock(_mutex);

    ConfigMap::iterator it = _cfgMap.find(name);
    if(it != _cfgMap.end())
    {
        it->second = value;
    }
    else
    {
        _cfgMap.insert(std::make_pair(name, value));
    }
    return _WriteFIle();
}

void ConfigReader::_LoadFile()
{
	FILE* fp = fopen(_cfgFile.c_str(), "r");
	if (!fp)
	{
		fprintf(stderr,
                "open configfile=%s,error=%s\n",
                _cfgFile.c_str(),
                strerror(errno));
		return;
	}

	for (;;)
	{
        char buf[256] = {0};
		char* p = fgets(buf, 256, fp);
		if (!p)
			break;

		size_t len = strlen(buf);
		if (buf[len - 1] == '\n')
			buf[len - 1] = 0;			// remove \n at the end
        if (buf[len - 2] == '\r')
            buf[len - 2] = 0;

		char* ch = strchr(buf, '#');	// remove string start with #
		if (ch)
			*ch = 0;

		if (strlen(buf) == 0)
			continue;

		_ParseLine(buf);
	}

	fclose(fp);
}

int ConfigReader::_WriteFIle()
{
   FILE * fp = fopen(_cfgFile.c_str(), "w");
   if(fp == nullptr)
   {
       return -1;
   }

   char szPaire[128] = {0};
   ConfigMap::iterator it = _cfgMap.begin();
   for(; it != _cfgMap.end(); it++)
   {
        memset(szPaire, 0, sizeof(szPaire));
        snprintf(szPaire, sizeof(szPaire), "%s=%s\n", it->first.c_str(), it->second.c_str());
        uint32_t ret =  fwrite(szPaire, strlen(szPaire),1,fp);
        if(ret != 1)
        {
            fclose(fp);
            return -1;
        }
   }
   fclose(fp);
   return 0;
}
void ConfigReader::_ParseLine(char * line)
{
	char * p = strchr(line, '=');
	if(p == nullptr)
		return;

	*p = 0;
	char* key =  _TrimSpace(line);
	char* value = _TrimSpace(p + 1);
	if(key && value)
	{
		_cfgMap.insert(std::make_pair(key, value));
	}
}

char * ConfigReader::_TrimSpace(char * name)
{
	// remove starting space or tab
	char * start_pos = name;
	while((*start_pos == ' ') || (*start_pos == '\t'))
	{
		start_pos++;
	}

	if(strlen(start_pos) == 0)
		return nullptr;

	// remove ending space or tab
	char * end_pos = name + strlen(name) - 1;
	while((*end_pos == ' ') || (*end_pos == '\t'))
	{
		*end_pos = 0;
		end_pos--;
	}

	int len = (int)(end_pos - start_pos) + 1;
	if (len <= 0)
		return nullptr;

	return start_pos;
}
