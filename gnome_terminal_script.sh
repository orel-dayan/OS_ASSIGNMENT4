#!/bin/bash

# Function to run telnet command in a GNOME terminal
function run_telnet() {
    gnome-terminal --title="Telnet Terminal" --command="telnet 127.0.0.1 9034"
}

# Open 100 GNOME terminals
for ((i=1; i<=100; i++)); do
    run_telnet &
done

# Wait for all terminals to finish running
wait