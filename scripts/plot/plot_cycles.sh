#!/bin/bash

# Base filename without extension
base_filename="plot"

# File extension
extension=".png"

# Initialize counter
counter=1

# Desired output filename
output_filename="../../resources/plots/${base_filename}${extension}"

# Loop to find a unique filename
while [ -f "$output_filename" ]; do
    output_filename="../../resources/plots/${base_filename}${counter}${extension}"
    counter=$((counter + 1))
done

# Inform the user about the chosen filename
echo "Generating plot: $output_filename"

# Run Gnuplot with the unique output filename
gnuplot -e "output_file='${output_filename}'" plot_cycles.gp

# Confirm completion
echo "Plot saved as $output_filename"
