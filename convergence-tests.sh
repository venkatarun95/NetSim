# Find the port numbers (and the ip addresses)
awk '{print $3}' convergence-test-tcpdump-ascii | sort -u > /tmp/unique_ports

# Separate the sent packets into different files, segregated by sender port
gnuplot_script="plot "
i=1
echo "" > /tmp/r_script
while read p; do
        # Prints the sending rate as indicated by sender
        grep $p\ \> convergence-test-tcpdump-ascii | \
                awk '{print $1, ":", $2}' | \
                awk -F ':' 'BEGIN{prev=0}{print $2*60+$3, $4; prev=$2*60+$3}' > /tmp/Sender_$p
        # Prints sending rate as inferred from timestamps
        # grep $p\ \> convergence-test-tcpdump-ascii | \
                # awk '{print $1}' | \
                # awk -F ':' 'BEGIN{prev=0}{if($3==prev)print $2*60+$3, 0; else print $2*60+$3, 1/($3-prev); prev=$2*60+$3}' > /tmp/Sender_$p
        echo "Sender_$p"
        gnuplot_script="$gnuplot_script \"/tmp/Sender_$p\" using 1:2, "

        #i=`echo $p | awk -F '.'{print $5}`
        echo "v_$i <- read.csv(file='/tmp/Sender_$p', head=FALSE, sep=' ')" >> /tmp/r_script
        echo "h_$i <- hist(v_$i\$V1, breaks=(max(v_$i\$V1) - min(v_$i\$V1))*0.001)" >> /tmp/r_script
        i=$((i+1))
done </tmp/unique_ports

# echo "plot(h_1, col=rgb(1,0,0,1/4), xlim=c(min(min(v_3\$V1), min(v_2\$V1), min(v_1\$V1)), max(max(v_3\$V1), max(v_2\$V1), max(v_1\$V1))))" >> /tmp/r_script
echo "plot(h_1, col=rgb(1,0,0,1/4), xlim=c(min(min(v_4\$V1), min(v_3\$V1), min(v_2\$V1), min(v_1\$V1)), max(max(v_4\$V1), max(v_3\$V1), max(v_2\$V1), max(v_1\$V1))), ylim=c(0, max(max(h_1\$counts), max(h_2\$counts), max(h_3\$counts), max(h_4\$counts))))" >> /tmp/r_script
echo "plot(h_2, col=rgb(0,1,0,1/4), add=T)" >> /tmp/r_script
echo "plot(h_3, col=rgb(0,0,1,1/4), add=T)" >> /tmp/r_script
echo "plot(h_4, col=rgb(1,1,0,1/4), add=T)" >> /tmp/r_script

echo $gnuplot_script | head -c -2 > /tmp/gnuplot_script
echo $gnuplot_script
cat /tmp/r_script
# gnuplot /tmp/gnuplot_script
