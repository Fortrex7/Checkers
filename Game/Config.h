#pragma once
#include <fstream>
#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include "../Models/Project_path.h"

class Config
{
  public:
    Config()
    {
        reload();
    }

    //Загрузка настроек из settings.json
    void reload()
    {
        std::ifstream fin(project_path + "settings.json"); 
        fin >> config;
        fin.close();
    }
    //Оператор круглые скобки позволяет принимать по setting_dir и setting_name нужную настройку из settings
    auto operator()(const string &setting_dir, const string &setting_name) const
    {
        return config[setting_dir][setting_name];
    }

  private:
    json config;
};
