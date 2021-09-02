#include <Windows.h>
#include <iostream>
#include <vector>
#include <time.h>
#include <fstream>
#include <string>

#if !defined(_WIN32) && !defined(_WIN64) 
static_assert(false, "Este código está implementado para usarse en windows. Para usarlo en otro S.O. modifique las funciones GetAsyncKeyState y system");
#endif

struct Position
{
    int x = 0, y = 0;

    Position(const int x, const int y) : x(x), y(y) {}

    inline auto operator == (const Position& position) const -> bool
    {
        // Operador para comparar 2 posiciones
        //
        return this->x == position.x && this->y == position.y;
    }
};

template<std::size_t rows, std::size_t columns>
class Board
{
private:
    char board_[rows][columns] = { 0 }; // Tablero

public:
    auto Print() const -> void
    {
        // Imprimir tablero
        //
        for (std::size_t row = 0; row < rows; ++row)
        {
            for (std::size_t column = 0; column < columns; ++column)
            {
                std::cout << this->board_[row][column] << " ";
            }

            std::cout << "|\n";
        }

        for (std::size_t column = 0; column < columns; ++column)
        {
            std::cout << "- ";
        }
    }

    inline auto Clear() const -> void
    {
        // Limpiar pantalla
        //
        system("cls");
    }

    inline auto operator [] (const std::size_t row) -> char*
    {
        // Operador para obtener el tablero
        //
        return reinterpret_cast<char*>(this->board_[row]);
    }
};

// Dirección de la serpiente
//
const enum class Direction { kNone, kLeft, kRight, kUp, kDown };

template<std::size_t rows, std::size_t columns>
class Snake
{
    // Verificar tamaño del tablero
    //
    static_assert(rows >= 5, "La cantidad de filas tiene que ser mayor o igual a 5");
    static_assert(columns >= 5, "La cantidad de columnas tiene que ser mayor o igual a 5");

private:
    Board<rows, columns> board_;                            // Tablero
    std::vector<Position> snake_    = { Position(std::size_t(columns / 2), std::size_t(rows / 2)) };   // Serpiente
    Direction current_direction_    = Direction::kNone;     // Dirección actual de la serpiente
    Direction keys_[2]              = { Direction::kNone }; // Arreglo de teclas presionadas
    Position target_                = Position(0, 0);       // Punto objetivo
    std::size_t difficulty_         = 0;                    // Dificultad de la serpiente
    clock_t time_                   = 0;                    // Contador tiempo
    std::size_t record_             = 0;                    // Record de puntaje

    inline auto MoveRight() -> void
    {
        // Verifica movimiento válido
        //
        if (this->current_direction_ != Direction::kLeft && this->current_direction_ != Direction::kRight && this->keys_[0] == Direction::kNone)
        {
            this->keys_[0] = Direction::kRight;
            return;
        }
        
        if (this->keys_[0] != Direction::kLeft && this->keys_[0] != Direction::kRight && this->keys_[0] != Direction::kNone && this->keys_[1] == Direction::kNone)
        {
            this->keys_[1] = Direction::kRight;
            return;
        }
    }

    inline auto MoveLeft() -> void
    {
        // Verifica movimiento válido
        //
        if (this->current_direction_ != Direction::kRight && this->current_direction_ != Direction::kLeft && this->keys_[0] == Direction::kNone)
        {
            this->keys_[0] = Direction::kLeft;
            return;
        }

        if (this->keys_[0] != Direction::kRight && this->keys_[0] != Direction::kLeft && this->keys_[0] != Direction::kNone && this->keys_[1] == Direction::kNone)
        {
            this->keys_[1] = Direction::kLeft;
            return;
        }
    }

    inline auto MoveUp() -> void
    {
        // Verifica movimiento válido
        //
        if (this->current_direction_ != Direction::kDown && this->current_direction_ != Direction::kUp && this->keys_[0] == Direction::kNone)
        {
            this->keys_[0] = Direction::kUp;
            return;
        }

        if (this->keys_[0] != Direction::kDown && this->keys_[0] != Direction::kUp && this->keys_[0] != Direction::kNone && this->keys_[1] == Direction::kNone)
        {
            this->keys_[1] = Direction::kUp;
            return;
        }
    }

    inline auto MoveDown() -> void
    {
        // Verifica movimiento válido
        //
        if (this->current_direction_ != Direction::kUp && this->current_direction_ != Direction::kDown && this->keys_[0] == Direction::kNone)
        {
            this->keys_[0] = Direction::kDown;
            return;
        }

        if (this->keys_[0] != Direction::kUp && this->keys_[0] != Direction::kDown && this->keys_[0] != Direction::kNone && this->keys_[1] == Direction::kNone)
        {
            this->keys_[1] = Direction::kDown;
            return;
        }
    }

