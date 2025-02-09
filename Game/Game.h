#pragma once
#include <chrono>
#include <thread>

#include "../Models/Project_path.h"
#include "Board.h"
#include "Config.h"
#include "Hand.h"
#include "Logic.h"

class Game
{
  public:
    Game() : board(config("WindowSize", "Width"), config("WindowSize", "Hight")), hand(&board), logic(&board, &config)
    {
        ofstream fout(project_path + "log.txt", ios_base::trunc);
        fout.close();
    }

    // Запуск шашек
    int play()
    {
        auto start = chrono::steady_clock::now(); //Таймер засекает начало работы программы
        if (is_replay) //Если выбрана переигровка
        {
            logic = Logic(&board, &config);
            config.reload();
            board.redraw();
        }
        else
        {
            board.start_draw(); //Если не переигровка, отрисовывается доска
        }
        is_replay = false;

        int turn_num = -1;
        bool is_quit = false;
        const int Max_turns = config("Game", "MaxNumTurns"); //Получение максимального количества ходов из config
        //Основной цикл игры
        while (++turn_num < Max_turns)
        {
            beat_series = 0;
            logic.find_turns(turn_num % 2);
            if (logic.turns.empty())
                break;
            //Установка уровня сложности бота
            logic.Max_depth = config("Bot", string((turn_num % 2) ? "Black" : "White") + string("BotLevel"));
            //Оперделение по цвету кто ходит первым бот или не бот
            if (!config("Bot", string("Is") + string((turn_num % 2) ? "Black" : "White") + string("Bot")))
            {
                auto resp = player_turn(turn_num % 2); //Вызов функции хода игрока
                //Обработка ответов
                if (resp == Response::QUIT) //Если нажата кнопка выход
                {
                    is_quit = true;
                    break;
                }
                else if (resp == Response::REPLAY) //Если нажата кнопка переиграть
                {
                    is_replay = true;
                    break;
                }
                else if (resp == Response::BACK) //Если нажата кнопка отмены хода
                {
                    // Проверяем, был ли предыдущий ход ботом и нет ли взятий, чтобы откатить ход
                    if (config("Bot", string("Is") + string((1 - turn_num % 2) ? "Black" : "White") + string("Bot")) &&
                        !beat_series && board.history_mtx.size() > 2)
                    {
                        board.rollback();
                        --turn_num;
                    }
                    if (!beat_series)
                        --turn_num;

                    board.rollback();
                    --turn_num;
                    beat_series = 0;
                }
            }
            else
                bot_turn(turn_num % 2); //Переход хода к боту и вызов функции хода бота
        }
        auto end = chrono::steady_clock::now(); // Таймер засекает конец работы программы
        ofstream fout(project_path + "log.txt", ios_base::app);
        fout << "Game time: " << (int)chrono::duration<double, milli>(end - start).count() << " millisec\n";
        fout.close();

        // Проверка ответа после окончания игры
        if (is_replay)
            return play(); // Если переигровка, рекурсивно запускается игра заново
        if (is_quit)
            return 0; // Возвращается 0, если вышли из игры
        int res = 2;
        if (turn_num == Max_turns) // Если достигнуто максимальное количество ходов, то ничья
        {
            res = 0;
        }
        else if (turn_num % 2) // Если игра закончилась нечетным ходом, победа черного
        {
            res = 1;
        }
        board.show_final(res); // Ожидание результата игры
        auto resp = hand.wait(); // Ожидание следующего ввода

        //Проверка на переигровку
        if (resp == Response::REPLAY)
        {
            is_replay = true;
            return play();
        }
        return res; // Возвращаем конечный результат
    }

  private:
      // Функция хода бота
    void bot_turn(const bool color)
    {
        auto start = chrono::steady_clock::now(); // Запоминаем время начала хода бота

        auto delay_ms = config("Bot", "BotDelayMS"); // Получаем задержку для бота между ходами
        // Создаем новый поток для обеспечения задержки между ходами бота
        thread th(SDL_Delay, delay_ms);
        auto turns = logic.find_best_turns(color); // Находим лучшие ходы для бота в зависимости от цвета
        th.join(); // Ждем завершения потока задержки, чтобы добавить паузу до начала хода
        bool is_first = true; // Флаг для отслеживания, является ли текущий ход первым
        // Выполнение ходов бота
        for (auto turn : turns)
        {
            if (!is_first) // Если это не первый ход
            {
                SDL_Delay(delay_ms); // Ожидаем задержку между ходами
            }
            is_first = false; // После первого хода устанавливаем флаг на false
            beat_series += (turn.xb != -1); // Увеличиваем количество взятий, если текущий ход включает взятие
            board.move_piece(turn, beat_series); // Делаем ход на доске, передавая информацию о текущем ходе и количестве взятий
        }

        auto end = chrono::steady_clock::now(); // Запоминаем время окончания хода
        // Записываем в лог время, затраченное на ход бота
        ofstream fout(project_path + "log.txt", ios_base::app);
        fout << "Bot turn time: " << (int)chrono::duration<double, milli>(end - start).count() << " millisec\n";
        fout.close();
    }

