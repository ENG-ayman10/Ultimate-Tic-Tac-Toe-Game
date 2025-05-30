#include <iostream>
#include <vector>
#include <limits>
#include <cstdlib>
#include <ctime>
#include <thread>

using namespace std;


#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN    "\033[36m"
#define BOLD    "\033[1m"

#ifdef _WIN32
#define CLEAR "cls"
#else
#define CLEAR "clear"
#endif


enum class Difficulty { EASY, MEDIUM, HARD };
enum class GameMode { HUMAN_VS_HUMAN, HUMAN_VS_AI };

struct GameState {
    char board[3][3];
    int playerXWins = 0;
    int playerOWins = 0;
    int draws = 0;
    bool gameEnded = false;
    int currentPlayer = 0; 
};


bool checkWin(const GameState& state, char player);
bool checkDraw(const GameState& state);


class AI {
public:
    virtual int getMove(GameState& state) = 0;
    virtual ~AI() = default;
};

class EasyAI : public AI {
public:
    int getMove(GameState& state) override {
        vector<int> available;
        for (int i = 0; i < 9; i++) {
            int row = i / 3, col = i % 3;
            if (isdigit(state.board[row][col]))
                available.push_back(i + 1);
        }
        if (available.empty()) return -1;
        return available[rand() % available.size()];
    }
};

class MediumAI : public AI {
public:
    int getMove(GameState& state) override {
       
        for (int i = 0; i < 9; i++) {
            int row = i / 3, col = i % 3;
            if (isdigit(state.board[row][col])) {
                char original = state.board[row][col];
                state.board[row][col] = 'O';
                if (checkWin(state, 'O')) {
                    state.board[row][col] = original;
                    return i + 1;
                }
                state.board[row][col] = original;
            }
        }

        
        for (int i = 0; i < 9; i++) {
            int row = i / 3, col = i % 3;
            if (isdigit(state.board[row][col])) {
                char original = state.board[row][col];
                state.board[row][col] = 'X';
                if (checkWin(state, 'X')) {
                    state.board[row][col] = original;
                    return i + 1;
                }
                state.board[row][col] = original;
            }
        }

        EasyAI easy;
        return easy.getMove(state);
    }
};

class HardAI : public AI {
private:
    int evaluate(const GameState& state) {
        if (checkWin(state, 'O')) return 1;
        if (checkWin(state, 'X')) return -1;
        return 0;
    }

    int minimax(GameState state, int depth, bool isMaximizing) {
        int score = evaluate(state);
        if (score != 0) return score;
        if (checkDraw(state)) return 0;

        int best = isMaximizing ? -1000 : 1000;
        for (int i = 0; i < 9; i++) {
            int row = i / 3, col = i % 3;
            if (isdigit(state.board[row][col])) {
                char original = state.board[row][col];
                state.board[row][col] = isMaximizing ? 'O' : 'X';
                int current = minimax(state, depth + 1, !isMaximizing);
                state.board[row][col] = original;
                best = isMaximizing ? max(best, current) : min(best, current);
            }
        }
        return best;
    }

public:
    int getMove(GameState& state) override {
        int bestVal = -1000;
        int bestMove = -1;

        for (int i = 0; i < 9; i++) {
            int row = i / 3, col = i % 3;
            if (isdigit(state.board[row][col])) {
                char original = state.board[row][col];
                state.board[row][col] = 'O';
                int moveVal = minimax(state, 0, false);
                state.board[row][col] = original;

                if (moveVal > bestVal) {
                    bestVal = moveVal;
                    bestMove = i + 1;
                }
            }
        }
        return bestMove;
    }
};


void initializeBoard(GameState& state) {
    char num = '1';
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            state.board[i][j] = num++;
}

void displayBoard(const GameState& state) {
    system(CLEAR);
    cout << CYAN << BOLD << "-------------" << RESET << endl;
    for (int i = 0; i < 3; i++) {
        cout << CYAN << "| " << RESET;
        for (int j = 0; j < 3; j++) {
            char cell = state.board[i][j];
            if (cell == 'X') cout << RED << BOLD << cell << RESET;
            else if (cell == 'O') cout << GREEN << BOLD << cell << RESET;
            else cout << YELLOW << cell << RESET;
            cout << CYAN << " | " << RESET;
        }
        cout << endl << "-------------" << endl;
    }
}

