#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <array>

using namespace sf;
using namespace std;

/***************************************************
                 VARIABILE GLOBALE:
****************************************************/

int MATRIX[9][9]; // Matricea jocului care se actualizeaza dupa fiecare mutare (0=pozitie libera, 1=PLAYER, 2=COMPUTER)
int OCCUPIED_COL[9]; // se actualizeaza deodata cu matricea si OCCUPIED_COL[i] returneaza cate jetoane sunt pe coloana i
bool ENDGAME = false;
int GAMEMODE = 0; // 0=neselectat (ecranul de inceput); 1=easy (usor); 2=medium (mediu); 3=hard (greu)
int TURNS = 0; // creste
const int NUM_COL = 7; // numarul de coloane din matrice
const int NUM_ROW = 6; // numarul de randuri din matrice
const int PLAYER = 1; // numarul playerului care e folosit atat in MATRIX[][] cat si in algoritmul AI-ului
const int COMPUTER = 2; // numarul AI-ului folosit atat in MATRIX[][] cat si in algoritmul AI-ului
const int WIDTH = 1190, HEIGHT = 1020; //170/2 per box (the board is 7 by 6)

RenderWindow window(VideoMode(WIDTH, HEIGHT), "Connect4 by Efraim", Style::Close | Style::Titlebar); //cream window-ul (fereastra)

/***************************************************
               ANTETELE FUNCTIILOR:
****************************************************/

//functii folosite la joc si imagine:
void spawnToken(int , int& , int& );
bool fourHorizontal_withHighlight(int , int );
bool fourVertical_withHighlight(int , int );
bool fourDiagonal_withHighlight(int , int );
bool isBoardFull(int );
void updateMATRIX(int , int);
void updateVisuals(RectangleShape );
void ENDGAMENewScreen(RectangleShape );
bool mouseInRangeOfButton(int , Vector2i );
void highlightStartScreen(Vector2i );
//void highlightColWithMouse(Vector2i )

//functii folosite la A.I. (Artificial Inteligence):
int** convertBoard(int);
array<int, 2> miniMax(int** , int , int );
int tabScore(int** , int );
int scoreSet(int, int);
int heuristicValue(int , int , int );
int** copyBoard(int** );
bool winningMove(int**& , int );
void makeMove(int** , int , int );

/***************************************************
           FUNCTII PENTRU JOC SI IMAGINE:
****************************************************/


void spawnToken(int currentPlayer, int& I, int& J) 
{  
    //functia este apelata la fiecare element din M si creeaza si afiseaza pe ecran un token (jeton).
    const int RADIUS = 67;
    array<int, 2> tokenPos = { 78 + (J - 1) * 171,  84 + (I - 1) * 170.5 };

    CircleShape token;
    token.setRadius(RADIUS);
    token.setPosition(tokenPos[0], tokenPos[1]);
    Texture tokenTexture;
    switch (currentPlayer)
    {
    case PLAYER:
        tokenTexture.loadFromFile("resources/tokenYellow.png");
        break;
    case COMPUTER:
        tokenTexture.loadFromFile("resources/tokenRed.png");
        break;
    case PLAYER + 7: // daca playerul a castigat
        tokenTexture.loadFromFile("resources/tokenYellowWin.png");
        break;
    case COMPUTER + 7: // daca computerul a castigat
        tokenTexture.loadFromFile("resources/tokenRedWin.png");
        break;
    }
    token.setTexture(&tokenTexture);
    token.setOrigin(RADIUS, RADIUS);

    window.draw(token);
}


bool fourHorizontal_withHighlight(int y, int x)
{
    //Cautam pe ORIZONTALA daca ultimul token pus a castigat (cel putin 4 intr-o linie ORIZONTALA)
    int tokenColor = MATRIX[y][x];
    int cnt = 1, k;
    for (k = x + 1; MATRIX[y][k] == tokenColor; k++)
        cnt++;
    for (k = x - 1; MATRIX[y][k] == tokenColor; k--)
        cnt++;
    if (cnt >= 4)
    {
        while (MATRIX[y][++k] == tokenColor)
            MATRIX[y][k] = tokenColor + 7;
        return true;
    }
    else 
        return false;
}

