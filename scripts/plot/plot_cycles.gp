# plot_cycles_combined.gp

# Check if the 'output_file' variable is set; if not, default to 'cycle_counts_combined.png'
if (!exists("output_file")) {
    output_file = "plot.png"
}

# Set the terminal to PNG with a specific size and enhanced text
# Increased height to 1800 to accommodate three plots comfortably
set terminal pngcairo size 1000,1800 enhanced font 'Verdana,12'

# Specify the output file
set output output_file

# Overlay the label without affecting the existing plots
# Enable multiplot layout: 3 rows, 1 column
# Removed the overall title to allocate full vertical space to individual plots
set multiplot layout 3,1 rowsfirst

###########################################
# First Plot: Full Cycle Count Range
###########################################

# Set titles and labels for the first plot
set title "Full Cycle Count Range         ".strftime("%a %b %d %H:%M:%S %Y", time(0))
set xlabel "Pixel Number"
set ylabel "Cycle Count (cycles)"

# Enable grid for better readability
set grid

# Define the style for the plot
set style line 1 lc rgb 'blue' lt 1 lw 1 pt 7 ps 0.5

# Set Y-axis range to fit up to 300,000 cycles
#set yrange [0:300000]

# Set X-axis range to cover all pixels (0 to 512)
set xrange [0:512]

# Plot the data
plot "cycle_counts.dat" using 1:2 with linespoints ls 1 title "Cycle Count"

###########################################
# Second Plot: Zoomed-In Cycle Count Range
###########################################

# Set titles and labels for the second plot
set title "Cycle Count Range Up to 1400 Cycles"
set xlabel "Pixel Number"
set ylabel "Cycle Count (cycles)"

# Enable grid
set grid

# Define the style for the plot (different color for distinction)
set style line 1 lc rgb 'red' lt 1 lw 1 pt 7 ps 0.5

# Set Y-axis range to limit cycle counts up to 1400 cycles
set yrange [500:1400]

# Set X-axis range to cover all pixels (0 to 512)
set xrange [0:512]

# Plot the data
plot "cycle_counts.dat" using 1:2 with linespoints ls 1 title "Cycle Count"

################################################
# Third Plot: Very Zoomed-In Cycle Count Range #
################################################

# Set titles and labels for the second plot
set title "Cycle Count Range Up to 400 Cycles"
set xlabel "Pixel Number"
set ylabel "Cycle Count (cycles)"

# Enable grid
set grid

# Define the style for the plot (different color for distinction)
set style line 1 lc rgb 'green' lt 1 lw 1 pt 7 ps 0.5

# Set Y-axis range to limit cycle counts up to 1400 cycles
set yrange [0:500]

# Set X-axis range to cover all pixels (0 to 512)
set xrange [0:512]

# Plot the data
plot "cycle_counts.dat" using 1:2 with linespoints ls 1 title "Cycle Count"
# Exit multiplot mode

unset multiplot
