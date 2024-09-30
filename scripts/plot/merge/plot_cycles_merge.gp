# Check if the 'output_file' variable is set; if not, default to 'cycle_counts_combined.png'
if (!exists("output_file")) {
    output_file = "plot.png"
}

# Set the terminal to PNG with a specific size and enhanced text
set terminal pngcairo size 1000,1800 enhanced font 'Verdana,12'

# Specify the output file
set output output_file

# Enable multiplot layout: 3 rows, 1 column
set multiplot layout 3,1 rowsfirst

# Define line styles for the plots (different colors for each file)
set style line 1 lc rgb 'blue' lt 1 lw 1 pt 7 ps 0.5
set style line 2 lc rgb 'red' lt 1 lw 1 pt 7 ps 0.5
set style line 3 lc rgb 'green' lt 1 lw 1 pt 7 ps 0.5
set style line 4 lc rgb 'orange' lt 1 lw 1 pt 7 ps 0.5
set style line 5 lc rgb 'purple' lt 1 lw 1 pt 7 ps 0.5

###########################################
# First Plot: Full Cycle Count Range
###########################################

set title "Full Cycle Count Range         ".strftime("%a %b %d %H:%M:%S %Y", time(0))
set xlabel "Pixel Number"
set ylabel "Cycle Count (cycles)"
set grid
set xrange [0:512]

plot "cycle_counts1.dat" using 1:2 with linespoints ls 1 title "ken's", \
     "cycle_counts2_processed.dat" using 1:2 with linespoints ls 2 title "8xperlin", \
     "cycle_counts3.dat" using 1:2 with linespoints ls 3 title "4xlerp", \
     "cycle_counts4.dat" using 1:2 with linespoints ls 4 title "4xgrad", \
     "cycle_counts5.dat" using 1:2 with linespoints ls 5 title "4xSIMDperlin"

###########################################
# Second Plot: Zoomed-In Cycle Count Range
###########################################

set title "Cycle Count Range Up to 1400 Cycles"
set ylabel "Cycle Count (cycles)"
set grid
set yrange [500:1400]
set xrange [0:512]

plot "cycle_counts1.dat" using 1:2 with linespoints ls 1 title "ken's", \
     "cycle_counts2_processed.dat" using 1:2 with linespoints ls 2 title "8xperlin", \
     "cycle_counts3.dat" using 1:2 with linespoints ls 3 title "4xlerp", \
     "cycle_counts4.dat" using 1:2 with linespoints ls 4 title "4xgrad", \
     "cycle_counts5.dat" using 1:2 with linespoints ls 5 title "4xSIMDperlin"

################################################
# Third Plot: Very Zoomed-In Cycle Count Range #
################################################

set title "Cycle Count Range Up to 400 Cycles"
set ylabel "Cycle Count (cycles)"
set grid
set yrange [0:500]
set xrange [0:512]

plot "cycle_counts1.dat" using 1:2 with linespoints ls 1 title "ken's", \
     "cycle_counts2_processed.dat" using 1:2 with linespoints ls 2 title "8xperlin", \
     "cycle_counts3.dat" using 1:2 with linespoints ls 3 title "4xlerp", \
     "cycle_counts4.dat" using 1:2 with linespoints ls 4 title "4xgrad", \
     "cycle_counts5.dat" using 1:2 with linespoints ls 5 title "4xSIMDperlin"

# Exit multiplot mode
unset multiplot
