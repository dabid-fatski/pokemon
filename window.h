#ifndef WINDOW_H
#define WINDOW_H

#include <QMainWindow>

#include "updatePokedex.h"
#include "getPokedexInfo.h"

class QLineEdit;
class QComboBox;
class QTextEdit;

class Window :
    public QMainWindow{
    Q_OBJECT

    public:
        explicit Window(QWidget * parent = nullptr);

    private slots:

        void updateInfo();
        
    private:
        QLineEdit *pokemonInput;
        QComboBox *generationSelect;
        QComboBox *gameSelect;

        QTextEdit *movesDisplay;
        QTextEdit *abilityDisplay;
        QTextEdit *statsDisplay;
        QTextEdit *locationsDisplay;

        updatePokedex update;
        getPokedexInfo pokedex;

        void setupUI();
        void updateGames();
        int previous;
};

#endif