# Задание 15 — рабочие заметки и источники

## Проверенная конфигурация Ubuntu VM
- Ядро: Linux 7.0.0-34-generic
- Доступно процессоров: 4
- Терминал: SCHED_OTHER, priority 0
- Обнаружены SCHED_FIFO, SCHED_RR, SCHED_DEADLINE
- В /proc/self/sched есть se.vruntime, переключения и миграции

## Основные тезисы

1. Чистые SJF и SRTN практически неприменимы без знания
   будущей длительности выполнения процессов.

2. Исторический Linux CFS применяет vruntime, nice и
   красно-чёрное дерево для справедливого распределения CPU.

3. Начиная с Linux 6.6 выполняется переход к EEVDF.
   EEVDF учитывает lag и виртуальные дедлайны.
   EEVDF не равен EDF реального времени.

4. Linux SCHED_FIFO и SCHED_RR используют приоритеты
   реального времени. SCHED_DEADLINE применяет EDF + CBS.

5. Windows использует приоритетное вытесняющее планирование,
   кванты и временные повышения динамических приоритетов.

6. Инверсию приоритетов решают специальными механизмами:
   в Windows — AutoBoost, в POSIX — например PTHREAD_PRIO_INHERIT.

7. Для многоядерных систем нужны балансировка нагрузки,
   учёт привязки потоков к ядрам и локальности данных.

## Официальные источники

Linux CFS:
https://docs.kernel.org/scheduler/sched-design-CFS.html

Linux EEVDF:
https://docs.kernel.org/scheduler/sched-eevdf.html

Linux SCHED_DEADLINE:
https://docs.kernel.org/scheduler/sched-deadline.html

Linux scheduling policies:
https://man7.org/linux/man-pages/man7/sched.7.html

Linux priority inheritance:
https://man7.org/linux/man-pages/man3/pthread_mutexattr_getprotocol.3p.html

Windows scheduling:
https://learn.microsoft.com/en-us/windows/win32/procthread/scheduling

Windows priority boosts:
https://learn.microsoft.com/en-us/windows/win32/procthread/priority-boosts

Windows priority inversion:
https://learn.microsoft.com/en-us/windows/win32/procthread/priority-inversion

Windows multiple processors:
https://learn.microsoft.com/en-us/windows/win32/procthread/multiple-processors
