#include "ConfigFileReader.h"

#include <stdio.h>
#include <string.h>

#include "BaseUtil.h"

ConfigFileReader::ConfigFileReader(const char * filename)
{
	_LoadFile(filename);
}

ConfigFileReader::~ConfigFileReader()
{
}

char* ConfigFileReader::GetConfigName(const char* name)
{
	if (!_load_ok)
		return nullptr;

	char* value = nullptr;
	ConfigMap::iterator it = _config_map.find(name);
	if (it != _config_map.end()) {
		value = (char*)it->second.c_str();
	}

	return value;
}

int ConfigFileReader::SetConfigValue(const char* name, const char* value)
{
    if(!_load_ok)
        return -1;

    ConfigMap::iterator it = _config_map.find(name);
    if(it != _config_map.end())
    {
        it->second = value;
    }
    else
    {
        _config_map.insert(std::make_pair(name, value));
    }
    return _WriteFIle();
}
void ConfigFileReader::_LoadFile(const char* filename)
{
    _config_file.clear();
    _config_file.append(filename);
	FILE* fp = fopen(filename, "r");
	if (!fp)
	{
		LOG_FATAL("can not open %s,errno = %d", filename,errno);
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
	_load_ok = true;
}

int ConfigFileReader::_WriteFIle(const char* filename)
{
   FILE* fp = nullptr;
   if(filename == nullptr)
   {
       fp = fopen(_config_file.c_str(), "w");
   }
   else
   {
       fp = fopen(filename, "w");
   }
   if(fp == nullptr)
   {
       return -1;
   }

   char szPaire[128];
   ConfigMap::iterator it = _config_map.begin();
   for (; it != _config_map.end(); it++)
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
void ConfigFileReader::_ParseLine(char* line)
{
	char* p = strchr(line, '=');
	if (p == nullptr)
		return;

	*p = 0;
	char* key =  _TrimSpace(line);
	char* value = _TrimSpace(p + 1);
	if (key && value)
	{
		_config_map.insert(std::make_pair(key, value));
	}
}

char* ConfigFileReader::_TrimSpace(char* name)
{
	// remove starting space or tab
	char* start_pos = name;
	while((*start_pos == ' ') || (*start_pos == '\t'))
	{
		start_pos++;
	}

	if(strlen(start_pos) == 0)
		return nullptr;

	// remove ending space or tab
	char* end_pos = name + strlen(name) - 1;
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
