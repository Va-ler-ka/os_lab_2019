#!/bin/bash

count=$#

if [ "$count" -eq 0 ]; then
    echo "Нет аргументов"
    exit 1
fi

average=$(printf '%s\n' "$@" | awk '{sum+=$1} END {print sum/NR}')

echo "Количество аргументов: $count"
echo "Среднее арифметическое: $average"
