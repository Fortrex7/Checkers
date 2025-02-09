#pragma once
#include <random>
#include <vector>

#include "../Models/Move.h"
#include "Board.h"
#include "Config.h"

const int INF = 1e9;

class Logic
{
  public:
    // Конструктор, инициализирующий ссылку на объект доски и настройки конфигурации
    Logic(Board *board, Config *config) : board(board), config(config)
    {
        // Использование случайного генератора для бота
        rand_eng = std::default_random_engine (
            !((*config)("Bot", "NoRandom")) ? unsigned(time(0)) : 0);
        scoring_mode = (*config)("Bot", "BotScoringType");
        optimization = (*config)("Bot", "Optimization");
    }

   

private:
    // Метод для выполнения хода и возврата новой матрицы
    vector<vector<POS_T>> make_turn(vector<vector<POS_T>> mtx, move_pos turn) const
    {
        if (turn.xb != -1)
            mtx[turn.xb][turn.yb] = 0; // Удаление побитой фигуры
        if ((mtx[turn.x][turn.y] == 1 && turn.x2 == 0) || (mtx[turn.x][turn.y] == 2 && turn.x2 == 7))
            mtx[turn.x][turn.y] += 2; // Превращение в дамку
        mtx[turn.x2][turn.y2] = mtx[turn.x][turn.y]; // Перемещение фигуры
        mtx[turn.x][turn.y] = 0; // Обнуление старой позиции
        return mtx; // Возвращение изменённой матрицы
    }

    // Метод для вычисления оценки текущего состояния доски
    double calc_score(const vector<vector<POS_T>> &mtx, const bool first_bot_color) const
    {
        // color - who is max player
        double w = 0, wq = 0, b = 0, bq = 0; // Переменные для подсчёта количества фигур и дамок
        for (POS_T i = 0; i < 8; ++i) // Перебор всех позиций на доске
        {
            for (POS_T j = 0; j < 8; ++j)
            {
                w += (mtx[i][j] == 1); // Подсчёт белых фигур
                wq += (mtx[i][j] == 3); // Подсчёт белых дамок
                b += (mtx[i][j] == 2); // Подсчёт чёрных фигур
                bq += (mtx[i][j] == 4); // Подсчёт чёрных дамок
                if (scoring_mode == "NumberAndPotential")
                {
                    w += 0.05 * (mtx[i][j] == 1) * (7 - i); // Оценка позиций белых
                    b += 0.05 * (mtx[i][j] == 2) * (i); // Оценка позиций чёрных
                }
            }
        }
        // Обмен значениями счётчиков, если текущий цвет не цвет первого бота
        if (!first_bot_color)
        {
            swap(b, w); // Меняем количество фигур
            swap(bq, wq); // Меняем количество дамок
        }
        // Обработка случаев, когда ни одна из сторон не имеет фигур
        if (w + wq == 0)
            return INF; // Возвращаем бесконечность, если нет белых фигур
        if (b + bq == 0)
            return 0; // Возвращаем 0, если нет чёрных фигур
        int q_coef = 4; // Коэффициент, используемый для дамок
        if (scoring_mode == "NumberAndPotential")
        {
            q_coef = 5; // Изменение коэффициента для этого режима
        }
        // Возвращает отношение количества вражеских фигур (с учётом дамок) к количеству своих фигур (с учётом дамок)
        return (b + bq * q_coef) / (w + wq * q_coef);
    }

    

public:
    // Перегруженная функция для поиска возможных ходов с заданным цветом фигур
    void find_turns(const bool color)
    {
        find_turns(color, board->get_board()); // Вызов функции поиска с текущей доской
    }

    // Перегруженная функция для поиска возможных ходов для фигуры по её координатам
    void find_turns(const POS_T x, const POS_T y)
    {
        find_turns(x, y, board->get_board()); // Вызов функции поиска с текущей доской
    }

private:
    // Функция выполняет поиск всех возможных ходов для фигур заданного цвета на доске
    void find_turns(const bool color, const vector<vector<POS_T>> &mtx)
    {
        vector<move_pos> res_turns; // Вектор для хранения найденных ходов
        bool have_beats_before = false; // Флаг наличия ходов с побитием
        for (POS_T i = 0; i < 8; ++i) // Перебор всех позиций на доске
        {
            for (POS_T j = 0; j < 8; ++j)
            {
                if (mtx[i][j] && mtx[i][j] % 2 != color) // Проверка на наличие фигуры определенного цвета
                {
                    find_turns(i, j, mtx); // Поиск возможных ходов для найденной фигуры
                    if (have_beats && !have_beats_before)
                    {
                        have_beats_before = true; // Если найдены побития, обновляем флаг
                        res_turns.clear(); // Очищаем вектор ходов
                    }
                    // Если есть побития или не было побитий раньше, добавляем новые ходы
                    if ((have_beats_before && have_beats) || !have_beats_before)
                    {
                        res_turns.insert(res_turns.end(), turns.begin(), turns.end());
                    }
                }
            }
        }
        turns = res_turns; // Сохраняем все найденные ходы
        shuffle(turns.begin(), turns.end(), rand_eng); // Перемешиваем найденные ходы
        have_beats = have_beats_before; // Обновляем статус наличия побитий
    }

