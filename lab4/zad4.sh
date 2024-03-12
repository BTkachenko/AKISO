#!/bin/bash

# Sprawdź, czy ścieżka katalogu została podana
if [ -z "$1" ]; then
    echo "Użycie: $0 [ścieżka_do_katalogu]"
    exit 1
fi

# Przejście do podanego katalogu
cd "$1"

# Tworzenie tymczasowego pliku do przechowywania sum kontrolnych i ścieżek
temp_file=$(mktemp)

# Obliczanie sum kontrolnych dla wszystkich plików
# i zapisywanie ich wraz ze ścieżką do tymczasowego pliku
find . -type f -exec sha256sum {} + > "$temp_file"

# Sortowanie i wyświetlanie duplikatów
awk '{print $1}' "$temp_file" | sort | uniq -d | while read -r checksum; do
    grep "$checksum" "$temp_file" | awk '{print $2}' | while read -r file; do
        # Wyświetlanie ścieżki i rozmiaru pliku
        echo "$(du -h "$file" | cut -f1) $file"
    done
done | sort -rh

# Usuwanie tymczasowego pliku
rm "$temp_file"
