#pragma once

class ServerConfig
{
public:


public:
    string statPath;
    string skillPath;
};

class ConfigManager
{
public:
    static ConfigManager& GetInstance() {
        static ConfigManager instance;
        return instance;
    }

    ServerConfig GetServerConfig() { return _config; }
    void LoadConfig();

private:
    ConfigManager() = default;
    ConfigManager(const ConfigManager&) = delete;
    ConfigManager& operator=(const ConfigManager&) = delete;


    void SetServerConfig(ServerConfig config) { _config = config; }

public:
    ServerConfig _config;
    
private:

};

