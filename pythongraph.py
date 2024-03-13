import serial
import matplotlib.pyplot as plt

def read_serial_data(port, baud_rate):
    ser = serial.Serial(port, baud_rate)  # Set a timeout of 1 second
    
    try:
        while True:
            line = ser.readline().decode().strip()
            if "fluffy bunnies" in line:
                print("Received 'fluffy bunnies'. Starting to take readings.")
                break

    except KeyboardInterrupt:
        print("Script terminated.")
        return

    periods = []

    try:
        while True:
            line = ser.readline().decode().strip()

            if "fluffy bunnies" in line:
                break
            
            try:
                period = float(line)
                periods.append(period)
                print(f"Received period: {period}")

            except ValueError:
                print(f"Invalid value received: {line}")

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
        plt.xlabel('Frequency (Hz)')
        plt.ylabel('Quantity')
        plt.title('Period Histogram')
        plt.show()

if __name__ == "__main__":
    serial_port = 'COM5'  # Update with the correct COM port
    baud_rate = 115200

    try:
        read_serial_data(serial_port, baud_rate)
    except Exception as e:
        print(f"An error occurred: {e}")
