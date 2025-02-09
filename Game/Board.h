#pragma once
#include <iostream>
#include <fstream>
#include <vector>

#include "../Models/Move.h"
#include "../Models/Project_path.h"

#ifdef __APPLE__
    #include <SDL2/SDL.h>
    #include <SDL2/SDL_image.h>
#else
    #include <SDL.h>
    #include <SDL_image.h>
#endif

using namespace std;

// Класс Board представляет игровую доску и реализует логику игры в шашки
class Board
{
public:
    Board() = default;
    // Конструктор с параметрами ширины и высоты доски
    Board(const unsigned int W, const unsigned int H) : W(W), H(H)
    {
    }

    // Метод для инициализации и отрисовки стартовой доски
    int start_draw()
    {
        // Инициализация SDL
        if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
        {
            print_exception("SDL_Init can't init SDL2 lib");
            return 1; // Возврат ошибки при неудачной инициализации
        }
        // Установка размеров окна в случае их равенства нулю
        if (W == 0 || H == 0)
        {
            SDL_DisplayMode dm;
            if (SDL_GetDesktopDisplayMode(0, &dm))
            {
                print_exception("SDL_GetDesktopDisplayMode can't get desctop display mode");
                return 1;
            }
            W = min(dm.w, dm.h);
            W -= W / 15; // Установка ширины
            H = W; // Установка высоты равной ширине
        }
        // Создание окна и рендерера
        win = SDL_CreateWindow("Checkers", 0, H / 30, W, H, SDL_WINDOW_RESIZABLE);
        if (win == nullptr)
        {
            print_exception("SDL_CreateWindow can't create window");
            return 1; // Ошибка при создании окна
        }
        ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
        if (ren == nullptr)
        {
            print_exception("SDL_CreateRenderer can't create renderer");
            return 1; // Ошибка при создании рендерера
        }
        // Загрузка текстур
        board = IMG_LoadTexture(ren, board_path.c_str());
        w_piece = IMG_LoadTexture(ren, piece_white_path.c_str());
        b_piece = IMG_LoadTexture(ren, piece_black_path.c_str());
        w_queen = IMG_LoadTexture(ren, queen_white_path.c_str());
        b_queen = IMG_LoadTexture(ren, queen_black_path.c_str());
        back = IMG_LoadTexture(ren, back_path.c_str());
        replay = IMG_LoadTexture(ren, replay_path.c_str());
        // Проверка успешности загрузки текстур
        if (!board || !w_piece || !b_piece || !w_queen || !b_queen || !back || !replay)
        {
            print_exception("IMG_LoadTexture can't load main textures from " + textures_path);
            return 1;
        }
        SDL_GetRendererOutputSize(ren, &W, &H); // Получение размеров рендера
        make_start_mtx(); // Создание стартовой матрицы
        rerender(); // Перерисовка доски
        return 0; // Успешное завершение
    }

    // Метод для перерисовки доски и сброса состояния игры
    void redraw()
    {
        game_results = -1; // Сброс результата игры
        history_mtx.clear(); // Очистка истории ходов
        history_beat_series.clear(); // Очистка истории побитых фигур
        make_start_mtx(); // Создание стартовой матрицы
        clear_active(); // Сброс активной клетки
        clear_highlight(); // Сброс выделения клеток
    }

    // Метод для перемещения фигуры с учётом побитой
    void move_piece(move_pos turn, const int beat_series = 0)
    {
        if (turn.xb != -1) // Если фигура была побита
        {
            mtx[turn.xb][turn.yb] = 0; // Обнуление позиции побитой фигуры
        }
        // Перемещение фигуры
        move_piece(turn.x, turn.y, turn.x2, turn.y2, beat_series);
    }

    // Метод для перемещения фигуры по клеткам
    void move_piece(const POS_T i, const POS_T j, const POS_T i2, const POS_T j2, const int beat_series = 0)
    {
        if (mtx[i2][j2]) // Проверка, занята ли конечная позиция
        {
            throw runtime_error("final position is not empty, can't move");
        }
        if (!mtx[i][j]) // Проверка, занята ли начальная позиция
        {
            throw runtime_error("begin position is empty, can't move");
        }
        if ((mtx[i][j] == 1 && i2 == 0) || (mtx[i][j] == 2 && i2 == 7))
            mtx[i][j] += 2; // Превращение в дамку
        mtx[i2][j2] = mtx[i][j]; // Перемещение фигуры
        drop_piece(i, j); // Очистка старой позиции
        add_history(beat_series); // Добавление в историю
    }

    // Удаление фигуры с доски
    void drop_piece(const POS_T i, const POS_T j)
    {
        mtx[i][j] = 0; // Обнуление клетки
        rerender();
    }

    // Превращение фигуры в дамку
    void turn_into_queen(const POS_T i, const POS_T j)
    {
        if (mtx[i][j] == 0 || mtx[i][j] > 2)
        {
            throw runtime_error("can't turn into queen in this position");
        }
        mtx[i][j] += 2; // Увеличение значения для обозначения дамки
        rerender();
    }
    // Получение текущей матрицы доски
    vector<vector<POS_T>> get_board() const
    {
        return mtx;
    }