bool fourVertical_withHighlight(int y, int x)
{
    //Cautam pe VERTICALA daca ultimul token pus a castigat (cel putin 4 intr-o linie VERTICALA)
    int tokenColor = MATRIX[y][x];
    int cnt = 1, k;
    for (k = y + 1; MATRIX[k][x] == tokenColor; k++)
        cnt++;
    for (k = y - 1; MATRIX[k][x] == tokenColor; k--)
        cnt++;
    if (cnt >= 4)
    {
        while (MATRIX[++k][x] == tokenColor)
            MATRIX[k][x] = tokenColor + 7;
        return true;
    }
    else
        return false;
}
bool fourDiagonal_withHighlight(int x, int y)
{
    //Cautam pe DIAGONALA daca ultimul token pus a castigat (cel putin 4 intr-o linie DIAGONALA)
    bool t = false;
    int tokenColor = MATRIX[x][y];
    int cnt = 1, m, n;
    //Prima cautare: "/"
    for (m = x + 1, n = y - 1; MATRIX[m][n] == tokenColor; m++, n--)
        cnt++;
    for (m = x - 1, n = y + 1; MATRIX[m][n] == tokenColor; m--, n++)
        cnt++;
    if (cnt >= 4)
    {
        while (MATRIX[++m][--n] == tokenColor)
            MATRIX[m][n] = tokenColor + 7;
        t = true;
    }
    //A doua cautare: "\"
    cnt = 1;
    for (m = x + 1, n = y + 1; MATRIX[m][n] == tokenColor; m++, n++)
        cnt++;
    for (m = x - 1, n = y - 1; MATRIX[m][n] == tokenColor; m--, n--)
        cnt++;
    if (cnt >= 4)
    {
        while (MATRIX[++m][++n] == tokenColor)
            MATRIX[m][n] = tokenColor + 7;
        return true;
    }
    else
        return t;
}

bool isBoardFull(int M[9][9])
{
    //Pentru a verifica daca e remiza
    int fullColumns = 0;
    for (int c = 1; c <= 7;c++)
    {
        if (M[1][c] != 0)
            fullColumns++;
    }
    if (fullColumns == 7)
        return true;
    return false;
}

void updateMATRIX(int currentPlayer, int chosenCol)
{
    // actualizeaza MATRIX[][] si OCCUPIED_COL[] si verifica daca jocul se termina dupa aceasta mutare
    array<int, 2> pos = { 7 - OCCUPIED_COL[chosenCol] - 1, chosenCol };
  
    MATRIX[pos[0]][pos[1]] = currentPlayer;
    OCCUPIED_COL[chosenCol]++;

    if (fourHorizontal_withHighlight(pos[0], pos[1]) || fourVertical_withHighlight(pos[0], pos[1]) || fourDiagonal_withHighlight(pos[0], pos[1]) || isBoardFull(MATRIX))
          ENDGAME = true;
}

void updateVisuals(RectangleShape boardClone)
{
    //actualizam ceea ce se vede (fereastra de joc) si daca ENDGAME din updateMATRIX, atunci afisam ecranul de sfarsit
    window.clear(Color::White);

    for (int r = 1; r <= NUM_ROW; r++)
        for (int c = 1; c <= NUM_COL; c++)
            if (MATRIX[r][c] != 0)
                spawnToken(MATRIX[r][c], r, c);

    window.draw(boardClone);

    if (ENDGAME)
    {
        RectangleShape gameOverScreen(Vector2f((float)WIDTH, (float)HEIGHT));
        Texture gameOverTexture;
        gameOverTexture.loadFromFile("resources/gameOverScreen.png");
        gameOverScreen.setTexture(&gameOverTexture);

        window.draw(gameOverScreen);
    }

    window.display();
}

