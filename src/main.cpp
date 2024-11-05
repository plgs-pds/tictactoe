#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <array>
#include <ctime>

// Classe TicTacToe
class TicTacToe {
private:
    std::array<std::array<char, 3>, 3> board; // Tabuleiro do jogo
    std::mutex board_mutex;
    std::condition_variable turn_cv; // Sincronização entre turnos
    char current_player; // Jogador atual ('X' ou 'O')
    bool game_over;
    char winner;

public:
    TicTacToe() : board{{{' ', ' ', ' '}, {' ', ' ', ' '}, {' ', ' ', ' '}}}, 
                  current_player('X'), game_over(false), winner(' ') {}

    void display_board() {
        std::lock_guard<std::mutex> lock(board_mutex);
        for (const auto& row : board) {
            for (const auto& cell : row) {
                std::cout << (cell == ' ' ? '-' : cell) << " ";
            }
            std::cout << std::endl;
        }
        std::cout << std::endl;
    }

    bool make_move(char player, int row, int col) {
        std::lock_guard<std::mutex> lock(board_mutex);
        if (board[row][col] == ' ' && !game_over) {
            board[row][col] = player;
            if (check_win(player)) {
                game_over = true;
                winner = player;
            } else if (check_draw()) {
                game_over = true;
                winner = 'D'; // Empate
            }
            current_player = (current_player == 'X') ? 'O' : 'X';
            turn_cv.notify_all();
            return true;
        }
        return false;
    }

    bool check_win(char player) {
        // Verifica linhas, colunas e diagonais para vitória
        for (int i = 0; i < 3; ++i) {
            if ((board[i][0] == player && board[i][1] == player && board[i][2] == player) || 
                (board[0][i] == player && board[1][i] == player && board[2][i] == player))
                return true;
        }
        if ((board[0][0] == player && board[1][1] == player && board[2][2] == player) ||
            (board[0][2] == player && board[1][1] == player && board[2][0] == player))
            return true;

        return false;
    }

    bool check_draw() {
        for (const auto& row : board) {
            for (const auto& cell : row) {
                if (cell == ' ') return false;
            }
        }
        return true;
    }

    bool is_game_over() {
        return game_over;
    }

    char get_winner() {
        return winner;
    }

    char get_current_player() {
        return current_player;
    }

    void wait_for_turn(char player) {
        std::unique_lock<std::mutex> lock(board_mutex);
        turn_cv.wait(lock, [&] { return current_player == player || game_over; });
    }
};

// Classe Player
class Player {
private:
    TicTacToe& game;
    char symbol;
    std::string strategy;

public:
    Player(TicTacToe& g, char s, std::string strat) 
        : game(g), symbol(s), strategy(strat) {}

    void play() {
        while (!game.is_game_over()) {
            game.wait_for_turn(symbol);
            if (game.is_game_over()) break;
            game.display_board();
            if (strategy == "sequential") {
                play_sequential();
            } else if (strategy == "random") {
                play_random();
            }
        }
    }

private:
    void play_sequential() {
        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 3; ++j) {
                if (game.make_move(symbol, i, j)) {
                    return;
                }
            }
        }
    }

    void play_random() {
        while (true) {
            int row = rand() % 3;
            int col = rand() % 3;
            if (game.make_move(symbol, row, col)) {
                return;
            }
        }
    }
};

// Função principal
int main() {
    std::srand(std::time(0)); // Inicializa a semente para geração aleatória

    TicTacToe game;
    Player player1(game, 'X', "sequential");
    Player player2(game, 'O', "random");

    std::thread t1(&Player::play, &player1);
    std::thread t2(&Player::play, &player2);

    t1.join();
    t2.join();

    game.display_board();
    char winner = game.get_winner();
    if (winner == 'D') {
        std::cout << "Empate!" << std::endl;
    } else {
        std::cout << "Vencedor: " << winner << std::endl;
    }

    return 0;
}
