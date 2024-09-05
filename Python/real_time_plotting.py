import serial
import matplotlib.pyplot as plt

def plot_data():
    plt.figure()
    plt.ion()

    x_coordinates = []
    y_coordinates = []

    while True:
        if ser.in_waiting > 0:
            data = ser.readline().decode('utf-8').strip()
            print(data)

            try:
                data_int_list = list(map(int, data.split(',')))

                if len(data_int_list) != 64:
                    data_int_list = [0]*64 

                x_coordinates.extend(data_int_list[0:32])
                y_coordinates.extend(data_int_list[32:64])

                plt.clf()
                plt.plot(x_coordinates, y_coordinates, marker='o', linestyle='', color='b')

                plt.xlim(min(x_coordinates) - 10, max(x_coordinates) + 10)
                plt.ylim(min(y_coordinates) - 10, max(y_coordinates) + 10)

                plt.gca().set_aspect('equal', adjustable='datalim')

                plt.title('Real-Time Plot of x-y Coordinates')
                plt.xlabel('X')
                plt.ylabel('Y')
                plt.draw()
                plt.pause(0.1)

            except ValueError:
                continue

try:
    ser = serial.Serial('COM8', 115200)
    if not ser.isOpen():
        ser.open()

    plot_data()

except KeyboardInterrupt:
    if ser.isOpen():
        ser.close()
    print("Serial port closed.")