void ENDGAMENewScreen(RectangleShape newScreen)
{
    //se termina jocul si incepe alta runda
    sleep(seconds(4));
    ENDGAME = false;
    for (int r = 0;r <= NUM_ROW;r++)
        for (int c = 0;c <= NUM_COL;c++)
            MATRIX[r][c] = 0;
    for (int i = 0;i <= 8;i++)
        OCCUPIED_COL[i] = 0;
    GAMEMODE = 0;
    TURNS = 0;
    window.clear(Color::White);
    window.draw(newScreen);
    window.display();
}

bool mouseInRangeOfButton(int button, Vector2i mouse)
{
    // pentru a verifica daca cursorul este in aria unui buton
    // este folosita in highlightStartScreen
    if (button == 1 && (mouse.x > 455 && mouse.x < 741 && mouse.y > 410 && mouse.y < 484))
        return true;
    if (button == 2 && (mouse.x > 445 && mouse.x < 741 && mouse.y > 525 && mouse.y < 640))
        return true;
    if (button == 3 && (mouse.x > 455 && mouse.x < 741 && mouse.y > 694 && mouse.y < 773))
        return true;
    if (button == 9 && (mouse.x > 930 && mouse.x < 1105 && mouse.y > 922 && mouse.y < 970))
        return true;
    return false;
}

void highlightStartScreen(Vector2i mouseCoord)
{
    // daca mousul este in aria unui buton semnalam asta printr-un efect vizual (buton gri)
    RectangleShape startHighlight(Vector2f(1190.0f, 1020.0f));
    Texture highlightTexture;

    if (mouseInRangeOfButton(1, mouseCoord))
        highlightTexture.loadFromFile("resources/startHEasy.png");
    else if (mouseInRangeOfButton(2, mouseCoord))
        highlightTexture.loadFromFile("resources/startHMedium.png");
    else if (mouseInRangeOfButton(3, mouseCoord))
        highlightTexture.loadFromFile("resources/startHHard.png");
    else if (mouseInRangeOfButton(9, mouseCoord))
        highlightTexture.loadFromFile("resources/startHQuit.png");
    else
        highlightTexture.loadFromFile("resources/startScreen.png");

    startHighlight.setTexture(&highlightTexture);

    window.clear(Color::White);
    window.draw(startHighlight);
    window.display();
}

/***************************************************
             FUNCTII PENTRU A.I.:
****************************************************/

int** convertBoard(int b[9][9])
{
    //functie folosita pentru a converti matricea MATRIX[][] intr o matrice de tipul int** pentru a putea fi folosita in codul pentru A.I.
    int** newBoard = 0;
    newBoard = new int* [20];
    for (int r = 0; r < 20; r++)
    {
        newBoard[r] = new int[20];
        for (int c = 0; c < 20; c++)
            newBoard[r][c] = 0;
    }
    for (int r = 0; r < NUM_ROW; r++)
        for (int c = 0; c < NUM_COL; c++)
            newBoard[r][c] = b[NUM_ROW - r][c + 1];

    return newBoard;
}

int** copyBoard(int** B)
{
    //copiaza o matrice intr-o alta matrice pentru a nu modifica matricea de care avem nevoie
    int** newBoard = 0;
    newBoard = new int* [20];//
    for (int r = 0; r < NUM_ROW; r++)
    {
        newBoard[r] = new int[20];
        for (int c = 0; c < NUM_COL; c++)
        {
            newBoard[r][c] = B[r][c]; // just straight copy
        }
    }
    return newBoard;
}

void makeMove(int** b, int col, int currentPlayer) 
{
    //   Pune piesa jucatorului curent in matricea b pe coloana col
    //   Nu am putut folosi updateMATRIX() deoarece se refera doar la MATRIX, iar aceasta functie este folosita in
   // algoritmul AI-ului pentru a "simula" mutarile pe o alta matrice
    for (int r = 0; r < NUM_ROW; r++) 
        if (b[r][col] == 0)// primul loc liber
        { 
            b[r][col] = currentPlayer;
            break;
        }
}

