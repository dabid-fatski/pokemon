#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <cctype>
#include <stdexcept>
#include <fstream>
#include <sstream>
#include <curl/curl.h>
#include <nlohmann/json.hpp>

#include "updatePokedex.h"
#include "getPokedexInfo.h"
#include "genMap.h"

using namespace std;
using json = nlohmann::ordered_json;

// fuzzy matching (levenshtein distance)
int levenshtein(const string& original, const string& check){
    const int r = original.length();
    const int c = check.length();

    vector<int> prev(c+1);
    vector<int> cur(c+1);

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
            cur[j] = min({cur[j-1] + 1, prev[j] + 1, prev[j-1] + cost});
        }
        prev.swap(cur);
    }
    return prev[c];
}

// normalize the generation name
string getPokedexInfo::normalize(const string& input){
    string gen;

    for (const unsigned char c: input){
        if (isspace(c) || c == '-' || c == '/'){
            continue;
        }
        gen += tolower(c);
    }
    return gen;
}

// find the matching key
string findGen(const string& input, const unordered_map<string, string>& genMap){
    const string genName = input;

    // get api name
    const auto found = genMap.find(genName);

    if (found == genMap.end()){
        throw runtime_error("Unknown generation or game: " + input);
    }

    return found->second;
}

// make a fuzzy for pokemon names

struct moveInfo{
    string name;
    int level;
    string method;
};

// constructor
getPokedexInfo::getPokedexInfo(updatePokedex& update)
    : update(update)
{
    ifstream data("pokemon.json");

    if (!data.is_open()){
        throw runtime_error("Could not open pokemon.json");
    }

    data >> pokemonData;
}

string getPokedexInfo::getMoves(const string& name, const string& generation){
    // get the api game name
    const string gameName = findGen(generation, moveGenMap);
    
    const json& pokemon = pokemonData.at(name);
    
    const json& pkmnMoves = pokemon["moves"];

    if (pkmnMoves.find(gameName) == pkmnMoves.end()){
        return "This pokemon does not appear in this game";
    }
    
    // ostringstream saves it to memory instead of cout which jus toutputs it instantly
    // creates an empty stream
    // everything output << is saved and then returned as if a document
    ostringstream output;

    const json& moves = pokemon["moves"][gameName];

    for (const auto& move : moves){
        string method = move["method"];
        output << move["name"] << '\t';
        
        if (method == "level-up"){
            output << "lvl: " << move["level"] << endl;
        } else {
            output << method << endl;
        }
    }

    return output.str();
}

string getPokedexInfo::getLocations(const string& name, const string& generation){
    // get the api game name
    const string gameName = findGen(generation, locationGenMap);

    const json& pokemon = pokemonData.at(name);

    // check if the location exists
    const json& encounters = pokemon["location_area_encounters"];

    if (encounters.find(gameName) == encounters.end()){
        return "This pokemon doesn't appear in this game.";
    }

    ostringstream output;

    const json& game = pokemon["location_area_encounters"][gameName];

    for (const auto& [locationName, location] : game.items()){
        output << locationName << " " <<
        "Method: " << location["method"] << ' ' <<
        "Min chance: " << location["min_chance"] << ' ' <<
        "Max chance: " << location["max_chance"] << ' ' <<
        "Min level: " << location["min_level"] << ' ' <<
        "Max level: " << location["max_level"]; 
        
        const json& conditions = location["conditions"];

        if (!conditions.empty()){
            for (const auto& condition: location["conditions"]){
                output << condition << ' ';
            } 
        }
        
        output << endl;
    }
   
    return output.str();
}

string getPokedexInfo::getAbility(const string& name){

    const json& pokemon = pokemonData.at(name);
        
    ostringstream output;

    for (const auto& ability : pokemon["abilities"]){
        output << ability["ability"]["name"].get<string>();

        if (ability["is_hidden"]){
            output << " Hidden ability (Gen V+)";
        }
        
        output << endl;
    }
    return output.str();
}

string getPokedexInfo::getStats(const string& name){

    const json& pokemon = pokemonData.at(name);

    ostringstream output;

    for (const auto& stat : pokemon["stats"]){
        output << stat["stat"]["name"] << ": " << stat["base_stat"] << endl;
    }

    return output.str();
}