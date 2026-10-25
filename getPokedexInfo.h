#pragma once

#include <string>

#include "updatePokedex.h"

using namespace std;

class getPokedexInfo{
    private:
        updatePokedex& update;

    public:
        // pass the updatePokedex object by reference
        // this lets us use updatePokedex member functions
        getPokedexInfo(updatePokedex& update);

        static string normalize(const string& input);

        string getMoves(const string& name, const string& generation) const;
        string getLocations(const string& name, const string& generation) const;
        string getAbility(const string& name) const;
        string getStats(const string& name) const;
};