bool winningMove(int**& b, int currentPlayer) 
{
    // Determina daca mutarea e castigatoare.
    // Din nou, nu am putut folosi ce aveam deoarece aici avem alta situatie (alti parametri compatibili cu AI-ul)
    int sequence = 0; // pentru a numara secventele
  
    for (int c = 0; c < NUM_COL - 3; c++)
    {
        for (int r = 0; r < NUM_ROW; r++) 
        { 
            for (int i = 0; i < 4; i++) { // recall you need 4 to win
                if (b[r][c + i] == currentPlayer)
                    sequence++; // add sequence count
                if (sequence == 4) 
                    return true;  // if 4 in row
            }
            sequence = 0; // reset counter
        }
    }
    // verificam pe verticala:
    for (int c = 0; c < NUM_COL; c++)
    {
        for (int r = 0; r < NUM_ROW - 3; r++) 
        {
            for (int i = 0; i < 4; i++) 
            {
                if (b[r + i][c] == currentPlayer) 
                    sequence++;
                if (sequence == 4) 
                    return true; 
            }
            sequence = 0;
        }
    }
    // verificam pe diagonala: "/"
    for (int c = 0; c < NUM_COL - 3; c++)
    {
        for (int r = 3; r < NUM_ROW; r++)
        {
            for (int i = 0; i < 4; i++) 
            {
                if (b[r - i][c + i] == currentPlayer) 
                    sequence++;
                if (sequence == 4) 
                    return true; 
            }
            sequence = 0;
        }
    }
    // "\"
    for (int c = 0; c < NUM_COL - 3; c++)
    {
        for (int r = 0; r < NUM_ROW - 3; r++)
        {
            for (int i = 0; i < 4; i++) 
            {
                if (b[r + i][c + i] == currentPlayer) 
                    sequence++;
                if (sequence == 4)  
                    return true; 
            }
            sequence = 0;
        }
    }
    return false; // nu avem mutare castigatoare
}

int heuristicValue(int g, int b, int e) 
{
    //functie care ofera scorul unei secvente de 4 (cu cat e mai buna secventa, cu atat e mai mare scorul)
    int score = 0;
    if (g == 4) 
        score += 500001; // castigam mai dehraba decat sa blocam
    else if (g == 3 && e == 1)
        score += 5000; 
    else if (g == 2 && e== 2) 
        score += 500; 
    else if (b == 2 && e == 2) 
        score -= 501;
    else if (b == 3 && e == 1)  
        score -= 5001;
    else if (b == 4) 
        score -= 500000; 
    return score;
}

int scoreSet(int v[4], int currentPlayer) 
{
    // gaseste scorul unui set de 4 locuri. 
    // v este coloana/randul de verificat
    int good = 0; 
    int bad = 0; 
    int empty = 0; 
    for (int i = 0; i < 4; i++)
    {
        if (v[i] == currentPlayer)
            good++;
        if (v[i] == PLAYER || v[i] == COMPUTER)
            bad++;
        if (v[i] == 0)
            empty++;
    }
    // bad a fost calculat ca si (bad+good), asa ca scadem good
    bad -= good;
    return heuristicValue(good, bad, empty);
}

