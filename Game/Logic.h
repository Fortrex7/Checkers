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
    Logic(Board* board, Config* config) : board(board), config(config)
    {
        // Использование случайного генератора для бота
        rand_eng = std::default_random_engine(
            !((*config)("Bot", "NoRandom")) ? unsigned(time(0)) : 0);
        scoring_mode = (*config)("Bot", "BotScoringType");
        optimization = (*config)("Bot", "Optimization");
    }

    // Метод для нахождения лучших ходов для указанного цвета
    vector<move_pos> find_best_turns(const bool color) {
        // Очищаем векторы для хранения следующих оптимальных ходов и состояний
        next_move.clear();
        next_best_state.clear();

        // Запускаем рекурсивный поиск первого лучшего хода
        find_first_best_turn(board->get_board(), color, -1, -1, 0);

        // Вектор для хранения найденных лучших ходов
        vector<move_pos> res;
        int state = 0; // Инициализация состояния, с которого начинается поиск
        // Цикл для извлечения найденных ходов из структуры next_move
        do {
            res.push_back(next_move[state]); // Добавляем текущий лучший ход в результат
            state = next_best_state[state]; // Переходим к следующему состоянию
        } while (state != -1 && next_move[state].x != -1); // Продолжаем, пока есть доступные ходы
        return res; // Возвращаем вектор с найденными лучшими ходами
    }

private:
    // Метод для поиска первого лучшего хода в заданной позиции
    double find_first_best_turn(vector<vector<POS_T>> mtx, const bool color, const POS_T x, const POS_T y, size_t state,
        double alpha = -1) {
        // Инициализация нового хода со значениями -1
        next_move.emplace_back(-1, -1, -1, -1);
        // Сохранение состояния для перевода к следующему шагу
        next_best_state.push_back(-1);
        // Если состояние не 0, ищем возможные ходы для фигуры
        if (state != 0) {
            find_turns(x, y, mtx);
        }
        // Сохраняем текущее состояние возможных ходов и флаг побитий
        auto now_turns = turns;
        auto now_have_beats = have_beats;

        // Если нет возможности побитий, вызываем рекурсивную функцию для следующего цвета
        if (!now_have_beats && state != 0) {
            return find_best_turns_rec(mtx, 1 - color, 0, alpha);
        }
        double best_score = -1; // Инициализация лучшего счёта (отрицательное значение)
        // Перебор всех возможных ходов
        for (auto turn : now_turns) {
            size_t new_state = next_move.size(); // Получаем индекс следующего состояния
            double score;
            // Если есть возможность побитий, рекурсивно ищем следующий лучший ход
            if (now_have_beats) {
                score = find_first_best_turn(make_turn(mtx, turn), color, turn.x2, turn.y2, new_state, best_score);
            }
            else {
                // Если побитий нет, ищем обычный ход
                score = find_best_turns_rec(make_turn(mtx, turn), 1 - color, 0, best_score);
            }
            // Обновляем лучший счёт и состояние, если текущий ход лучше
            if (score > best_score) {
                best_score = score; // Обновляем лучший счёт
                next_move[state] = turn; // Сохраняем текущий ход как лучший для состояния
                // Определяем следующее состояние в зависимости от наличия побитий
                next_best_state[state] = (now_have_beats ? new_state : -1);
            }
        }
        return best_score; // Возвращаем лучший счёт
    }
        
    // Рекурсивный метод для поиска лучших ходов с учётом текущего состояния и глубины поиска
    double find_best_turns_rec(vector<vector<POS_T>> mtx, const bool color, const size_t depth, double alpha = -1,
        double beta = INF + 1, const POS_T x = -1, const POS_T y = -1) {
        // Если достигнута максимальная глубина, возвращаем оценку текущей позиции
        if (depth == Max_depth) {
            return calc_score(mtx, (depth % 1 == color)); // Проверяем, чей ход, и оцениваем позицию
        }
        // Если переданы координаты фигуры, ищем её возможные ходы
        if (x != -1) {
            find_turns(color, mtx); // Получаем возможные ходы для фигур заданного цвета
        }
        // Сохраняем текущее состояние доступных ходов и флаг наличия побитий
        auto now_turns = turns;
        auto now_have_beats = have_beats;
        // Если не найдено побитий и переданы координаты, продолжаем для противника
        if (!now_have_beats && x != -1) {
            return find_best_turns_rec(mtx, 1 - color, +1, alpha, beta);
        }

        // Если нет доступных ходов, возвращаем 0 (для текущего цвета) или бесконечность (для противника)
        if (turns.empty()) {
            return (depth % 2 ? 0 : INF);
        }
        
        double min_score = INF + 1; // Инициализация минимального счёта
        double max_score = -1; // Инициализация максимального счёта
        // Перебор всех возможных ходов
        for (auto turn : now_turns) {
            double score;
            // Если есть возможность побития, продолжаем с текущим цветом
            if (now_have_beats) {
                score = find_best_turns_rec(make_turn(mtx, turn), color, depth, alpha, beta, turn.x2, turn.y2);
            }
            else {
                // Если побитий нет, переключаемся на противника и увеличиваем глубину
                score = find_best_turns_rec(make_turn(mtx, turn), 1 - color, depth + 1, alpha, beta);
            }
            // Обновляем минимальный и максимальный счёт
            min_score = min(min_score, score);
            max_score = max(max_score, score);
            // alpha-beta отсечение
            if (depth % 2) {
                alpha = max(alpha, max_score); // Если текущий игрок максимизирует счёт
            }
            else {
                beta = min(beta, min_score); // Если текущий игрок минимизирует счёт
            }
            // Если alpha больше beta, прерываем цикл (оптимизация)
            if (optimization != "O0" && alpha > beta) {
                break;
            }
            // Специальная проверка для оптимизации
            if (optimization == "O2" && alpha == beta) {
                return (depth % 2 ? max_score + 1 : min_score - 1);
            }
        }
        // Возвращаем лучший найденный счёт в зависимости от текущего игрока
        return (depth % 2 ? max_score : min_score);
    }

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
