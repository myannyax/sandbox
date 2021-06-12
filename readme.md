# Sandbox

## Features:

* Ограничение времени работы и потребления памяти. [**TODO**]
* Ограничение порождения процессов (защита от «fork-бобм») [**TODO**]
* Настройка ограничения доступа к файлам и каталогам (read и write, open) [**TODO**] [In progress]
* Ограничение рассылки сигналов (запущенное приложение не должно иметь возможности 
  «убить» песочницу или какой-либо другой процесс. Кроме, быть может, процессов, порожденных самим приложением) [**TODO**]
* Запрет запуска приложений, способных повысить свои права (suid-бит, linux capabilities) [**TODO**] [User namespace?]
* Настройка приоритета выполнения запущенной программы (niceness) [**TODO**]
* Возможность приостановить (продолжить), а также досрочно завершить выполнение [**TODO**]
* Составление отчета о: [**TODO**]
    * запрошенных неразрешенных операциях
    * причинах завершения программы песочницей
    * причинах невозможности запустить программу, если таковые имеются
* Завершение песочницы (аварийное или штатное) должно приводить к завершению процесса и всех его дочерних процессов. [**TODO**]