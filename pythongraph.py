import serial
import matplotlib.pyplot as plt

def read_serial_data(port, baud_rate):
    ser = serial.Serial(port, baud_rate)  # Set a timeout of 1 second
    
    try:
        periods = []

        while True:
            line = ser.readline().decode().strip()

            if "fluffy bunnies" in line:
                break
            
            try:
                period = float(line)
                periods.append(period)

            except ValueError:
                print(f"{line}")

    except KeyboardInterrupt:
        print("Script terminated.")

    finally:
        print("Histogram Generated")
        ser.close()
        generate_and_show_plot(periods)

def generate_and_show_plot(data):
    if data:
        min_value = min(data)
        max_value = max(data)

        plt.xlim(min_value, max_value)
        plt.hist(data, bins=20, edgecolor='black')
        plt.xlabel('Period (ms)')
        plt.ylabel('Frequency')
        plt.title('Period Histogram')
        plt.show()

if __name__ == "__main__":
    serial_port = 'COM8'  # Update with the correct COM port
    baud_rate = 115200

    try:
        periods_array = []
        read_serial_data(serial_port, baud_rate)
    except Exception as e:
        print(f"An error occurred: {e}")
