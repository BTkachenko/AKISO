#!/bin/bash

# Wypisanie nagłówka
echo -e "PPID\tPID\tCOMM\t\tSTATE\tTTY\tRSS\tPGID\tSID\tOPEN FILES"

# Iteracja przez katalogi /proc/<PID>
for pid in /proc/[0-9]*/; do
    # Sprawdzenie, czy katalog procesu istnieje
    [ -d "$pid" ] || continue

    # Czytanie wymaganych informacji
    ppid=$(awk '/PPid/{print $2}' "$pid/status" 2>/dev/null)
    comm=$(awk '/Name/{print $2}' "$pid/status" 2>/dev/null)
    state=$(awk '{print $3}' "$pid/stat" 2>/dev/null)
    tty_nr=$(awk '{print $7}' "$pid/stat" 2>/dev/null)
    tty=$(ls -l /proc/$tty_nr/exe 2>/dev/null | awk '{print $11}')
    rss=$(awk '/VmRSS/{print $2}' "$pid/status" 2>/dev/null)
    pgid=$(awk '{print $5}' "$pid/stat" 2>/dev/null)
    sid=$(awk '{print $6}' "$pid/stat" 2>/dev/null)
    open_files=$(ls "$pid/fd" 2>/dev/null | wc -l)

    # Wypisanie informacji
    echo -e "$ppid\t$pid\t$comm\t\t$state\t$tty\t$rss\t$pgid\t$sid\t$open_files"
done | column -t