    // Обновляем статус наличия побитий
    void find_turns(const POS_T x, const POS_T y, const vector<vector<POS_T>> &mtx)
    {
        turns.clear(); // Очищаем вектор найденных ходов
        have_beats = false; // Сбрасываем флаг наличия побитий
        POS_T type = mtx[x][y]; // Получаем тип фигуры по её координатам
        // Проверка на побития для обычных шашек
        switch (type)
        {
        case 1: // Обычная белая шашка
        case 2: // Обычная черная шашка
            // Проверка ходов с побитием по диагонали
            for (POS_T i = x - 2; i <= x + 2; i += 4)
            {
                for (POS_T j = y - 2; j <= y + 2; j += 4)
                {
                    if (i < 0 || i > 7 || j < 0 || j > 7) // Проверка границ доски
                        continue;
                    POS_T xb = (x + i) / 2, yb = (y + j) / 2; // Позиция побитой шашки
                    // Условия для добавления хода с побитием
                    if (mtx[i][j] || !mtx[xb][yb] || mtx[xb][yb] % 2 == type % 2)
                        continue;
                    turns.emplace_back(x, y, i, j, xb, yb); // Добавляем возможный ход с побитием
                }
            }
            break;
        default: // Обработка дамок
            // Проверка ходов с побитием для дамок
            for (POS_T i = -1; i <= 1; i += 2)
            {
                for (POS_T j = -1; j <= 1; j += 2)
                {
                    POS_T xb = -1, yb = -1; // Инициализация переменных для позиции побитой шашки
                    for (POS_T i2 = x + i, j2 = y + j; i2 != 8 && j2 != 8 && i2 != -1 && j2 != -1; i2 += i, j2 += j)
                    {
                        if (mtx[i2][j2]) // Проверка наличия фигуры на проверяемой позиции
                        {
                            if (mtx[i2][j2] % 2 == type % 2 || (mtx[i2][j2] % 2 != type % 2 && xb != -1))
                            {
                                break; // Прерывание, если фигура одного цвета или уже найдена одна побитая шашка
                            }
                            // Сохраняем позицию последней фигуры для возможного побития
                            xb = i2;
                            yb = j2;
                        }
                        if (xb != -1 && xb != i2) // Проверка, добавлять ли ход
                        {
                            turns.emplace_back(x, y, i2, j2, xb, yb); // Добавление хода с побитием
                        }
                    }
                }
            }
            break;
        }
        // Проверка других возможных ходов
        if (!turns.empty()) // Если хоть один ход найден
        {
            have_beats = true; // Обновляем статус наличия побитий
            return; // Завершаем функцию
        }
        switch (type)
        {
        case 1: // Обычная белая шашка
        case 2: // Обычная черная шашка
            // Проверка шашек
            {
                POS_T i = ((type % 2) ? x - 1 : x + 1); // Определяем направление движения
                for (POS_T j = y - 1; j <= y + 1; j += 2)
                {
                    if (i < 0 || i > 7 || j < 0 || j > 7 || mtx[i][j]) // Проверка границ доски и свободных клеток
                        continue;
                    turns.emplace_back(x, y, i, j); // Добавляем возможный обычный ход
                }
                break;
            }
        default:
            // Проверка дамок
            for (POS_T i = -1; i <= 1; i += 2)
            {
                for (POS_T j = -1; j <= 1; j += 2)
                {
                    for (POS_T i2 = x + i, j2 = y + j; i2 != 8 && j2 != 8 && i2 != -1 && j2 != -1; i2 += i, j2 += j)
                    {
                        if (mtx[i2][j2]) // Прерывание при наличии фигуры
                            break;
                        turns.emplace_back(x, y, i2, j2); // Добавление обычного хода для дамки
                    }
                }
            }
            break;
        }
    }

  public:
    vector<move_pos> turns; // Вектор возможных ходов
    bool have_beats; // Статус наличия побитий
    int Max_depth; // Максимальная глубина поиска

  private:
    default_random_engine rand_eng; // Генератор случайных чисел
    string scoring_mode; // Режим оценки
    string optimization; // Режим оптимизации
    vector<move_pos> next_move; // Вектор следующих ходов
    vector<int> next_best_state; // Вектор состояний для следующих ходов
    Board *board; // Указатель на объект доски
    Config *config; // Указатель на объект конфигурации
};
