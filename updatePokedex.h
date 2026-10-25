#pragma once

#include <string>
#include <nlohmann/json.hpp>

using namespace std;
using json = nlohmann::ordered_json;

class updatePokedex{
    private:
        const string APIurl;
        
        json pokedexData;
        void loadPokedex();
        void savePokedex();

    public:
        updatePokedex();
        ~updatePokedex();

        const json& getData() const;
        void getPokedex();

        void getGeneration();
        void getPokemon(const string& name);
        json getLocation(const string& url) const;
        json getMoves(json& pokemonData) const;
};