    // Выделение клеток для возможных ходов
    void highlight_cells(vector<pair<POS_T, POS_T>> cells)
    {
        for (auto pos : cells)
        {
            POS_T x = pos.first, y = pos.second;
            is_highlighted_[x][y] = 1; // Установка выделения
        }
        rerender();
    }

    // Сброс выделения всех клеток
    void clear_highlight()
    {
        for (POS_T i = 0; i < 8; ++i)
        {
            is_highlighted_[i].assign(8, 0); // Обнуление выделения
        }
        rerender();
    }

    // Установка активной клетки
    void set_active(const POS_T x, const POS_T y)
    {
        // Установка активного хода
        active_x = x;
        active_y = y;
        rerender();
    }

    // Сброс активной клетки
    void clear_active()
    {
        // Обнуление активной клетки
        active_x = -1;
        active_y = -1;
        rerender();
    }

    // Проверка, выделена ли клетка
    bool is_highlighted(const POS_T x, const POS_T y)
    {
        return is_highlighted_[x][y]; // Возврат состояния выделенной клетки
    }

    // Откат хода
    void rollback()
    {
        auto beat_series = max(1, *(history_beat_series.rbegin())); // Получение количества побитых
        while (beat_series-- && history_mtx.size() > 1) // Откат по истории
        {
            history_mtx.pop_back();
            history_beat_series.pop_back();
        }
        mtx = *(history_mtx.rbegin()); // Восстановление состояния доски
        clear_highlight(); // Сброс выделений
        clear_active(); // Сброс активной клетки
    }

    // Показ результата игры
    void show_final(const int res)
    {
        game_results = res; // Установка результата
        rerender(); // Перерисовка для отображения результата
    }

    // Перерисовка окна при изменении размера
    void reset_window_size()
    {
        SDL_GetRendererOutputSize(ren, &W, &H); // Получение новых размеров рендера
        rerender();
    }

    // Корректное завершение работы игры и освобождение ресурсов
    void quit()
    {
        // Освобождение ресурсов текстур
        SDL_DestroyTexture(board);
        SDL_DestroyTexture(w_piece);
        SDL_DestroyTexture(b_piece);
        SDL_DestroyTexture(w_queen);
        SDL_DestroyTexture(b_queen);
        SDL_DestroyTexture(back);
        SDL_DestroyTexture(replay);
        SDL_DestroyRenderer(ren); // Освобождение рендерера
        SDL_DestroyWindow(win); // Освобождение окна
        SDL_Quit(); // Завершение работы SDL
    }

    // Деструктор для освобождения ресурсов
    ~Board()
    {
        if (win)
            quit(); // Вызов освобождения ресурсов
    }

private:
    // Добавление состояния в историю
    void add_history(const int beat_series = 0)
    {
        history_mtx.push_back(mtx); // Сохранение текущего состояния
        history_beat_series.push_back(beat_series); // Сохранение количества побитых
    }
    // Создание стартовой матрицы доски
    void make_start_mtx()
    {
        for (POS_T i = 0; i < 8; ++i)
        {
            for (POS_T j = 0; j < 8; ++j)
            {
                mtx[i][j] = 0; // Обнуление клеток
                // Установка фигур для начальной расстановки
                if (i < 3 && (i + j) % 2 == 1)
                    mtx[i][j] = 2; // Черные фигуры
                if (i > 4 && (i + j) % 2 == 1)
                    mtx[i][j] = 1; // Белые фигуры
            }
        }
        add_history(); // Сохранение начального состояния в историю
    }

