#pragma once

#include <string>
#include <nlohmann/json.hpp>

using namespace std;
using json = nlohmann::ordered_json;

class updatePokedex{
    private:
        const string APIurl;

    public:
        updatePokedex();
        ~updatePokedex();

        void getPokedex();
        void getGeneration();
        void getPokemon(const string& name);
        json getLocation(const string& url);
};