#pragma once

#include <string>

using namespace std;

class updatePokedex{
    private:
        string APIurl;

    public:
        updatePokedex();
        ~updatePokedex();

        void getPokedex();
        void getGeneration();
        void getPokemon(const string& name);
};