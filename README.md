# Checkers  
Desktop application for playing checkers with a bot / friend in C++.  
Using the SDL2 framework for rendering.  
Supports the game bot vs bot with the setting of the depth of calculation for each separately (from settings.json).  
## For developers:  
To work install SDL2 and SDL2_image(Board.h, Hand.h), nlohmann/json(Config.h) and correct path strings in Board.h and Config.h.
The calculation is made for the number of steps equal to depth + 1, where, for example, steps with multiple takes are counted as 1 step.  
State traversal uses a minimax algorithm with alpha-beta pruning heuristics.  
To calculate values in leaf states, the Logic::calc_score function is used.  

��������� � settings.json:  
 ### ��������� ������� ���� �������� ����������
 "Width": 0,  ������ ���� �������� ���������� (0 �������������)
 "Hight": 0,  ������ ���� �������� ���������� (0 �������������)
  
 ### ��������� ����
  "IsWhiteBot": false, ������� �� ����� ��� (false � ������ �� �������)
  "IsBlackBot": true, ������� �� ������ ��� (true � ������ �������)
  "WhiteBotLevel": 0, ������� ��������� ������ ���� (0 � ������ ��������� ��� ������� ����)
  "BlackBotLevel": 5, ������� ��������� ������� ���� (5 � ������� ������� ���������)
  "BotScoringType": "NumberAndPotential", ����� ������ ��������� (��������� ��� � �������� ���������)
  "BotDelayMS": 0, �������� � ������������� ����� ����� ���� (0 � ������ ���������� ���)
  "NoRandom": false, ����������� �� ������� ����������� � ���� ����� (false � ������ ����������� ��������)
  "Optimization": "O1", ��� ����������� ���������� �����, ��������� ������ ����� ��������� ���� (O1 � ����������� ������� ������)
  
 ### ����
  "MaxNumTurns": 120, ������������ ���������� ����� � ���� (120 ����� � ������ ��� ���������� ����) 
