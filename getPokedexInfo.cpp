#include <iostream>
#include <nlohmann/json_fwd.hpp>
#include <vector>
#include <string>
#include <unordered_set>
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

static unordered_map<string, unordered_set<string>> generationMap = {
    {"red-blue", {"red", "blue"}},
    {"red", {"red"}},
    {"blue", {"blue"}},
    {"yellow", {"yellow"}},
    {"gold-silver", {"gold", "silver"}},
    {"gold", {"gold"}},
    {"silver", {"silver"}},
    {"crystal", {"crystal"}},
    {"ruby-sapphire", {"ruby", "sapphire"}},
    {"ruby", {"ruby"}},
    {"sapphire", {"sapphire"}},
    {"emerald", {"emerald"}},
    {"firered-leafgreen", {"firered", "leafgreen"}},
    {"firered", {"firered"}},
    {"leafgreen", {"leaftreen"}},
    {"diamond-pearl", {"diamond", "pearl"}},
    {"diamond", {"diamond"}},
    {"pearl", {"pearl"}},
    {"platinum", {"platinum"}},
    {"heartgold-soulsilver", {"heartgold", "soulsilver"}},
    {"black-white", {"black", "white"}},
    {"black", {"black"}},
    {"white", {"white"}},
    {"colosseum", {"colosseum"}},
    {"xd", {"xd"}},
    {"black-2-white-2", {"black2", "white2"}},
    {"black-2", {"black2"}},
    {"white-2", {"white2"}},
    {"x-y", {"x", "y"}},
    {"omega-ruby-alpha-sapphire", {"omegaruby", "alphasapphire", "6"}},
    {"omega-ruby", {"omegaruby"}},
    {"alpha-sapphire", {"alphasapphire"}},
    {"sun-moon", {"sun", "moon"}},
    {"ultra-sun-ultra-moon", {"ultrasun", "ultramoon"}},
    {"ultra-sun", {"ultrasun"}},
    {"ultra-moon", {"ultramoon"}},
    {"lets-go-pikachu-lets-go-eevee", {"letsgopikachu", "letsgoeevee", "pikachu", "eevee"}},
    {"lets-go-pikachu", {"letsgopikachu", "pikachu"}},
    {"lets-go-eevee", {"letsgoeevee", "eevee"}},
    {"sword-shield", {"sword", "shield"}},
    {"sword", {"sword"}},
    {"shield", {"shield"}},
    {"brilliant-diamond-shining-pearl", {"brilliantdiamond", "shiningpearl"}},
    {"brilliant-diamond", {"brilliantdiamond"}},
    {"shining-pearl", {"shiningpearl"}},
    {"scarlet-violet", {"scarlet", "violet"}},
    {"scarlet", {"scarlet"}},
    {"violet", {"violet"}}
};


// normalize the generation name
string normalize(const string& input){
    string gen;

    for (unsigned char c: input){
        if (isspace(c) || c == '-'){
            continue;
        }
        gen += tolower(c);
    }
    return gen;
}

// find the matching key
string findGen(const string& input){
    // normalize the gen name
    string genName = normalize(input);

    for (const auto& [apiName, gen] : generationMap){
        if (normalize(apiName) == genName){
            return apiName;
        }

        if (gen.find(genName) != gen.end()){
            return apiName;
        }
    }

    throw invalid_argument("Unknown generation");
}

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
    
    string genName = findGen(generation);
    
    // load json
    ifstream data("pokemon.json");

    if (!data.is_open()){
        throw runtime_error("Could not open pokemon.json");
    }

    json pokemonData = json::parse(data);

    data.close();

    json& pokemon = pokemonData[name];

    vector<moveInfo> moves;
    for (auto& move : pokemon["moves"]){
        for (auto& version : move["version_group_details"]){
            // get the generation
            if (version["version_group"]["name"] == genName){
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


void getPokedexInfo::getAbility(const string& name){
    // update pokedex
    update.getPokemon(name);

    ifstream data("pokemon.json");

    if (!data.is_open()){
        throw runtime_error("Could not open Pokemon.json");
    }

    json pokemonData = json::parse(data);

    data.close();

    json& pokemon = pokemonData[name];
        
    for (auto& ability : pokemon["abilities"]){
        if (ability["is_hidden"] == false){
            cout << ability["ability"]["name"] << endl;
        } else {
            cout << ability["ability"]["name"] << "\t" << " Hidden ability (Gen V+)" << endl;
        }
    }
    return;
}

void getPokedexInfo::getLocations(const string& name, const string& generation){
    // update pokedex
    update.getPokemon(name);
    update.getLocation(name);

    string genName = findGen(generation);

    ifstream data("pokemon.json");

    if (!data.is_open()){
        throw runtime_error("Could not open pokemon.json");
    }

    json pokemonData = json::parse(data);

    data.close();

    json& pokemon = pokemonData[name];

    for (auto& location : pokemon["location_area_encounters"]){
        json& version_details = location["version_details"];
        for (auto& version : version_details){
            json& versionName = version["version"]["name"];
            if (versionName == genName){
                int min_level = 101;
                int max_level = 0;
                int min_chance = 101;
                int max_chance = 0;
                unordered_set<string> encounter_methods;

                for (auto& method : version["encounter_details"]){
                    if (method["min_level"] < min_level){
                        min_level = method["min_level"];
                    }
                    if (method["max_level"] > max_level){
                        max_level = method["max_level"];
                    }

                    if (method["chance"] < min_chance){
                        min_chance = method["chance"];
                    }
                    if (method["chance"] > max_chance){
                        max_chance = method["chance"];
                    }
                    encounter_methods.insert(method["method"]["name"]);
                }

                cout << location["location_area"]["name"] << "\t";

                cout << "Encounter Methods: ";
                for (auto& method : encounter_methods){
                    cout << method << " ";
                }

                cout << "\t" << "Min/Max encounter level: " << min_level << "/" << max_level << "\t"
                << "Min/Max encounter chance: " << min_chance << "/" << max_chance << "\t" << endl;

                return;
            }
        }
    }
    cout << "This pokemon does not appear in this generation" << endl;
}

void getPokedexInfo::getStats(const string& name){
    // get pokemon
    update.getPokemon(name);

    ifstream data("pokemon.json");

    if (!data.is_open()){
        throw runtime_error("Could not open pokemon.json");
    }

    json pokemonData = json::parse(data);

    data.close();

    json& pokemon = pokemonData[name];

    cout << "Stats for " << name << endl;
    for (auto& stat : pokemon["stats"]){
        cout << stat["stat"]["name"] << ": " << stat["base_stat"] << endl;
    }
}