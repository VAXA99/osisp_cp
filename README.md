# Инструкция

Привет!

Варфоломеев при проверке у меня Курсовой работы не требовал запускать программу, однако на всякий случай тебе
следует наладить работу программы. 

На случай если захочешь изменить цвет графиков, диаграмм и прочее, то все файлы, включая Excel и drawio, прикладываю вместе с кодом.
Их и сам отчёт ты можешь найти в `/docs`.

## Настройка проекта
1. Установи компилятор для С++: https://sourceforge.net/projects/mingw/.
1. Установи Git: https://git-scm.com/download/win.
1. Выбери директорию, где сохранишь проект.
1. Открой терминал на `Shift + ё`.
1. `git clone https://github.com/VAXA99/osisp_cp.git`

## Порядок работы с кодом
1. Найди съемный накопитель.
1. Создай там директорию `/files`, внутри неё `/in` и `/out`.
1. Скопируй абсолютный путь до `/files/in` и `/files/out`.
1. В файлах исходного кода следует поменять путь до съемного накопителя. Они помечены комменатриями `// In` и `// Out`.
В `inputDirectory` следует записать путь `/files/in`, `outputDirectory` - /files/out`.
1. Открой терминал на `Shift + Ё`.
1. Перекомпилируй каждый файл (a, b, c). Пример команды: `g++ -o a_krivonogov a_krivonogov.cpp util.cpp`.

## Использование
1. Открой терминал на `Shift + ё`.
1. Чтобы запустить исполнение программы следует ввести такую команду `./[тип_программы]_[фамилия] [тип_пути] [имена_файлов]`.
1. Тип пути: `--harddisk` (чтение с жесткого диска), `--removable` (съемный накопитель).
1. Пример команды: `./a_krivonogov --harddisk 1.txt`.
