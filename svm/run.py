import joblib
import pandas as pd
import time
import serial
import sys
import signal

# Configuration
MODEL_FILE = 'irrigation_model.pkl'  # Path to your trained SVR model
SERIAL_PORT = 'COM3'  # Update to your Arduino's port (e.g., '/dev/ttyUSB0' on Linux/Mac)
BAUD_RATE = 9600
FLOW_RATE_ML_PER_S = 6.67  # Calibrated flow rate (e.g., 100 ml in 15 seconds → 6.67 ml/s)
MIN_WATER_ML = 10  # Minimum water to trigger pump (avoid tiny runs)
MAX_WATER_ML = 500  # Maximum water per cycle (safety cap)
LOOP_DELAY_S = 5  # Time between sensor reads/predictions (seconds)

ser = None

def signal_handler(sig, frame):
    """Graceful exit on Ctrl+C."""
    print("\nExiting gracefully...")
    if ser:
        ser.close()
    sys.exit(0)

signal.signal(signal.SIGINT, signal_handler)

def make_prediction_and_pump():
    """
    Loads the SVR model, reads live sensor data from Arduino via serial,
    predicts water needed (ml), calculates pump runtime (seconds),
    and sends a command to Arduino to run the pump for that exact time.
    """
    try:
        # Load the trained SVR model
        model = joblib.load(MODEL_FILE)
        print("SVR model loaded successfully.")

        # Establish serial connection to Arduino
        global ser
        ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
        time.sleep(2)  # Wait for connection to stabilize
        print(f"Connected to Arduino on {SERIAL_PORT}.")

        while True:
            # Read a line from Arduino (expected format: 'moisture,temp,humidity\n')
            line = ser.readline().decode('utf-8').strip()

            if line:
                try:
                    sensor_values = line.split(',')

                    if len(sensor_values) == 3:
                        moisture = float(sensor_values[0])
                        temperature = float(sensor_values[1])
                        humidity = float(sensor_values[2])

                        print(f"\nSensors: Moisture={moisture}%, Temp={temperature}°C, Humidity={humidity}%")

                        # Prepare DataFrame for model prediction
                        live_sensor_data = {
                            'Soil Moisture': [moisture],
                            'Air temperature (C)': [temperature],
                            'Air humidity (%)': [humidity]
                        }
                        new_data_df = pd.DataFrame(live_sensor_data)

                        # Predict water needed (in ml)
                        predicted_ml = model.predict(new_data_df)[0]
                        print(f"Predicted water needed: {predicted_ml:.2f} ml")

                        # Calculate and send pump command if needed
                        if predicted_ml >= MIN_WATER_ML:
                            predicted_ml = min(predicted_ml, MAX_WATER_ML)  # Cap for safety
                            runtime_s = predicted_ml / FLOW_RATE_ML_PER_S
                            command = f"PUMP:{int(runtime_s)}\n"  # e.g., "PUMP:15"
                            ser.write(command.encode('utf-8'))
                            print(f"Sent command: {command.strip()} (for {predicted_ml:.2f} ml in {runtime_s:.2f} seconds)")
                        else:
                            print("Predicted water below minimum threshold. Skipping pump.")

                    else:
                        print("Invalid data format from Arduino. Skipping...")

                except (ValueError, IndexError) as e:
                    print(f"Error processing data: {e}. Skipping...")

            # Delay before next read
            time.sleep(LOOP_DELAY_S)

    except FileNotFoundError:
        print(f"Error: '{MODEL_FILE}' not found. Ensure the model file exists.")
    except serial.SerialException as e:
        print(f"Serial error: {e}. Check port and Arduino connection.")
    except Exception as e:
        print(f"Unexpected error: {e}")

if __name__ == "__main__":
    make_prediction_and_pump()