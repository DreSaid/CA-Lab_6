import serial
import matplotlib.pyplot as plt
import numpy as np
import csv
from datetime import datetime  # Import datetime module for timestamping

def read_serial_data(port, baud_rate, save_to_file=True):
    ser = serial.Serial(port, baud_rate)  # Set a timeout of 1 second

    # Create a CSV file for saving data
    if save_to_file:
        timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
        csv_filename = f"phasor_data_{timestamp}.csv"
        csv_file = open(csv_filename, mode='w', newline='')
        csv_writer = csv.writer(csv_file)
        csv_writer.writerow(['Magnitude', 'Phase'])  # Write header with only two columns

    try:
        plt.ion()  # Turn on interactive mode for real-time plotting

        max_magnitude = 0.0  # Variable to track the maximum magnitude

        while True:
            line = ser.readline().decode().strip()

            try:
                # Split the line into magnitude and phase angle
                magnitude, phase_angle = map(float, line.split())

                # Update the phasor diagram in real-time
                max_magnitude = max(max_magnitude, magnitude)
                generate_and_show_phasor_arrow(magnitude, phase_angle, max_magnitude)
                plt.pause(0.1)  # Adjust the pause time as needed

                # Save data to the CSV file
                if save_to_file:
                    csv_writer.writerow([magnitude, phase_angle])
                    csv_file.flush()  # Flush the buffer to ensure data is written immediately

            except ValueError:
                print(f"Invalid data format: {line}")

    except KeyboardInterrupt:
        print("Script terminated.")
        plt.ioff()  # Turn off interactive mode before exiting
        if save_to_file:
            csv_file.close()

    finally:
        ser.close()

def generate_and_show_phasor_arrow(magnitude, phase_angle, max_magnitude):
    # Convert polar coordinates to Cartesian coordinates
    real_part = magnitude * np.cos(np.radians(phase_angle))
    imag_part = magnitude * np.sin(np.radians(phase_angle))

    # Set the axes limits based on the current and maximum values
    xlim = max(max_magnitude, abs(real_part)) + 1
    ylim = max(max_magnitude, abs(imag_part)) + 1

    # Plot the phasor arrow
    plt.clf()  # Clear the previous plot
    plt.quiver(0, 0, real_part, imag_part, angles='xy', scale_units='xy', scale=1, color='blue', label='Test Phasor')
    plt.text(real_part, imag_part, f'Magnitude: {magnitude:.2f} V\nPhase: {phase_angle:.2f}°', fontsize=8, ha='right', va='top')
    plt.axhline(0, color='black', linewidth=0.5)
    plt.axvline(0, color='black', linewidth=0.5)
    plt.grid(color='gray', linestyle='--', linewidth=0.5)
    plt.xlabel('Real Part')
    plt.ylabel('Imaginary Part')
    plt.title('Phasor Diagram')
    plt.legend()
    plt.xlim(-xlim, xlim)
    plt.ylim(-ylim, ylim)
    plt.draw()

if __name__ == "__main__":
    serial_port = 'COM8'  # Update with the correct COM port
    baud_rate = 115200

    try:
        read_serial_data(serial_port, baud_rate)
    except Exception as e:
        print(f"An error occurred: {e}")
