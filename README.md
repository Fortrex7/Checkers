# Checkers  
Desktop application for playing checkers with a bot / friend in C++.  
Using the SDL2 framework for rendering.  
Supports the game bot vs bot with the setting of the depth of calculation for each separately (from settings.json).  
## For developers:  
To work install SDL2 and SDL2_image(Board.h, Hand.h), nlohmann/json(Config.h) and correct path strings in Board.h and Config.h.
The calculation is made for the number of steps equal to depth + 1, where, for example, steps with multiple takes are counted as 1 step.  
State traversal uses a minimax algorithm with alpha-beta pruning heuristics.  
To calculate values in leaf states, the Logic::calc_score function is used.  

Параметры в settings.json:  
 ### Настройка размера окна игрового интерфейса
 "Width": 0,  Ширина окна игрового интерфейса (0 полноэкранный)
 "Hight": 0,  Высота окна игрового интерфейса (0 полноэкранный)
  
 ### Настройки бота
  "IsWhiteBot": false, Активен ли белый бот (false — значит не активен)
  "IsBlackBot": true, Активен ли черный бот (true — значит активен)
  "WhiteBotLevel": 0, Уровень сложности белого бота (0 — значит неактивен или слишком слаб)
  "BlackBotLevel": 5, Уровень сложности черного бота (5 — высокий уровень сложности)
  "BotScoringType": "NumberAndPotential", Метод оценки состояния (насколько бот в выгодном положении)
  "BotDelayMS": 0, Задержка в миллисекундах перед ходом бота (0 — значит мгновенный ход)
  "NoRandom": false, Применяется ли элемент случайности в ходе ботов (false — значит случайность включена)
  "Optimization": "O1", Тип оптимизации алгоритмов ботов, насколько быстро будут выполнять ходы (O1 — оптимизация первого уровня)
  
 ### Игра
  "MaxNumTurns": 120, Максимальное количество ходов в игре (120 ходов — предел для завершения игры) 
