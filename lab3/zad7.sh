#! /bin/bash

for file in ./*
do
    if [ "$file" != "${file,,}" ]; then
        mv "$file" "${file,,}"
    fi
done    