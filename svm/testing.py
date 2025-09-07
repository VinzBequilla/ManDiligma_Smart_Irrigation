import numpy as np
import pandas as pd
from sklearn.model_selection import train_test_split
from sklearn.svm import SVR 
from sklearn.metrics import mean_squared_error, r2_score
import joblib

# Load the data from the CSV file.
try:
    df = pd.read_csv('svm/TARP.csv')
    print("Data loaded successfully from 'svm/TARP.csv'.")
    print("Original columns in the dataset:", df.columns.tolist())
except FileNotFoundError:
    print("Error: The file 'svm/TARP.csv' was not found. Please ensure the path is correct.")
    exit()

# --- DATASET INSPECTION ---
print("\n--- Inspecting the Dataset ---")
# Display the first 5 rows to check column names and data types
print("\nFirst 5 rows of the dataset:")
print(df.head())

# Get a concise summary of the DataFrame
print("\nDataFrame information:")
print(df.info())

# Show descriptive statistics for numerical columns
print("\nDescriptive statistics:")
print(df.describe())

# Check for any missing (null) values in each column
print("\nMissing values per column:")
print(df.isnull().sum())
print("----------------------------")


# --- DATA CLEANING ---
# SVR does not handle missing values, so we will remove any rows with NaN.
initial_rows = len(df)
df.dropna(inplace=True)
cleaned_rows = len(df)
if initial_rows > cleaned_rows:
    print(f"\nRemoved {initial_rows - cleaned_rows} rows with missing values.")
    print(f"Dataset now has {cleaned_rows} rows.")
    print("----------------------------")

# --- IMPORTANT: TARGET VARIABLE MISSING ---
# Your model needs a target variable (e.g., 'water_needed') to learn from.
# The dataset you provided does not have a column for the amount of water used.
# You must add this column to your CSV file for the model to work correctly.
# The 'water_needed' column should contain the measured amount of water that was
# applied for each set of sensor readings.
# For now, we will create a placeholder column with dummy data.
df['water_needed'] = np.random.uniform(1, 10, len(df))

# Separate the features (X) and the target variable (y).
# We are selecting the most relevant features based on your input.
# If your column names are different, please update this line.
features = ['Soil Moisture', 'Air temperature (C)', 'Air humidity (%)']
X = df[features]
y = df['water_needed']

# Split the data into training and testing sets.
# We use 80% of the data for training and 20% for testing.
X_train, X_test, y_train, y_test = train_test_split(X, y, test_size=0.2, random_state=42)

# We are now using a Support Vector Regressor (SVR).
# The 'kernel' parameter defines the type of function used to find the hyperplane.
# The 'C' and 'gamma' parameters are crucial for tuning.
# You can experiment with different values and kernels to find the best fit.
model = SVR(kernel='rbf', C=100, gamma=0.1)

# Train the model using the training data.
print("\nTraining the SVR model...")
model.fit(X_train, y_train)
print("Model training complete.")

# Make predictions on the unseen test data.
y_pred = model.predict(X_test)

# Calculate performance metrics.
# Mean Squared Error (MSE) measures the average squared difference between the
# predicted and actual values. Lower values are better.
mse = mean_squared_error(y_test, y_pred)
# R-squared (R2) score indicates how well the model's predictions fit the actual data.
# A value of 1.0 means a perfect fit.
r2 = r2_score(y_test, y_pred)

print("\n--- Model Evaluation ---")
print(f"Mean Squared Error (MSE): {mse:.4f}")
print(f"R-squared (R2) Score: {r2:.4f}")

# Simulate new sensor readings.
# These would be the live data from your IoT sensors.
new_sensor_readings = pd.DataFrame({
    'Soil Moisture': [350],
    'Air temperature (C)': [28],
    'Air humidity (%)': [55]
})

# Use the trained model to predict the amount of water needed.
predicted_water_needed = model.predict(new_sensor_readings)

print("\n--- Making a New Prediction ---")
print(f"New sensor readings:")
print(f"  Soil Moisture: {new_sensor_readings['Soil Moisture'].values[0]}")
print(f"  Air temperature (C): {new_sensor_readings['Air temperature (C)'].values[0]}")
print(f"  Air humidity (%): {new_sensor_readings['Air humidity (%)'].values[0]}")
print(f"Predicted water needed: {predicted_water_needed[0]:.2f} units.")

# You can optionally save the trained model to a file for later use
# without retraining.
joblib.dump(model, 'irrigation_model.pkl')
print("\nModel saved to 'irrigation_model.pkl'.")
