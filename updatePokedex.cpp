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

struct Encounter{
    string method;
    int min_chance;
    int max_chance;
    int min_level;
    int max_level;
    vector<string> conditions;
};

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

void updatePokedex::getPokemon(const string& name){
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

        // update the json
        pokemon["location_area_encounters"] = getLocation(url);

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

    /*
    encounter{
        method
        min_chance
        max_chance
        min_level
        max_level
        conditions[]
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
    json locationJson = json::object();

    for (const auto& [game, locations] : locationMap){
        locationJson[game] = json::object();

        for (const auto& [location, encounter] : locations){
            locationJson[game][location] = {{"method", encounter.method},
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