    // Перерисовка всех текстур на доске
    void rerender()
    {
        SDL_RenderClear(ren); // Очистка рендерера
        SDL_RenderCopy(ren, board, NULL, NULL); // Отрисовка доски

        // Отрисовка фигур
        for (POS_T i = 0; i < 8; ++i)
        {
            for (POS_T j = 0; j < 8; ++j)
            {
                if (!mtx[i][j])
                    continue; // Пропуск пустой клетки
                // Вычисление позиций X и Y
                int wpos = W * (j + 1) / 10 + W / 120;
                int hpos = H * (i + 1) / 10 + H / 120;
                SDL_Rect rect{ wpos, hpos, W / 12, H / 12 }; // Рамка фигуры

                // Выбор текстуры фигуры в зависимости от значения в матрице
                SDL_Texture* piece_texture;
                if (mtx[i][j] == 1)
                    piece_texture = w_piece;
                else if (mtx[i][j] == 2)
                    piece_texture = b_piece;
                else if (mtx[i][j] == 3)
                    piece_texture = w_queen;
                else
                    piece_texture = b_queen;

                SDL_RenderCopy(ren, piece_texture, NULL, &rect); // Отрисовка фигуры
            }
        }

        // Отрисовка выделенных клеток
        SDL_SetRenderDrawColor(ren, 0, 255, 0, 0); // Цвет выделения
        const double scale = 2.5;
        SDL_RenderSetScale(ren, scale, scale); // Масштабирование
        for (POS_T i = 0; i < 8; ++i)
        {
            for (POS_T j = 0; j < 8; ++j)
            {
                if (!is_highlighted_[i][j]) // Пропуск непримеченных клеток
                    continue;
                SDL_Rect cell{ int(W * (j + 1) / 10 / scale), int(H * (i + 1) / 10 / scale), int(W / 10 / scale),
                              int(H / 10 / scale) };
                SDL_RenderDrawRect(ren, &cell); // Отрисовка рамки выделенной клетки
            }
        }

        // Отрисовка активной клетки
        if (active_x != -1)
        {
            SDL_SetRenderDrawColor(ren, 255, 0, 0, 0); // Цвет активности
            SDL_Rect active_cell{ int(W * (active_y + 1) / 10 / scale), int(H * (active_x + 1) / 10 / scale),
                                 int(W / 10 / scale), int(H / 10 / scale) };
            SDL_RenderDrawRect(ren, &active_cell); // Отрисовка рамки для активной клетки
        }
        SDL_RenderSetScale(ren, 1, 1); // Возврат масштаба к 1

        // Отрисовка стрелок
        SDL_Rect rect_left{ W / 40, H / 40, W / 15, H / 15 }; // Позиция кнопки назад
        SDL_RenderCopy(ren, back, NULL, &rect_left); // Отрисовка кнопки назад
        SDL_Rect replay_rect{ W * 109 / 120, H / 40, W / 15, H / 15 }; // Позиция кнопки переигровки
        SDL_RenderCopy(ren, replay, NULL, &replay_rect); // Отрисовка кнопки переигровки

        // Отрисовка результата игры
        if (game_results != -1)
        {
            string result_path = draw_path; // Путь к изображению результата
            if (game_results == 1)
                result_path = white_path; // Путь если выиграли белые
            else if (game_results == 2)
                result_path = black_path; // Путь если выиграли черные
            // Загрузка текстуры результата
            SDL_Texture* result_texture = IMG_LoadTexture(ren, result_path.c_str());
            if (result_texture == nullptr)
            {
                print_exception("IMG_LoadTexture can't load game result picture from " + result_path);
                return;
            }
            SDL_Rect res_rect{ W / 5, H * 3 / 10, W * 3 / 5, H * 2 / 5 }; // Рамка для результата
            SDL_RenderCopy(ren, result_texture, NULL, &res_rect); // Отрисовка результата
            SDL_DestroyTexture(result_texture); // Освобождение текстуры результата
        }

        SDL_RenderPresent(ren); // Завершение отрисовки
        // Следующий цикл для macOS
        SDL_Delay(10); // Задержка
        SDL_Event windowEvent;
        SDL_PollEvent(&windowEvent); // Обработка событий окна
    }

    // Запись исключений в лог
    void print_exception(const string& text) {
        ofstream fout(project_path + "log.txt", ios_base::app);
        fout << "Error: " << text << ". "<< SDL_GetError() << endl; // Запись ошибки
        fout.close(); // Закрытие файла
    }

  public:
    // Ширина и высота доски
    int W = 0;
    int H = 0;
    // История состояний доски
    vector<vector<vector<POS_T>>> history_mtx;

  private:
    SDL_Window *win = nullptr; // Указатель на SDL окно
    SDL_Renderer *ren = nullptr; // Указатель на SDL рендерер
    // Текстуры для фигур и доски
    SDL_Texture *board = nullptr;
    SDL_Texture *w_piece = nullptr;
    SDL_Texture *b_piece = nullptr;
    SDL_Texture *w_queen = nullptr;
    SDL_Texture *b_queen = nullptr;
    SDL_Texture *back = nullptr;
    SDL_Texture *replay = nullptr;
    // Путь до файлов текстур
    const string textures_path = project_path + "Textures/";
    const string board_path = textures_path + "board.png";
    const string piece_white_path = textures_path + "piece_white.png";
    const string piece_black_path = textures_path + "piece_black.png";
    const string queen_white_path = textures_path + "queen_white.png";
    const string queen_black_path = textures_path + "queen_black.png";
    const string white_path = textures_path + "white_wins.png";
    const string black_path = textures_path + "black_wins.png";
    const string draw_path = textures_path + "draw.png";
    const string back_path = textures_path + "back.png";
    const string replay_path = textures_path + "replay.png";
    // Координаты активной клетки
    int active_x = -1, active_y = -1;
    // Результат игры
    int game_results = -1;
    // Матрица выделенных клеток для возможных ходов
    vector<vector<bool>> is_highlighted_ = vector<vector<bool>>(8, vector<bool>(8, 0));
    // Матрица текущих позиций фигур
    // 1 - белая, 2 - черная, 3 - белая дамка, 4 - черная дамка
    vector<vector<POS_T>> mtx = vector<vector<POS_T>>(8, vector<POS_T>(8, 0));
    // История побитых фигур для каждого хода
    vector<int> history_beat_series;
};
