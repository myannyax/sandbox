# Sandbox

## Features:

* Ограничение времени работы и потребления памяти.
* Ограничение порождения процессов (защита от «fork-бобм»)
* Настройка ограничения доступа к файлам и каталогам (read и write, open)
* Ограничение рассылки сигналов (запущенное приложение не должно иметь возможности 
  «убить» песочницу или какой-либо другой процесс. Кроме, быть может, процессов, порожденных самим приложением)
* Запрет запуска приложений, способных повысить свои права (suid-бит, linux capabilities) [user namespace?]
* Настройка приоритета выполнения запущенной программы (niceness)
* Возможность приостановить (продолжить), а также досрочно завершить выполнение
* Составление отчета о:
    * запрошенных неразрешенных операциях
    * причинах завершения программы песочницей
    * причинах невозможности запустить программу, если таковые имеются
* Завершение песочницы (аварийное или штатное) должно приводить к завершению процесса и всех его дочерних процессов.

### Флаги конфигурации:
| Name        | Meaning           | Default  |
| ------------- |:-------------:| :-----:|
| fork_limit     | ограничение количества форков | inf |
| fs: path + action (deny/allow/readonly) | ограничения для файлов по конкретному пути  | N/A |
| max_stack | максимальный размер стека |    inf |
| max_data | максимальный сегмент данных  | inf |
| max_memory | ограничение адресного пространства  | inf |
| max_cpu_time | ограничение времени на CPU | inf |
| max_time | ограничение времени исполнения | inf |
| mount_root | монтировать корень как overlay систему или нет | false |
| niceness | приоритет процесса | N/A |
| user_namespace | создавать user namespace | false |
| mount_namespace | создавать mount namespace (нужен user namespace) | false |
| pid_namespace | создавать pid namespace (нужен mount namespace) | false |
