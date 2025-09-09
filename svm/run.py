import joblib
import pandas as pd
import time
import serial

# This script demonstrates how to load a trained model and use it for new predictions.
# You will run this script on a device PC that is
# connected to your Arduino to get live sensor readings.

def make_prediction_from_live_data():
    """
    Loads the trained machine learning model and makes a water prediction
    based on a new set of sensor readings.
    """
    try:
        # Load the trained model from the .pkl file
        # The file 'irrigation_model.pkl' should be in the same directory as this script.
        model = joblib.load('irrigation_model.pkl')
        print("SVR model loaded successfully.")

        # establish serial connection to Arduino
        ser = serial.Serial('COM3', 9600, timeout=1)
        time.sleep(2)  # connection time to establish

        print("Connected to Arduino. Reading live sensor data...")

        while True:
            # read a line from Arduino's serial output
            line = ser.readline().decode('utf-8').strip()

            if line:
                try:
                    sensor_values = line.split(',')

                    if len(sensor_values) == 3:
                        moisture = float(sensor_values[0])
                        temperature = float(sensor_values[1])
                        humidity = float(sensor_values[2])
                        
                        #use these real values instead of the simulated ones
                        live_sensor_data = {
                            'Soil Moisture': [moisture],
                            'Air temperature (C)': [temperature],
                            'Air humidity (%)': [humidity]
                        }
                        
                        # create a pandas DataFrame from the live data
                        new_data_df = pd.DataFrame(live_sensor_data)
                        
                        # make a prediction using the loaded model
                        predicted_water_needed = model.predict(new_data_df)
                        
                        print("\nPredicted water needed based on live sensor readings:")
                        print(f"{predicted_water_needed[0]:.2f} units of water.")
                    
                    else:
                        print("Invalid data format received from Arduino. Skipping...")
                
                except (ValueError, IndexError) as e:
                    print(f"Error processing data from Arduino: {e}. Skipping...")
            
            # wait for 5 seconds before checking for new data,
            # but allow for faster updates if Arduino sends data more frequently.
            time.sleep(5)
            
    except FileNotFoundError:
        print("Error: 'irrigation_model.pkl' not found.")
        print("Please make sure you have run the 'smart_irrigation_model.py' script first to train and save the model.")
    except serial.SerialException as e:
        print(f"Error: Could not connect to Arduino on port. Please check the port name and if the Arduino is connected. {e}")
    except Exception as e:
        print(f"An unexpected error occurred: {e}")

# Run
if __name__ == "__main__":
    make_prediction_from_live_data()