int tabScore(int ** b, int p) 
{
    int score = 0;
    int rs[NUM_COL+1];
    int cs[NUM_ROW+1];
    int set[5];
    // verificam secvente de 4 pozitii care contin orice combinatie de PLAYER(1), COMPUTER(2), spatiu gol(0)
    // pentru a putea gasi scorul intregii table de joc cu ajutorul functiilor de mai sus
    for (int r = 0; r < NUM_ROW; r++) 
    {
        for (int c = 0; c < NUM_COL; c++)
            rs[c] = b[r][c];
        for (int c = 0; c < NUM_COL - 3; c++) 
        {
            for (int i = 0; i < 4; i++) 
                set[i] = rs[c + i]; // pentru fiecare set de 4 posibile in acel rand
            score += scoreSet(set, p); // gaseste scorul
        }
    }
    // verticala
    for (int c = 0; c < NUM_COL; c++) 
    {
        for (int r = 0; r < NUM_ROW; r++)
            cs[r] = b[r][c];
        for (int r = 0; r < NUM_ROW - 3; r++)
        {
            for (int i = 0; i < 4; i++)
                set[i] = cs[r + i];
            score += scoreSet(set, p);
        }
    }
    // diagonale
    for (int r = 0; r < NUM_ROW - 3; r++)
    {
        for (int c = 0; c < NUM_COL; c++)
            rs[c] = b[r][c];
        for (int c = 0; c < NUM_COL - 3; c++) 
        {
            for (int i = 0; i < 4; i++) 
                set[i] = b[r + i][c + i];
            score += scoreSet(set, p);
        }
    }
    for (int r = 0; r < NUM_ROW - 3; r++) 
    {
        for (int c = 0; c < NUM_COL; c++) 
            rs[c] = b[r][c];
        for (int c = 0; c < NUM_COL - 3; c++)
        {
            for (int i = 0; i < 4; i++)
                set[i] = b[r + 3 - i][c + i];
            score += scoreSet(set, p);
        }
    }
    return score;
}

array<int, 2> miniMax(int ** b, int depth, int currentPlayer) 
{
    
    //Implementarea algoritmului Minimax

    if (depth == 0 || depth >= (NUM_COL * NUM_ROW) - TURNS) // daca depth este 0 sau suntem pe pozitie terminala 
        return array<int, 2>{tabScore(b, COMPUTER), -1}; // returnam scorul tablei cu ajutorul functiilor de mai sus

    if (currentPlayer == COMPUTER) // MAXimizam!
    { 
        array<int, 2> moveSoFar = { INT_MIN, -1 }; 
        if (winningMove(b, PLAYER)) // daca playerul e pe cale sa castige blocam
            return moveSoFar;
        for (int c = 0; c < NUM_COL; c++) // pentru fiecare mutare (coloana)
        { 
            if (b[NUM_ROW - 1][c] == 0) // dar daca nu e plina acea coloana (mutarea e posibila)
            { 
                int ** newBoard = copyBoard(b); // cream o copie a tablei de joc
                makeMove(newBoard, c, currentPlayer); // incercam o noua mutarea
                int score = miniMax(newBoard, depth - 1, PLAYER)[0]; //gasim scorul bazat pe urmatoarea tabla de joc care contine mutarea de mai sus 
                if (score > moveSoFar[0]) // MAXimizam alegand cel mai mare scor posibil
                    moveSoFar = { score, (int)c };
            }
        }
        return moveSoFar; // return best possible move
    }
    else //currentPlayer==PLAYER => minimizam!
    {
        array<int, 2> moveSoFar = { INT_MAX, -1 }; //MINImizam!
        if (winningMove(b, COMPUTER)) 
            return moveSoFar; // daca AI-ul e pe cale sa castige, facem acea mutare
        for (int c = 0; c < NUM_COL; c++) // pentru fiecare mutare (coloana)
        {
            if (b[NUM_ROW - 1][c] == 0) // dar daca mutarea e posibila
            {
                int ** newBoard = copyBoard(b); //copiem tabla de joc
                makeMove(newBoard, c, currentPlayer); //incercam o noua mutare
                int score = miniMax(newBoard, depth - 1, COMPUTER)[0]; //gasim scorul bazat pe urmatoarea tabla de joc care contine mutarea de mai sus
                if (score < moveSoFar[0]) //MINImizam alegand cel mai mic scor posibil
                    moveSoFar = { score, (int)c };
            }
        }
        return moveSoFar;
    }
}

int aiMove(int DEPTH) 
{
    int** pointedM = convertBoard(MATRIX);
    return 1 + miniMax(pointedM, DEPTH, COMPUTER)[1]; //returnam 1+mutare deoarece AI-ul gandeste de la 0
}

/***************************************************
                 FUNCTIA MAIN():
****************************************************/

