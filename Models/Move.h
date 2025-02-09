#pragma once
#include <stdlib.h>

typedef int8_t POS_T; // Определяем тип POS_T как 8-битное целое

struct move_pos
{
    POS_T x, y;             // Исходные координаты (откуда)
    POS_T x2, y2;           // Конечные координаты (куда)
    POS_T xb = -1, yb = -1; // Координаты побитой фигуры, по умолчанию равные -1 (означают, что никто не побит)

    // Конструктор для инициализации переменной перемещения из одной позиции в другую
    move_pos(const POS_T x, const POS_T y, const POS_T x2, const POS_T y2) : x(x), y(y), x2(x2), y2(y2)
    {
    }
    // Конструктор для инициализации переменной перемещения с указанием побитой фигуры
    move_pos(const POS_T x, const POS_T y, const POS_T x2, const POS_T y2, const POS_T xb, const POS_T yb)
        : x(x), y(y), x2(x2), y2(y2), xb(xb), yb(yb)
    {
    }

    // Оператор сравнения для проверки равенства двух перемещений
    bool operator==(const move_pos &other) const
    {
        return (x == other.x && y == other.y && x2 == other.x2 && y2 == other.y2);
    }
    // Оператор неравенства. Если два перемещения не равны, возвращает true
    bool operator!=(const move_pos &other) const
    {
        return !(*this == other);
    }
};
