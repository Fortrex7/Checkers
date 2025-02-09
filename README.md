# Checkers  
Desktop application for playing checkers with a bot / friend in C++.  
Using the SDL2 framework for rendering.  
Supports the game bot vs bot with the setting of the depth of calculation for each separately (from settings.json).  
## For developers:  
To work install SDL2 and SDL2_image(Board.h, Hand.h), nlohmann/json(Config.h) and correct path strings in Board.h and Config.h.
The calculation is made for the number of steps equal to depth + 1, where, for example, steps with multiple takes are counted as 1 step.  
State traversal uses a minimax algorithm with alpha-beta pruning heuristics.  
To calculate values in leaf states, the Logic::calc_score function is used.  

ѕараметры в settings.json:  
 ### Ќастройка размера окна игрового интерфейса
 "Width": 0,  Ўирина окна игрового интерфейса (0 полноэкранный)
 "Hight": 0,  ¬ысота окна игрового интерфейса (0 полноэкранный)
  
 ### Ќастройки бота
  "IsWhiteBot": false, јктивен ли белый бот (false Ч значит не активен)
  "IsBlackBot": true, јктивен ли черный бот (true Ч значит активен)
  "WhiteBotLevel": 0, ”ровень сложности белого бота (0 Ч значит неактивен или слишком слаб)
  "BlackBotLevel": 5, ”ровень сложности черного бота (5 Ч высокий уровень сложности)
  "BotScoringType": "NumberAndPotential", ћетод оценки состо€ни€ (насколько бот в выгодном положении)
  "BotDelayMS": 0, «адержка в миллисекундах перед ходом бота (0 Ч значит мгновенный ход)
  "NoRandom": false, ѕримен€етс€ ли элемент случайности в ходе ботов (false Ч значит случайность включена)
  "Optimization": "O1", “ип оптимизации алгоритмов ботов, насколько быстро будут выполн€ть ходы (O1 Ч оптимизаци€ первого уровн€)
  
 ### »гра
  "MaxNumTurns": 120, ћаксимальное количество ходов в игре (120 ходов Ч предел дл€ завершени€ игры) 
