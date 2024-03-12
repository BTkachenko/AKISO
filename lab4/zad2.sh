#!/bin/bash

# Inicjalizacja tablicy dla historii prędkości sieci
network_history=()
network_tx_history=()

# Funkcja do konwersji bajtów na odpowiedni format
function convert_bytes {
    if [[ $1 -lt 1024 ]]; then
        echo "${1}B"
    elif [[ $1 -lt 1048576 ]]; then
        echo "$(bc <<< "scale=2; $1/1024")KB"
    else
        echo "$(bc <<< "scale=2; $1/1048576")MB"
    fi
}

# Funkcja do rysowania wykresu słupkowego
function draw_bar {
    local value=$1
    local max_value=$((15 * 1048576)) # 15 MB jako maksymalna wartość
    local bar_width=30
    local scaled_value=$((value * bar_width / max_value))
    scaled_value=$((scaled_value > bar_width ? bar_width : scaled_value))

    # Rysowanie słupka
    printf "%-6s [" "$(convert_bytes $value)"
    for ((i=0; i<scaled_value; i++)); do
        echo -n '='
    done
    for ((i=scaled_value; i<bar_width; i++)); do
        echo -n ' '
    done
    echo "]"
}

# Funkcja do obliczania średniej prędkości sieciowej
function calculate_average {
    local -n history=$1
    local total=0
    local count=${#history[@]}
    for rate in "${history[@]}"; do
        total=$((total + rate))
    done
    if (( count > 0 )); then
        echo $((total / count))
    else
        echo 0
    fi
}

# Funkcja do pobierania i wyświetlania informacji o CPU
function get_cpu_info {
    local cpu_id=0
    while read -r line; do
        if [[ $line =~ ^cpu[0-9]+ ]]; then
            local cpu_usage=$(awk '{usage=($2+$4)*100/($2+$4+$5)} END {print usage}' <<< "$line")
            local cpu_freq=$(cat /sys/devices/system/cpu/cpu$cpu_id/cpufreq/scaling_cur_freq)
            cpu_freq=$(($cpu_freq / 1000)) # MHz
            echo "CPU$cpu_id: Użycie: $cpu_usage%, Częstotliwość: ${cpu_freq}MHz"
            ((cpu_id++))
        fi
    done < /proc/stat
}

# Funkcja do pobierania i wyświetlania informacji o pamięci
function get_memory_info {
    while read -r line; do
        if [[ $line =~ ^MemTotal:|^MemFree:|^MemAvailable: ]]; then
            echo "$line"
        fi
    done < /proc/meminfo
}

# Funkcja do pobierania i wyświetlania czasu działania systemu
function get_uptime {
    read -r uptime < /proc/uptime
    local total_seconds=${uptime%%.*}
    local sec=$((total_seconds % 60))
    local min=$((total_seconds / 60 % 60))
    local hour=$((total_seconds / 3600 % 24))
    local day=$((total_seconds / 86400))
    echo "Czas działania systemu: $day dni, $hour godzin, $min minut, $sec sekund"
}

# Funkcja do pobierania i wyświetlania obciążenia systemu
function get_load_average {
    read -r load1 load5 load15 nr_tasks last_pid < /proc/loadavg
    echo "Obciążenie systemu (średnia z ostatnich 1/5/15 minut): $load1, $load5, $load15"
    echo "Liczba aktywnych/zawieszonych procesów: ${nr_tasks%%/*}"
    echo "Łączna liczba procesów: ${nr_tasks##*/}"
    echo "Ostatnie ID procesu: $last_pid"
}

# Funkcja do pobierania i wyświetlania stanu baterii
function get_battery_status {
    if [ -f /sys/class/power_supply/BATT/capacity ]; then
        read -r battery_capacity < /sys/class/power_supply/BATT/capacity
        echo "Stan baterii: $battery_capacity%"
    else
        echo "Informacja o baterii nie jest dostępna"
    fi
}

# Główna pętla skryptu
while true; do
    # Dane sieciowe
    network_interface=$(ls /sys/class/net | grep -E '^(en|wl)' | head -n 1)
    if [[ -n "$network_interface" ]]; then
        rx_prev=$(cat /sys/class/net/"$network_interface"/statistics/rx_bytes)
        tx_prev=$(cat /sys/class/net/"$network_interface"/statistics/tx_bytes)
        sleep 1
        rx_next=$(cat /sys/class/net/"$network_interface"/statistics/rx_bytes)
        tx_next=$(cat /sys/class/net/"$network_interface"/statistics/tx_bytes)

        rx_rate=$(($rx_next - $rx_prev))
        tx_rate=$(($tx_next - $tx_prev))

        # Aktualizacja historii sieciowej
        network_history=($rx_rate "${network_history[@]}")
        network_tx_history=($tx_rate "${network_tx_history[@]}")
        if [ ${#network_history[@]} -gt 10 ]; then
            network_history=("${network_history[@]:0:10}")
            network_tx_history=("${network_tx_history[@]:0:10}")
        fi

        rx_average=$(calculate_average network_history)
        tx_average=$(calculate_average network_tx_history)
    fi

    clear

    if [[ -n "$network_interface" ]]; then
        echo "Aktualna prędkość sieci - Odbiór: $(convert_bytes $rx_rate)/s, Wysyłanie: $(convert_bytes $tx_rate)/s"
        echo "Średnia prędkość sieci - Odbiór: $(convert_bytes $rx_average)/s, Wysyłanie: $(convert_bytes $tx_average)/s"
        echo "Historia (Odbiór):"
        for rate in "${network_history[@]}"; do
            draw_bar $rate
            echo
        done
    else
        echo "Interfejs sieciowy nie został znaleziony."
    fi

    # Wyświetlanie informacji o CPU
    get_cpu_info

    # Wyświetlanie informacji o pamięci
    get_memory_info

    # Wyświetlanie czasu działania systemu
    get_uptime

    # Wyświetlanie obciążenia systemu
    get_load_average

    # Wyświetlanie stanu baterii
    get_battery_status

    sleep 1
done
