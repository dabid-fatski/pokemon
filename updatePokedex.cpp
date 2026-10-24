#include <iostream>
#include <string>
#include <unordered_map>
#include <cctype>
#include <stdexcept>
#include <fstream>
#include <algorithm>

#include <curl/curl.h>
#include <nlohmann/json.hpp>

#include "updatePokedex.h"

using namespace std;
using json = nlohmann::ordered_json;

size_t writeCallback(char* data, size_t size, size_t count, string* output){
	output->append(data, size * count);
	return size * count;
}

json fetchData(string url){
    CURL* curl = curl_easy_init();

    if (!curl){
        throw runtime_error("Failed to initialize CURL");
    }

    string response;

    // where to send the request
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    // how to handle the response
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    // where to put the response
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    // follow http redirects
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    CURLcode result = curl_easy_perform(curl);

    long httpCode = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);

    curl_easy_cleanup(curl);

    if (result != CURLE_OK){
        throw runtime_error(string("CURL error: ") + curl_easy_strerror(result));
    }

    if (httpCode < 200 || httpCode >= 300){
        throw runtime_error("HTTP request failed with status " + to_string(httpCode));
    }
    
    return json::parse(response);
}

updatePokedex::updatePokedex()
    : APIurl {"https://pokeapi.co/api/v2/pokemon?limit=100000&offset=0"}
{
    CURLcode result = curl_global_init(CURL_GLOBAL_DEFAULT);

    if (result != CURLE_OK) {
        throw runtime_error("Failed to initialize CURL");
    }

    checkPokedex();
}

updatePokedex::~updatePokedex(){
    curl_global_cleanup();
}

void updatePokedex::getPokedex(){

    json pokemonData = fetchData(APIurl);

    json pokedex = json::object();

    // make checking pokemon O(1)
    for (const auto& pokemon : pokemonData["results"]){
        pokedex[pokemon["name"]] = pokemon;
    }

    ofstream file("pokemon.json");

    if (!file.is_open()){
        throw runtime_error("Could not open pokemon.json");
    }

    file << pokedex.dump(4);
    file.close();

    cout << "Pokedex has been updated" << endl;
}

void updatePokedex::checkPokedex(){
    bool check = false;

    ifstream data("pokemon.json");

    if (!data.is_open()){
        check = true;
    } else {
        try {
            json pokemonData = json::parse(data);

            if (!pokemonData.is_object() || pokemonData.empty()){
                check = true;
            }
        } catch (const json::parse_error& e){
            cerr << "Invalid pokemon.json: " << e.what() << endl;
            check = true;
        }
    }
    if (check){
        getPokedex();
    }
}

void updatePokedex::getPokemon(const string& name){
    // see if there is a valid file
    checkPokedex();
    
    // load json
    ifstream data("pokemon.json");

    if (!data.is_open()){
        throw runtime_error("Couldn't open pokemon.json");
    }
    json pokemonData = json::parse(data);
    data.close();

    json& pokemon = pokemonData[name];

    if (pokemon.contains("id")){
        cout << "Already have the pokemon" << endl;
        return;
    } else {
        cout << "Don't have the pokemon" << endl;
        // otherwise request it
        string url = pokemon["url"];
        json pokemonDetails = fetchData(url);

        // update the json
        pokemon = pokemonDetails;

        // get location
        url = pokemon["location_area_encounters"];

        // parse the locations
        pokemon["location_area_encounters"] = getLocation(url);

        // parse the moves
        pokemon["moves"] = getMoves(pokemon);

        ofstream file("pokemon.json");

        if (!file.is_open()){
            throw runtime_error("Couldn't open pokemon.json");
        }

        file << pokemonData.dump(4);
        file.close();

        cout << "Did an API call for " << name << endl;
        return;
    }

    cout << "That is not a known pokemon" << endl;
}

