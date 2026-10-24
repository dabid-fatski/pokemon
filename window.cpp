#include <QLineEdit>
#include <QComboBox>
#include <QTextEdit>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QWidget>
#include <QMessageBox>
#include <QAbstractItemView>
#include <QElapsedTimer>
#include <QCoreApplication>

#include <iostream>
#include <string>
#include <exception>
#include <sstream>

#include "window.h"
#include "updatePokedex.h"
#include "getPokedexInfo.h"


Window::Window(QWidget *parent)
    : QMainWindow(parent), // initializes the Qt base class
    // constructs the updatePokedex object
    update(),
    // constructs the getPokedexInfo object
    pokedex(update)
{
    // creates the widgets and connects them together
    setupUI();
}

// create the basic window
void Window::setupUI(){
    pokemonInput = new QLineEdit(this);
    generationSelect = new QComboBox(this);
    gameSelect = new QComboBox(this);

    movesDisplay = new QTextEdit(this);
    abilityDisplay = new QTextEdit(this);
    statsDisplay = new QTextEdit(this);
    locationsDisplay = new QTextEdit(this);

    /*
    why this?
    because 1 they are pointers
    using pointers because they can create objects dynamically
    2 lets the widgit be a child of the class and lets Qt manage the lifetime
    once the parent widgit (in this case the whole app window) is destroyed, the children are also destroyed
    could have not made them pointers, but if they are direct members, you could have a member object be a C++ member object and a Qt child widgit at the same time
    this would mean that once qt destroys the child widgit, c++ would try to destroy the member, but since it was already destroyed, undefined behavior or a crash happens
    */
    QPushButton *searchButton = new QPushButton("Search Pokemon", this);

    // configure the widgets
    pokemonInput->setPlaceholderText("Enter a pokemon name");

    generationSelect->addItems({
        "Gen I",
        "Gen II",
        "Gen III",
        "Gen IV",
        "Gen V",
        "Gen VI",
        "Gen VII",
        "Gen VIII",
        "Gen IX"
    });

    updateGames();

    generationSelect->setMinimumWidth(75);
    gameSelect->setSizeAdjustPolicy(QComboBox::AdjustToContents);

    movesDisplay->setReadOnly(true);
    abilityDisplay->setReadOnly(true);
    statsDisplay->setReadOnly(true);
    locationsDisplay->setReadOnly(true);

    // create a central widget and layouts
    QWidget *window = new QWidget(this);

    // layouts
    QVBoxLayout *mainLayout = new QVBoxLayout(window);
    QHBoxLayout *displayLayout = new QHBoxLayout();

    // arrange the search widgets
    QHBoxLayout *searchLayout = new QHBoxLayout();
    
    searchLayout->addWidget(pokemonInput);
    searchLayout->addWidget(generationSelect);
    searchLayout->addWidget(gameSelect);
    searchLayout->addWidget(searchButton);

    // arrange the info display widgets
    /*
    no need to put "this" in QVBoxLayout
    main layout is assigned to window (QWidget) which tells QT that it should organize it's contents using mainLayout
    "this" refers to QMainWindow, so if it's passed to another layout, it would be trying to override it
    */
    QVBoxLayout *movesLayout = new QVBoxLayout();
    QVBoxLayout *abilityLayout = new QVBoxLayout();
    QVBoxLayout *statLayout = new QVBoxLayout();
    QVBoxLayout *locationLayout = new QVBoxLayout();

    movesLayout->addWidget(new QLabel("Moves", window));
    movesLayout->addWidget(movesDisplay);

    abilityLayout->addWidget(new QLabel("Abilities", window));
    abilityLayout->addWidget(abilityDisplay);

    statLayout->addWidget(new QLabel("Stats", window));
    statLayout->addWidget(statsDisplay);

    locationLayout->addWidget(new QLabel("Locations", window));
    locationLayout->addWidget(locationsDisplay);

    displayLayout->addLayout(movesLayout);
    displayLayout->addLayout(abilityLayout);
    displayLayout->addLayout(statLayout);
    displayLayout->addLayout(locationLayout);

    mainLayout->addLayout(searchLayout);
    mainLayout->addLayout(displayLayout);

    setCentralWidget(window);
    setWindowTitle("Pokedex");
    resize(900, 500);

    // connect gen select to game select
    connect(generationSelect, &QComboBox::currentTextChanged, this, &Window::updateGames);

    // connect the ui events to my functions
    connect(searchButton, &QPushButton::clicked, this, &Window::updateInfo);

    connect(pokemonInput, &QLineEdit::returnPressed, this, &Window::updateInfo);
}

void Window::updateInfo(){
    // get user input and read it
    const QString name = pokemonInput->text().trimmed();
    const QString generationName = generationSelect->currentText();
    const QString gameName = gameSelect->currentText();

    if (name.isEmpty()){
        QMessageBox::warning(this, "Missing Pokemon name", "Please enter a pokemon name");
        return;
    }

    QElapsedTimer timer;
    timer.start();

    // wrap entire thing in try because if something fails the program just crashes
    try {
        // get the corresponding api names
        const string pokemon = pokedex.normalize(name.toStdString());
        const string generation = generationName.toStdString();
        const string game = pokedex.normalize(gameName.toStdString());

        // update the pokemon if needed
        // becomes 60 ms because i have to open the file again
        update.getPokemon(pokemon);

        // get the information fron the cache
        string moves = pokedex.getMoves(pokemon, generation);
        string locations = pokedex.getLocations(pokemon, game);
        string abilities = pokedex.getAbility(pokemon);
        string stats = pokedex.getStats(pokemon);
        
        movesDisplay->setPlainText(QString::fromStdString(moves));
        abilityDisplay->setPlainText(QString::fromStdString(abilities));
        statsDisplay->setPlainText(QString::fromStdString(stats));
        locationsDisplay->setPlainText(QString::fromStdString(locations));

        // Let Qt process pending UI events, including painting
        QCoreApplication::processEvents();

        qDebug() << "Total search and UI time:"
                 << timer.nsecsElapsed() / 1'000'000.0 << "ms";
    } catch(const exception& e){
        QMessageBox::critical(this, "Search error", e.what());
    }
}

void Window::updateGames(){
    int gen = generationSelect->currentIndex();

    if (gen == previous){
        return;
    }

    gameSelect->clear();

    switch(gen){
        case 0:
            gameSelect->addItems({
                "Red",
                "Blue",
                "Yellow"
            });
            break;
        case 1:
            gameSelect->addItems({
                "Gold",
                "Silver",
                "Crystal"
            });
            break;
        case 2:
            gameSelect->addItems({
                "FireRed",
                "LeafGreen",
                "Ruby",
                "Sapphire",
                "Emerald"
            });
            break;
        case 3:
            gameSelect->addItems({
                "Diamond",
                "Pearl",
                "Platinum",
                "HeartGold",
                "SoulSilver"
            });
            break;
        case 4:
            gameSelect->addItems({
                "Black",
                "White",
                "Black 2",
                "White 2"
            }); 
            break;
        case 5:
            gameSelect->addItems({
                "X",
                "Y",
                "Omega Ruby",
                "Alpha Sapphire"
            });
            break;
        case 6:
            gameSelect->addItems({
                "Sun",
                "Moon",
                "Ultra Sun",
                "Ultra Moon",
                "Let's Go Pikachu",
                "Let's Go Eevee"
            });
            break;
        case 7:
            gameSelect->addItems({
                "Sword",
                "Shield",
                "Brilliant Diamond",
                "Shining Pearl"
            });
            break;
        case 8:
            gameSelect->addItems({
                "Scarlet",
                "Violet"
            });
            break;
    }
}