#include <iostream>
#include <nlohmann/json_fwd.hpp>
#include <vector>
#include <string>
#include <algorithm>
#include <cctype>
#include <stdexcept>
#include <fstream>
#include <curl/curl.h>
#include <nlohmann/json.hpp>

#include "updatePokedex.h"
#include "getPokedexInfo.h"

using namespace std;
using json = nlohmann::ordered_json;

struct moveInfo{
    string name;
    int level;
    string method;
};

getPokedexInfo::getPokedexInfo(updatePokedex& update)
    : update(update)
{

}

void getPokedexInfo::getMoves(const string& name, const string& generation){
    // load the pokemon
    update.getPokemon(name);
    
    // load json
    ifstream data("pokemon.json");

    if (!data.is_open()){
        throw runtime_error("Could not open pokemon.json");
    }

    json pokemonData = json::parse(data);

    data.close();

    // find the pokemon

    for (auto& pokemon : pokemonData["results"]){
        if (pokemon["name"] != name){
            continue;
        }

        vector<moveInfo> moves;
        for (auto& move : pokemon["moves"]){
            for (auto& version : move["version_group_details"]){
                if (version["version_group"]["name"] == generation){
                    moves.push_back({move["move"]["name"], version["level_learned_at"], version["move_learn_method"]["name"]});
                }
            }
        }
        sort(moves.begin(), moves.end(), [](const moveInfo& a, const moveInfo& b){
            auto priority = [](const string& method){
                if (method == "level-up") return 0;
                if (method == "machine") return 1;
                if (method == "tutor") return 2;
                return 3;
            };
            int aPrio = priority(a.method);
            int bPrio = priority(b.method);

            // sort by level up vs machine first
            if (aPrio != bPrio){
                return aPrio < bPrio;
            }
            
            // sort the level ups
            if (a.method == "level-up" && a.level != b.level){
                return a.level < b.level;
            }
            
            // all other moves sorted alphabetically
            return a.name < b.name;
        });
        
        for (const auto& move : moves){
            cout << move.name << '\t' << move.level << '\t' << move.method << endl;
        }
        return;
    }
}