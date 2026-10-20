#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <cctype>
#include <stdexcept>
#include <fstream>
#include <curl/curl.h>
#include <nlohmann/json.hpp>

#include "updatePokedex.h"
#include "getPokedexInfo.h"
#include "genMap.h"

using namespace std;
using json = nlohmann::ordered_json;

// normalize the generation name
string normalize(const string& input){
    string gen;

    for (const unsigned char c: input){
        if (isspace(c) || c == '-'){
            continue;
        }
        gen += tolower(c);
    }
    return gen;
}

// fuzzy matching (levenshtein distance)
int levenshtein(const string& original, const string& check){
    const int r = original.length();
    const int c = check.length();

    vector<int> prev(min(r+1, c+1));
    vector<int> cur(min(r+1, c+1));

    for (int i = 0; i < c+1; i++){
        prev[i] = i;
    }

    // since only use the previous row, can optimize from a whole matrix to 2 vectors
    for (int i = 1; i < r+1; i++){
        cur[0] = i;
        for (int j = 1; j < c+1; j++){
            int cost = (original[i-1] == check[j-1]) ? 0 : 1;

            // cur[j-1] = add
            // prev[i] = delete
            // prev[i-1] = replace (cost depends if letters match)
            cur[j] = min({cur[j-1] + 1, prev[i] + 1, prev[i-1] + cost});
        }
        prev = cur;
    }
    return prev[c];
}

// find the matching key
string findGen(const string& input, const unordered_map<string, string>& genMap){
    const string genName = normalize(input);

    // if it exists, just return it instantly
    const auto found = genMap.find(genName);

    if (found != genMap.end()){
        return found->second;
    }
    
    // if it doesnt exist in the default map, levenshtein distnace it
    int diff = INT_MAX;
    string guess;
    string apiName;

    for (const auto& [name, original] : genMap){
        const int distance = levenshtein(name, genName);

        if (distance < diff){
            diff = distance;
            guess = name;
            apiName = original;
        }
    }

    // throw ones that are too wrong
    if (diff <= 3){
        char input;
        cout << "Did you mean " << guess << "? (y/n)" << endl;
        cin >> input;

        if (tolower(input) == 'y'){
            return apiName;
        }

        throw invalid_argument("Unknown generation/game");
    }

    throw invalid_argument("Unknown generation/game");
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
    
    const string genName = findGen(generation, moveGenMap);
    
    // load json
    ifstream data("pokemon.json");

    if (!data.is_open()){
        throw runtime_error("Could not open pokemon.json");
    }

    const json pokemonData = json::parse(data);

    data.close();

    const json& pokemon = pokemonData[name];

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
        const auto priority = [](const string& method){
            if (method == "level-up") return 0;
            if (method == "machine") return 1;
            if (method == "tutor") return 2;
            return 3;
        };
        const int aPrio = priority(a.method);
        const int bPrio = priority(b.method);

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

void getPokedexInfo::getLocations(const string& name, const string& generation){
    // update pokedex
    update.getPokemon(name);
    update.getLocation(name);

    const string genName = findGen(generation, locationGenMap);

    ifstream data("pokemon.json");

    if (!data.is_open()){
        throw runtime_error("Could not open pokemon.json");
    }

    const json pokemonData = json::parse(data);

    data.close();

    const json& pokemon = pokemonData[name];

    for (const auto& location : pokemon["location_area_encounters"]){
        const json& version_details = location["version_details"];
        for (const auto& version : version_details){
            const json& versionName = version["version"]["name"];
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

void getPokedexInfo::getAbility(const string& name){
    // update pokedex
    update.getPokemon(name);

    ifstream data("pokemon.json");

    if (!data.is_open()){
        throw runtime_error("Could not open Pokemon.json");
    }

    const json pokemonData = json::parse(data);

    data.close();

    const json& pokemon = pokemonData[name];
        
    for (const auto& ability : pokemon["abilities"]){
        if (ability["is_hidden"] == false){
            cout << ability["ability"]["name"] << endl;
        } else {
            cout << ability["ability"]["name"] << "\t" << " Hidden ability (Gen V+)" << endl;
        }
    }
    return;
}

void getPokedexInfo::getStats(const string& name){
    // get pokemon
    update.getPokemon(name);

    ifstream data("pokemon.json");

    if (!data.is_open()){
        throw runtime_error("Could not open pokemon.json");
    }

    const json pokemonData = json::parse(data);

    data.close();

    const json& pokemon = pokemonData[name];

    cout << "Stats for " << name << endl;
    for (const auto& stat : pokemon["stats"]){
        cout << stat["stat"]["name"] << ": " << stat["base_stat"] << endl;
    }
}