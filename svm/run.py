import joblib
import pandas as pd

# demonstrates a trained model and use it for new predictions.
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
        print("Trained model loaded successfully.")

        # In a real-world scenario, you would replace this with code that
        # reads live data from your Arduino's sensors.
        # For demonstration purposes, we will use a sample of new sensor readings.
        # The column names must match the ones used during training.
        live_sensor_data = {
            'Soil Moisture': [450],
            'Air temperature (C)': [25],
            'Air humidity (%)': [60]
        }
        
        # pandas DataFrame from the live data
        new_data_df = pd.DataFrame(live_sensor_data)
        
        # prediction using the loaded model
        # The .predict() method will output the predicted water needed.
        predicted_water_needed = model.predict(new_data_df)
        
        print("\nPredicted water needed based on live sensor readings:")
        print(f"{predicted_water_needed[0]:.2f} units of water.")
        
    except FileNotFoundError:
        print("Error: 'irrigation_model.pkl' not found.")
    except Exception as e:
        print(f"An error occurred: {e}")

# Run
if __name__ == "__main__":
    make_prediction_from_live_data()
