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

        void getMoves(const string& name, const string& generation);
        void getAbility(const string& name);
        void getLocations(const string& name, const string& generation);

};