    //Функция хода игрока
    Response player_turn(const bool color) 
    {
        // Возвращает 1, если выход
        vector<pair<POS_T, POS_T>> cells; // Вектор для хранения доступных клеток
        
        // Заполняем вектор доступными ячейками для текущего игрока
        for (auto turn : logic.turns)
        {
            cells.emplace_back(turn.x, turn.y); // Добавление возможных клеток в вектор
        }
        board.highlight_cells(cells); // Подсвечиваем доступные ячейки для выбора
        move_pos pos = {-1, -1, -1, -1}; // Инициализация структуры для хранения выбранного хода
        POS_T x = -1, y = -1; // Инициализация координат текущей ячейки
        
        // Цикл для выполнения первого хода
        while (true)
        {
            auto resp = hand.get_cell(); // Получаем ячейку, по которой кликнул игрок
            
            // Проверяем, является ли результат клика обозначением ячейки
            if (get<0>(resp) != Response::CELL)
                return get<0>(resp); // Возвращаем ответ, который не является ячейкой
            pair<POS_T, POS_T> cell{get<1>(resp), get<2>(resp)}; // Получаем координаты выбранной ячейки

            bool is_correct = false; // Флаг для проверки корректности выбора ячейки
            
            // Перебираем доступные ходы и проверяем, является ли выбранная ячейка корректной
            for (auto turn : logic.turns)
            {
                if (turn.x == cell.first && turn.y == cell.second)
                {
                    is_correct = true; // Ячейка доступна для выбора
                    break;
                }
                
                // Проверка старой позиции, чтобы определить, был ли выбран правильный ход
                if (turn == move_pos{x, y, cell.first, cell.second})
                {
                    pos = turn; // Устанавливаем переменную позиции хода
                    break; 
                }
            }
            if (pos.x != -1) // Если старая позиция была выбрана
                break; // Выходим из основного цикла
            
            if (!is_correct) // Проверка на корректность выбора ячейки
            {
                if (x != -1) // Если ранее была выбрана другая ячейка
                {
                    board.clear_active(); // Убираем выделение активной ячейки
                    board.clear_highlight(); // Убираем подсветку доступных клеток
                    board.highlight_cells(cells); // Снова подсвечиваем доступные клетки
                }
                x = -1; // Сбрасываем координаты
                y = -1;
                continue; // Возвращаемся в начало цикла для новой попытки выбора
            }

            // Обновление координат на основании выбора игрока
            x = cell.first;
            y = cell.second;
            board.clear_highlight(); // Очищаем подсветку предыдущих ячеек
            board.set_active(x, y); // Устанавливаем активную ячейку
            vector<pair<POS_T, POS_T>> cells2; // Вектор для хранения новых доступных клеток
            
            // Заполнение новых доступных клеток после выбора ячейки игроком
            for (auto turn : logic.turns)
            {
                if (turn.x == x && turn.y == y)
                {
                    cells2.emplace_back(turn.x2, turn.y2); // Добавляем доступные клетки для нового состояния
                }
            }
            board.highlight_cells(cells2); // Подсвечиваем новые возможные клетки для перемещения
        }
        // Отмена подсветки и активного выделения после выбора
        board.clear_highlight();
        board.clear_active();
        board.move_piece(pos, pos.xb != -1); // Перемещаем фигуру игрока на доске
        
        if (pos.xb == -1) // Если не было взятия
            return Response::OK; // Возвращаем статус успешного хода
        
        // Продолжаем взятие, пока возможно
        beat_series = 1; // Устанавливаем начальную серию взятий
        while (true)
        {
            logic.find_turns(pos.x2, pos.y2); // Ищем доступные ходы с текущей позицией
            if (!logic.have_beats) // Если взятий нет, выходим из цикла
                break;

            vector<pair<POS_T, POS_T>> cells; // Вектор доступных клеток
            // Заполняем вектор доступных клеток для новой позиции
            for (auto turn : logic.turns)
            {
                cells.emplace_back(turn.x2, turn.y2);
            }
            board.highlight_cells(cells); // Подсвечиваем доступные клетки для нового взятия
            board.set_active(pos.x2, pos.y2); // Устанавливаем активную позицию
            
            // Цикл для выполнения хода
            while (true)
            {
                auto resp = hand.get_cell(); // Получаем ячейку для следующего хода
                if (get<0>(resp) != Response::CELL) // Если был получен не ход
                    return get<0>(resp); // Возврат ответа

                pair<POS_T, POS_T> cell{get<1>(resp), get<2>(resp)}; // Получаем координаты следующей ячейки

                bool is_correct = false; // Флаг для проверки корректности следующего хода
                // Проверяем, корректный ли ход
                for (auto turn : logic.turns)
                {
                    if (turn.x2 == cell.first && turn.y2 == cell.second) // Если текущая ячейка доступна
                    {
                        is_correct = true; // Ход допустим
                        pos = turn; // Обновляем позицию
                        break; // Выходим из цикла
                    }
                }
                if (!is_correct) // Если ход некорректный
                    continue; // Возвращаемся в начало цикла

                board.clear_highlight(); // Очищаем подсветку клеток
                board.clear_active(); // Убираем выделение с текущей ячейки
                beat_series += 1; // Увеличиваем количество взятий
                board.move_piece(pos, beat_series); // Перемещаем фигуру на доске
                break; // Выход из цикла, чтобы завершить текущий ход
            }
        }

        return Response::OK; // Возвращаем статус успешного выполнения хода
    }

  private:
    Config config;
    Board board;
    Hand hand;
    Logic logic;
    int beat_series;
    bool is_replay = false;
};
