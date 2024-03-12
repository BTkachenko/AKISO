#!/bin/bash

# Pobierz obraz z The Cat API
cat_url=$(curl -s "https://api.thecatapi.com/v1/images/search" | jq -r '.[0].url')
curl -s "$cat_url" > cat_image

# Wyświetl obraz za pomocą catimg lub img2txt


    img2txt cat_image


echo "" # Pusta linia dla oddzielenia obrazu od cytatu

# Pobierz i wyświetl losowy cytat Chucka Norrisa
chuck_quote=$(curl -s "https://api.chucknorris.io/jokes/random" | jq -r '.value')
echo "Cytat Chucka Norrisa: $chuck_quote"

# Usuń obraz
#rm cat_image
