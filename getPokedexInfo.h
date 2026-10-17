#pragma once

#include <string>
#include "updatePokedex.h"

using namespace std;

class getPokedexInfo{
    private:
        updatePokedex& update;

    public:
        // construct an update pokdex object so we can make calls to it?
        getPokedexInfo(updatePokedex& update);

        void getMoves(const string& name, const string& generation);
        void getAbility(const string& name, const string& generation);
        void getLocations(const string& name, const string& generation);

};