bool checkWin(const GameState& state, char player) {
    
    for (int i = 0; i < 3; i++) {
        if (state.board[i][0] == player && state.board[i][1] == player && state.board[i][2] == player) return true;
        if (state.board[0][i] == player && state.board[1][i] == player && state.board[2][i] == player) return true;
    }
    
    return (state.board[0][0] == player && state.board[1][1] == player && state.board[2][2] == player) ||
        (state.board[0][2] == player && state.board[1][1] == player && state.board[2][0] == player);
}

bool checkDraw(const GameState& state) {
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            if (isdigit(state.board[i][j])) return false;
    return true;
}

void playRound(GameState& state, Difficulty difficulty, GameMode mode) {
    AI* ai = nullptr;
    if (mode == GameMode::HUMAN_VS_AI) {
        switch (difficulty) {
        case Difficulty::EASY: ai = new EasyAI(); break;
        case Difficulty::MEDIUM: ai = new MediumAI(); break;
        case Difficulty::HARD: ai = new HardAI(); break;
        }
    }

    initializeBoard(state);
    state.currentPlayer = 0;
    state.gameEnded = false;

    while (!state.gameEnded) {
        displayBoard(state);

        if (mode == GameMode::HUMAN_VS_AI && state.currentPlayer == 1) {
            cout << MAGENTA << "AI is thinking..." << RESET << endl;
            this_thread::sleep_for(chrono::seconds(1));
            int move = ai->getMove(state);
            if (move == -1) break;
            int row = (move - 1) / 3, col = (move - 1) % 3;
            state.board[row][col] = 'O';
        }
        else {
            int choice;
            do {
                cout << BOLD << "Player " << (state.currentPlayer == 0 ? "X" : "O") << " enter move (1-9): " << RESET;
                cin >> choice;
                if (cin.fail() || choice < 1 || choice > 9) {
                    cin.clear();
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
                    cout << RED << "Invalid input! " << RESET;
                    continue;
                }
                int row = (choice - 1) / 3, col = (choice - 1) % 3;
                if (isdigit(state.board[row][col])) {
                    state.board[row][col] = (state.currentPlayer == 0) ? 'X' : 'O';
                    break;
                }
                else {
                    cout << RED << "Cell occupied! " << RESET;
                }
            } while (true);
        }

        if (checkWin(state, state.currentPlayer == 0 ? 'X' : 'O')) {
            displayBoard(state);
            cout << GREEN << BOLD << "Player " << (state.currentPlayer == 0 ? "X" : "O") << " wins!" << RESET << endl;
            (state.currentPlayer == 0) ? state.playerXWins++ : state.playerOWins++;
            state.gameEnded = true;
        }
        else if (checkDraw(state)) {
            displayBoard(state);
            cout << YELLOW << BOLD << "Game is a draw!" << RESET << endl;
            state.draws++;
            state.gameEnded = true;
        }
        else {
            state.currentPlayer = (state.currentPlayer + 1) % 2;
        }
    }
    if (ai) delete ai;
}

int main() {
    srand(static_cast<unsigned int>(time(0)));
    GameState state;

    while (true) {
        system(CLEAR);
        cout << CYAN << BOLD << "======== Ultimate Tic-Tac-Toe ========" << RESET << endl
            << "1. Human vs Human\n2. Human vs AI\n3. Exit\nChoice: ";
        int choice;
        cin >> choice;
        if (choice == 3) break;

        GameMode mode = (choice == 1) ? GameMode::HUMAN_VS_HUMAN : GameMode::HUMAN_VS_AI;
        Difficulty difficulty = Difficulty::EASY;

        if (mode == GameMode::HUMAN_VS_AI) {
            cout << "Select Difficulty:\n1. Easy\n2. Medium\n3. Hard\nChoice: ";
            int diff;
            cin >> diff;
            diff = (diff < 1) ? 1 : (diff > 3) ? 3 : diff;
            difficulty = static_cast<Difficulty>(diff - 1);
        }

        playRound(state, difficulty, mode);

        cout << "\nPlay again? (y/n): ";
        char answer;
        cin >> answer;
        if (tolower(answer) != 'y') break;
    }

    cout << "\nFinal Results:\n"
        << "X Wins: " << state.playerXWins << "\n"
        << "O Wins: " << state.playerOWins << "\n"
        << "Draws: " << state.draws << endl;

    return 0;
}