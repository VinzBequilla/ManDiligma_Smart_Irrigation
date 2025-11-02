import pandas as pd
import numpy as np
from sklearn.experimental import enable_iterative_imputer
from sklearn.impute import IterativeImputer

# Load dataset
df = pd.read_csv('svm\TARP.csv')  # Update with your CSV path, e.g., 'C:/path/to/TARP.csv'

# Select 4 features
features = ['Soil Moisture', 'Time', 'Air temperature (C)', 'Air humidity (%)']

# Impute missing values
imputer = IterativeImputer(random_state=46)
df[features] = imputer.fit_transform(df[features])

# Sample 200 rows
subset = df[features + ['Status']].sample(n=200, random_state=42)

# Min/max from full dataset
mins = {'Soil Moisture': 1.0, 'Time': 0.0, 'Air temperature (C)': 11.22, 'Air humidity (%)': 0.59}
maxs = {'Soil Moisture': 90.0, 'Time': 110.0, 'Air temperature (C)': 45.56, 'Air humidity (%)': 96.0}

# Normalize to [0, 1]
normalized = subset[features].copy()
for col in features:
    normalized[col] = (normalized[col] - mins[col]) / (maxs[col] - mins[col])

# Encode Status
labels = subset['Status'].map({'ON': 1, 'OFF': 0}).values

# Output for Arduino
print("const int NUM_SAMPLES = 200;")
print("float dataset[NUM_SAMPLES][4] = {")
for row in normalized.values:
    print("  {" + ", ".join([f"{x:.4f}" for x in row]) + "},")
print("};")
print("int labels[NUM_SAMPLES] = {" + ", ".join([str(x) for x in labels]) + "};")
print("float mins[4] = {" + ", ".join([f"{mins[col]:.2f}" for col in features]) + "};")
print("float maxs[4] = {" + ", ".join([f"{maxs[col]:.2f}" for col in features]) + "};")