int main()
{
    //Creating the background board with texture
    RectangleShape board(Vector2f((float)WIDTH, (float)HEIGHT));
    Texture boardTexture;
    boardTexture.loadFromFile("resources/board01.png");
    board.setTexture(&boardTexture);

    //Creating the start screen
    RectangleShape startScreen(Vector2f((float)WIDTH, (float)HEIGHT));
    Texture startScrTexture;
    startScrTexture.loadFromFile("resources/startScreen.png");
    startScreen.setTexture(&startScrTexture);

    // Reduce CPU usage by limiting the times a frame is drawn per second
    window.setFramerateLimit(30);

    //Audio
    SoundBuffer buffer;
    if (!buffer.loadFromFile("resources/tokensound.ogg"));
    Sound sound;
    sound.setBuffer(buffer);

    // Start screen:
    window.clear(Color::White);
    window.draw(startScreen);
    window.display();

    srand((int)time(0));
    int turn = 0;
    while (window.isOpen())
    {
        Vector2i mousePos = Mouse::getPosition(window);

        if (GAMEMODE == 0)
            highlightStartScreen(mousePos);
        else
            ;//highlightColWithMouse(mousePos);

        Event evnt;
        while (window.pollEvent(evnt))
        {
            if (evnt.type == Event::Closed)
                window.close();
            else if ((evnt.type == evnt.MouseButtonReleased && evnt.mouseButton.button == sf::Mouse::Left) && GAMEMODE == 0)
            {
                //mouseInRangeOfButton: 1=easy, 2=medium, 3=hard, 9=quit
                if (mouseInRangeOfButton(1, mousePos))
                    GAMEMODE = 1;
                else if (mouseInRangeOfButton(2, mousePos))
                    GAMEMODE = 2;
                else if (mouseInRangeOfButton(3, mousePos))
                    GAMEMODE = 3;
                else if (mouseInRangeOfButton(9, mousePos))
                    window.close();
                if (GAMEMODE != 0)
                {
                    window.clear(Color::White);
                    window.draw(board);
                    window.display();
                }
            }
            else if ((evnt.type == evnt.MouseButtonReleased && evnt.mouseButton.button == sf::Mouse::Left) && GAMEMODE != 0)
            {
                //Player's turn:
                Vector2i mousePos = Mouse::getPosition(window);
                int clickedColumn = mousePos.x / 170 + 1;
                if (OCCUPIED_COL[clickedColumn] >= 6)
                {
                    printf("Nu este loc pe acea coloana!");
                    break;
                }
                updateMATRIX(PLAYER, clickedColumn);
                updateVisuals(board);
                sound.play();

                if (ENDGAME)
                    ENDGAMENewScreen(startScreen);
                TURNS++;

                //Computer's turn:
                int randomColumnAvailable = (rand() % 7) + 1;
                int randomChance50 = (rand() % 2) + 1;
                int randomChance10 = (rand() % 10) + 1;
                int chosenNumberByBot=0;
                while (OCCUPIED_COL[randomColumnAvailable] >= 6)
                    randomColumnAvailable = (rand() % 7) + 1;
                sleep(seconds(.5));
                switch (GAMEMODE)
                {
                case 1: //easy
                    if (randomChance50 == 1) //50% sanse sa joace un AI cu depth 1 si 50% sa fie random
                        chosenNumberByBot = aiMove(1);
                    else
                        chosenNumberByBot = randomColumnAvailable;
                    break;
                case 2: //medium
                    if (randomChance10 == 10) //10% sanse sa joace random (aleatoriu)
                        chosenNumberByBot = randomColumnAvailable;
                    else if (randomChance10 > 4) //50% sanse sa joace un AI cu depth 2
                        chosenNumberByBot = aiMove(2);
                    else
                        chosenNumberByBot = aiMove(1); //40% sanse sa joace un AI cu depth 1
                    break;
                case 3: //hard
                    chosenNumberByBot = aiMove(6);
                    break;
                }
                updateMATRIX(COMPUTER, chosenNumberByBot);
                updateVisuals(board);
                sound.play();

                if (ENDGAME)
                    ENDGAMENewScreen(startScreen);
                TURNS++;
            }
        }

    }
    return EXIT_SUCCESS;
}