json updatePokedex::getLocation(const string& url){
    json locationDetails = fetchData(url);

    struct Encounter{
        string method;
        int min_chance;
        int max_chance;
        int min_level;
        int max_level;
        vector<string> conditions;
    };

    /*
    "x" {
        location {
            encounter
        },
        location {
            encounter
        }
    }
    */

    // {"x" : {"location" : encounter, "location" : encounter,}}
    unordered_map<string, unordered_map<string, Encounter>> locationMap;

    for (const auto& location : locationDetails){
        string locationName = location["location_area"]["name"];

        for (const auto& versionDetails : location["version_details"]){
            string gameName = versionDetails["version"]["name"];
            
            for (const auto& encounterDetails : versionDetails["encounter_details"]){
                string method = encounterDetails["method"]["name"];

                int chance = encounterDetails["chance"];
                int min_level = encounterDetails["min_level"];
                int max_level = encounterDetails["max_level"];

                auto& locations = locationMap[gameName];
                auto check = locations.find(locationName);

                if (check == locations.end()){
                    // if we don't have the location add it
                    // get the conditions
                    vector<string> conditions;
                    if (encounterDetails["condition_values"].size() != 0){
                        for (const auto& condition : encounterDetails["condition_values"]){
                            conditions.push_back(condition["name"]);
                        }
                    }

                    locations[locationName] = {
                        method,
                        chance,
                        chance,
                        min_level,
                        max_level,
                        conditions
                    };
                } else {
                    // update if we have a location already
                    Encounter& e = check->second;

                    e.min_chance = min(e.min_chance, chance);
                    e.max_chance = max(e.max_chance, chance);
                    e.min_level = min(e.min_level, min_level);
                    e.max_level = max(e.max_level, max_level);
                }
            }
        }
    }

    // convert the map to a json
    // probably should sort by location name if im going to make a data structure and then convert it
    json locationJson = json::object();

    for (const auto& [game, locations] : locationMap){
        locationJson[game] = json::object();

        // sort the locations alphabetically
        vector<string> sortLocations;
        for (const auto& [location, encounter] : locations){
            sortLocations.push_back(location);
        }
        sort(sortLocations.begin(), sortLocations.end());

        // for every location in the sorted locations, set the json
        for (const auto& location : sortLocations){
            const Encounter& encounter = locations.at(location);
            locationJson[game][location] = {
                {"method", encounter.method},
                {"min_chance", encounter.min_chance},
                {"max_chance", encounter.max_chance},
                {"min_level", encounter.min_level},
                {"max_level", encounter.max_level},
                {"conditions", encounter.conditions}
            };
        }
    }

    return locationJson;
}

// parse moves
json updatePokedex::getMoves(json& pokemonData){

    struct Move{
        string moveName;
        string method;
        int level;
    };

    /*
    game name {
        move names [
            {
                move
            },
            {
                move
            }
        ]
    }
    */
    
    unordered_map<string, unordered_map<string, vector<Move>>> moveMap;

    for (const auto& move : pokemonData["moves"]){
        string moveName = move["move"]["name"];

        for (const auto& detail : move["version_group_details"]){
            int level = detail["level_learned_at"];
            string game = detail["version_group"]["name"];
            string method = detail["move_learn_method"]["name"];

            if (method == "level-up" && level == 0){
                continue;
            }

            moveMap[game][moveName].push_back({
                moveName, method, level
            });
        }
    }

    // sort the moves so dont have to sort when calling it
    // lambda comparator
    const auto priority = [](const string& method){
        if (method == "level-up") return 0;
        if (method == "machine") return 1;
        if (method == "tutor") return 2;
        if (method == "egg") return 3;
        return 4;
    };

    // sort the moves, then save it to the json 
    json moveJson = json::object();

    // for every game in moveMap
    for (const auto& [gameName, moveNames] : moveMap){
        // get every move
        vector<Move> sortedMoves;
        // for every move in moves
        for (const auto& [moveName,  moveInfo] : moveNames){
            // for every move method in each move
            for (const auto& info : moveInfo){
                sortedMoves.push_back(info);
            }
        }
        // [&priorty] references the lambda made earlier
        sort(sortedMoves.begin(), sortedMoves.end(), [&priority](const Move& a, const Move& b){
            const int aPrio = priority(a.method);
            const int bPrio = priority(b.method);

            // sort by the method first
            if (aPrio != bPrio){
                return aPrio < bPrio;
            }

            // sort by the level up requirements
            if (a.method == "level-up"){
                return a.level < b.level;
            }

            // sort everything else by name
            return a.moveName < b.moveName;
        });

        // convert it to a json array of moves all sorted
        moveJson[gameName] = json::array();
        for (const auto& move : sortedMoves){
            moveJson[gameName].push_back({
                {"name", move.moveName},
                {"method", move.method},
                {"level", move.level}
            });
        }
    }

    return moveJson;
}