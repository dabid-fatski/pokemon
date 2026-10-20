#pragma once

#include <string>

using namespace std;

class updatePokedex{
    private:
        const string APIurl;

    public:
        updatePokedex();
        ~updatePokedex();

        void getPokedex();
        void getGeneration();
        void getPokemon(const string& name);
        void getLocation(const string& name);
};