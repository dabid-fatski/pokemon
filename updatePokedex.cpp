#include <iostream>
#include <string>
#include <unordered_map>
#include <cctype>
#include <stdexcept>
#include <fstream>
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

updatePokedex::updatePokedex(){
    CURLcode result = curl_global_init(CURL_GLOBAL_DEFAULT);

    if (result != CURLE_OK) {
        throw runtime_error("Failed to initialize CURL");
    }

    APIurl = "https://pokeapi.co/api/v2/pokemon?limit=100000&offset=0";
}

updatePokedex::~updatePokedex(){
    curl_global_cleanup();
}

void updatePokedex::getPokedex(){   
    json pokemonData = fetchData(APIurl);

    ofstream file("pokemon.json");

    if (!file.is_open()){
        throw runtime_error("Could not open pokemon.json");
    }

    file << pokemonData.dump(4);
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

    // find the pokemon
    for (auto& pokemon : pokemonData["results"]){
        cout << pokemon["name"] << endl;
        if (pokemon["name"] == name){
            // if we already have the data
            if (pokemon.contains("id")){
                cout << "already have the pokemon" << endl;
                return;
            }

            cout << "don't have the pokemon" << endl;
            // otherwise request it
            string url = pokemon["url"];
            json pokemonDetails = fetchData(url);

            // update the json
            pokemon = pokemonDetails;

            ofstream file("pokemon.json");

            if (!file.is_open()){
                throw runtime_error("Couldn't open pokemon.json");
            }

            file << pokemonData.dump(4);
            file.close();

            cout << "Did an API call for " << name << endl;
            return;
        }
    }

    cout << "That is not a known pokemon" << endl;
}