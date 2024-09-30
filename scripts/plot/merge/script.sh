#!/bin/bash

input_file="cycle_counts2.dat"
output_file="cycle_counts2_processed.dat"

> "$output_file"  # Empty the output file

while read x y; do
    adjusted_y=$(echo "$y / 8" | bc -l)
    for ((i=0; i<8; i++)); do
        pixel_number=$(( x * 8 + i ))
        echo "$pixel_number $adjusted_y" >> "$output_file"
    done
done < "$input_file"
