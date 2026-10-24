#include <string>
#include <unordered_map>

#include "genMap.h"

using namespace std;

// Used for moves: version group names
const unordered_map<string, string> moveGenMap = {
    // Gen I
    {"Gen I", "yellow"},

    // Gen II
    {"Gen II", "crystal"},

    // Gen III
    {"Gen III", "emerald"},

    // Gen IV
    {"Gen IV", "heartgold-soulsilver"},

    // Gen V
    {"Gen V", "black-2-white-2"},

    // Gen VI
    {"Gen VI", "omega-ruby-alpha-sapphire"},

    // Gen VII
    {"Gen VII", "ultra-sun-ultra-moon"},

    // Gen VIII
    {"Gen VIII", "brilliant-diamond-shining-pearl"},

    // Gen IX
    {"Gen IX", "scarlet-violet"}
};


// Used for locations: individual game names
const unordered_map<string, string> locationGenMap = {
    // Generation I
    {"redblue", "red-blue"},
    {"red", "red"},
    {"blue", "blue"},
    {"yellow", "yellow"},

    // Generation II
    {"goldsilver", "gold-silver"},
    {"gold", "gold"},
    {"silver", "silver"},
    {"crystal", "crystal"},

    // Generation III
    {"rubysapphire", "ruby-sapphire"},
    {"ruby", "ruby"},
    {"sapphire", "sapphire"},
    {"emerald", "emerald"},
    {"fireredleafgreen", "firered-leafgreen"},
    {"firered", "firered"},
    {"leafgreen", "leafgreen"},

    // Generation IV
    {"diamondpearl", "diamond-pearl"},
    {"diamond", "diamond"},
    {"pearl", "pearl"},
    {"platinum", "platinum"},
    {"heartgoldsoulsilver", "heartgold-soulsilver"},
    {"heartgold", "heartgold"},
    {"soulsilver", "soulsilver"},

    // Generation V
    {"blackwhite", "black-white"},
    {"black", "black"},
    {"white", "white"},
    {"black2white2", "black-2-white-2"},
    {"black2", "black-2"},
    {"white2", "white-2"},

    // Generation VI
    {"xy", "x-y"},
    {"x", "x"},
    {"y", "y"},
    {"omegarubyalphasapphire", "omega-ruby-alpha-sapphire"},
    {"omegaruby", "omegaruby"},
    {"alphasapphire", "alphasapphire"},

    // Generation VII
    {"sunmoon", "sun-moon"},
    {"ultrasunultramoon", "ultra-sun-ultra-moon"},
    {"ultrasun", "ultra-sun"},
    {"ultramoon", "ultra-moon"},
    {"letsgopikachuletsgoeevee", "lets-go-pikachu-lets-go-eevee"},
    {"letsgopikachu", "lets-go-pikachu"},
    {"letsgoeevee", "lets-go-eevee"},

    // Generation VIII
    {"swordshield", "sword-shield"},
    {"sword", "sword"},
    {"shield", "shield"},
    {"brilliantdiamondshiningpearl", "brilliant-diamond-shining-pearl"},
    {"brilliantdiamond", "brilliant-diamond"},
    {"shiningpearl", "shining-pearl"},

    // Generation IX
    {"scarletviolet", "scarlet-violet"},
    {"scarlet", "scarlet"},
    {"violet", "violet"}
};