    auto UpdateBoard() -> void
    {
        // Actualiza la posición de la serpiente y el objetivo en el tablero
        //
        for (std::size_t row = 0; row < rows; row++)
        {
            for (std::size_t column = 0; column < columns; column++)
            {
                for (Position snake_pos : this->snake_)
                {
                    if (Position(column, row) == snake_pos)
                    {
                        this->board_[row][column] = '*';
                        break;
                    }
                    else
                    {
                        this->board_[row][column] = 0;
                    }
                }

                if (Position(column, row) == this->target_)
                {
                    this->board_[row][column] = '*';
                }
            }
        }
    }

    auto Update() -> bool
    {
        // Actualiza la posición de la serpiente y verifica colisiones
        //
        Position next_position = this->snake_[0];
        bool is_valid_position = false;

        if (this->keys_[1] != Direction::kNone)
        {
            this->current_direction_ = this->keys_[0];
            this->keys_[0] = this->keys_[1];
            this->keys_[1] = Direction::kNone;
        }
        else
        {
            if (this->keys_[0] != Direction::kNone)
            {
                this->current_direction_ = this->keys_[0];
                this->keys_[0] = Direction::kNone;
            }
        }

        switch (this->current_direction_)
        {
        case Direction::kNone:
            return true;
        case Direction::kLeft:
            next_position.x -= 1;
            is_valid_position = next_position.x >= 0;
            break;
        case Direction::kRight:
            next_position.x += 1;
            is_valid_position = next_position.x < columns;
            break;
        case Direction::kUp:
            next_position.y -= 1;
            is_valid_position = next_position.y >= 0;
            break;
        case Direction::kDown:
            next_position.y += 1;
            is_valid_position = next_position.y < rows;
            break;
        default:
            break;
        }

        if (next_position == this->target_)
        {
            this->target_ = Position(rand() % rows, rand() % columns);
            this->snake_.insert(this->snake_.begin(), next_position);
            this->UpdateBoard();
        }
        else
        {
            if (is_valid_position && this->board_[next_position.y][next_position.x] != '*')
            {
                this->snake_.pop_back();
                this->snake_.insert(this->snake_.begin(), next_position);
                this->UpdateBoard();
            }
            else
            {
                return false;
            }
        }

        return true;
    }

public:
    Snake(std::size_t difficulty)
    {
        this->difficulty_ = difficulty;
        srand(clock());
        this->target_ = Position(rand() % rows, rand() % columns);

        // Leer el record guardado en record.txt
        std::string record;
        std::ifstream record_file("record.txt");

        if (record_file.is_open())
        {
            while (record_file.good())
            {
                std::getline(record_file, record);
                if (!record.empty())
                    this->record_ = std::stoi(record);
            }

            record_file.close();
        }
        else
        {
            std::ofstream outfile("record.txt");
            outfile.close();
        }

        this->UpdateBoard();
    }

    auto Play() -> bool
    {
        // Verificar si el usuario presiona teclas de movimiento
        //
        if (GetAsyncKeyState(VK_RIGHT) & 1)
        {
            this->MoveRight();
        }

        if (GetAsyncKeyState(VK_LEFT) & 1)
        {
            this->MoveLeft();
        }

        if (GetAsyncKeyState(VK_UP) & 1)
        {
            this->MoveUp();
        }

        if (GetAsyncKeyState(VK_DOWN) & 1)
        {
            this->MoveDown();
        }

        // Actualizar la posición de la serpiente cada 'difficulty_' tiempo
        //
        if (clock() - this->time_ > this->difficulty_)
        {
            this->time_ = clock();
            bool result = this->Update(); // Actualizar antes de imprimir para evitar lag
            this->board_.Clear(); // Limpiar pantalla
            this->board_.Print(); // Imprimir tablero
            std::cout << "\n" << "Puntaje: " << this->snake_.size() - 1 << "\n"; // Imprimir puntuación
            std::cout << "Record: " << this->record_ << "\n";

            if (this->snake_.size() > this->record_)
            {
                this->record_ = this->snake_.size();
                std::ofstream record_file("record.txt");
                record_file << this->record_;
                record_file.close();
            }

            return result;
        }

        return true;
    }
};

auto main() -> int
{
    unsigned int difficulty = 0;

    // Menú
    //
    while (difficulty != 1 && difficulty != 2 && difficulty != 3 && difficulty != 4)
    {
        std::cout << "--- Juego snake ---\n";
        std::cout << "Seleccione la dificultad:\n";
        std::cout << "[1] - Extremo\n";
        std::cout << "[2] - Dificil\n";
        std::cout << "[3] - Medio\n";
        std::cout << "[4] - Facil\n";
        std::cin >> difficulty;
    }
    
    // Instanciar snake
    //
    Snake<15, 15> snake({ difficulty * 100 - 100 });

    // Bucle principal
    //
    while (snake.Play()) {}

    system("pause");
}
