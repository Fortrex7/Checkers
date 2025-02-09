#pragma once
#include <tuple>

#include "../Models/Move.h"
#include "../Models/Response.h"
#include "Board.h"

// Класс Hand отвечает за управление вводом от игрока
class Hand
{
  public:
      // Конструктор класса, принимающий указатель на объект Board
    Hand(Board *board) : board(board)
    {
    }
    // Метод для обработки событий мыши и получения выбранной ячейки
    tuple<Response, POS_T, POS_T> get_cell() const
    {
        SDL_Event windowEvent; // Переменная для хранения событий окна
        Response resp = Response::OK;
        int x = -1, y = -1; // Переменные для хранения координат щелчка мыши
        int xc = -1, yc = -1; // Переменные для хранения координат клеток
        
        // Бесконечный цикл для обработки событий
        while (true)
        {
            // Проверка на наличие событий в очереди
            if (SDL_PollEvent(&windowEvent))
            {
                switch (windowEvent.type)
                {
                case SDL_QUIT: // Если произошел клик по кнопке закрытия
                    resp = Response::QUIT;
                    break;
                case SDL_MOUSEBUTTONDOWN: // Если нажата кнопка мыши
                    x = windowEvent.motion.x;
                    y = windowEvent.motion.y;
                    // Преобразование координат окна в координаты ячеек на доске
                    xc = int(y / (board->H / 10) - 1);
                    yc = int(x / (board->W / 10) - 1);
                    // Обработка специальных команд
                    if (xc == -1 && yc == -1 && board->history_mtx.size() > 1)
                    {
                        resp = Response::BACK; // Команда на отмену хода
                    }
                    else if (xc == -1 && yc == 8)
                    {
                        resp = Response::REPLAY; // Команда на переигровку
                    }
                    else if (xc >= 0 && xc < 8 && yc >= 0 && yc < 8)
                    {
                        resp = Response::CELL; // Корректное выделение клетки
                    }
                    else
                    {
                        // Сброс координат при некорректном выборе
                        xc = -1;
                        yc = -1;
                    }
                    break;
                case SDL_WINDOWEVENT: // Изменение состояния окна
                    if (windowEvent.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
                    {
                        board->reset_window_size(); // Сброс размеров доски при изменении окна
                        break;
                    }
                }
                if (resp != Response::OK) // Если статус не OK, завершить цикл
                    break;
            }
        }
        return {resp, xc, yc}; // Возвращаем кортеж с статусом и координатами выбранной ячейки
    }

    // Метод, ожидающий ввода от игрока и возвращающий статус
    Response wait() const
    {
        SDL_Event windowEvent;
        Response resp = Response::OK;
        // Бесконечный цикл, ожидающий событий
        while (true)
        {
            if (SDL_PollEvent(&windowEvent))
            {
                switch (windowEvent.type)
                {
                case SDL_QUIT:
                    resp = Response::QUIT;
                    break;
                // Сброс размеров доски при изменении размеров окна
                case SDL_WINDOWEVENT_SIZE_CHANGED:
                    board->reset_window_size();
                    break;
                case SDL_MOUSEBUTTONDOWN: {
                    // Получение координат X и Y при нажатии
                    int x = windowEvent.motion.x;
                    int y = windowEvent.motion.y;
                    // Вычисление координат клетки по X и Y
                    int xc = int(y / (board->H / 10) - 1);
                    int yc = int(x / (board->W / 10) - 1);
                    // Проверка, если игрок нажал на кнопку переигровки
                    if (xc == -1 && yc == 8)
                        resp = Response::REPLAY;
                }
                break;
                }
                if (resp != Response::OK) // Выход из цикла при ответе отличном от успешного
                    break;
            }
        }
        return resp; //Возврат статуса
    }

  private:
    Board *board; // Указатель на объект класса Board, отвечающий за отображение игровой доски
};
