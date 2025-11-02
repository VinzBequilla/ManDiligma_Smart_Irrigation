import pandas as pd
import numpy as np
from sklearn.model_selection import train_test_split
from sklearn.neighbors import KNeighborsClassifier
from sklearn.metrics import accuracy_score
from sklearn.preprocessing import StandardScaler

# ==============================
# 1. Load dataset
# ==============================
df = pd.read_csv("svm\TARP.csv")

# Drop rows with NaN
df = df.dropna()

# Select features + label
# Adjust column names if needed
features = ["Soil Moisture", "Temperature", "Soil Humidity", "Air temperature (C)", "Air humidity (%)"]
X = df[features].values
y = df["Status"].map({"OFF":0, "ON":1}).values  # Convert ON/OFF → 1/0

# ==============================
# 2. Normalize features
# ==============================
scaler = StandardScaler()
X_scaled = scaler.fit_transform(X)

# ==============================
# 3. Split + Train KNN
# ==============================
X_train, X_test, y_train, y_test = train_test_split(X_scaled, y, test_size=0.2, random_state=42)

knn = KNeighborsClassifier(n_neighbors=3)
knn.fit(X_train, y_train)
y_pred = knn.predict(X_test)

print("KNN Accuracy:", accuracy_score(y_test, y_pred))

# ==============================
# 4. Export sample predictions
# ==============================
sample = [[600, 34, 40, 30, 60]]  # example soil moisture/temp/etc
sample_scaled = scaler.transform(sample)
print("Prediction for sample:", knn.predict(sample_scaled))

# ==============================
# 5. Export reduced dataset for Arduino
# ==============================
# Take small balanced subset (max 10 ON + 10 OFF)
df_on = df[df["Status"]=="ON"].sample(min(10, len(df[df["Status"]=="ON"])), random_state=42)
df_off = df[df["Status"]=="OFF"].sample(min(10, len(df[df["Status"]=="OFF"])), random_state=42)

df_small = pd.concat([df_on, df_off])

print("\n=== Arduino Training Set ===")
for _, row in df_small.iterrows():
    print(f'{{{{{int(row["Soil Moisture"])}, {int(row["Temperature"])}, {int(row["Soil Humidity"])}, '
          f'{int(row["Air temperature (C)"])}, {int(row["Air humidity (%)"])} }}, {1 if row["Status"]=="ON" else 0}}},')
