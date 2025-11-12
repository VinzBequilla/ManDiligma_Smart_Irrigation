# ==============================================================
# 1. Install required libraries (run once in your env)
# ==============================================================
# pip install pandas scikit-learn matplotlib seaborn

# ==============================================================
# 2. Imports
# ==============================================================
import pandas as pd
import numpy as np
from sklearn.model_selection import train_test_split, GridSearchCV
from sklearn.preprocessing import MinMaxScaler, LabelEncoder  # CHANGED: MinMaxScaler for Arduino compatibility
from sklearn.impute import SimpleImputer  # For handling NaNs
from sklearn.neighbors import KNeighborsClassifier
from sklearn.metrics import accuracy_score, classification_report, confusion_matrix
import matplotlib.pyplot as plt
import seaborn as sns

# ==============================================================
# 3. Create / load the dataset
# ==============================================================
# Use forward slashes or raw string (r"path\to\file.csv") for paths
df = pd.read_csv("Arduino_Uno/svm/TARP.csv")

print("First 5 rows:")
print(df.head())

# NEW: Check for NaNs (missing values) in the dataset
print("\nNaNs per column:")
print(df.isna().sum())

# ==============================================================
# 4. Basic cleaning
# ==============================================================
# Drop irrelevant columns for Arduino (only use Soil Moisture, Air temperature (C), Air humidity (%))
df = df.drop(columns=["Time", "Temperature", "Soil Humidity"], errors="ignore")

# Encode the target ('ON' → 1, 'OFF' → 0) — matches 1 = irrigate (pump ON)
le = LabelEncoder()
df["Status"] = le.fit_transform(df["Status"])

print("\nLabel mapping:", dict(zip(le.classes_, le.transform(le.classes_))))

# ==============================================================
# 5. Train / test split
# ==============================================================
X = df.drop(columns=["Status"])
y = df["Status"]

X_train, X_test, y_train, y_test = train_test_split(
    X, y, test_size=0.25, random_state=42, stratify=y
)

# ==============================================================
# 6. Impute NaNs (Handle missing values)
# ==============================================================
imputer = SimpleImputer(strategy='mean')  # Or 'median' if preferred
X_train_imputed = imputer.fit_transform(X_train)
X_test_imputed = imputer.transform(X_test)

# ==============================================================
# 7. Scale features (using MinMaxScaler for 0-1 range, matching Arduino)
# ==============================================================
scaler = MinMaxScaler()
X_train_scaled = scaler.fit_transform(X_train_imputed)
X_test_scaled = scaler.transform(X_test_imputed)

# ==============================================================
# 8. Hyper-parameter tuning (k = 1..15)
# ==============================================================
param_grid = {"n_neighbors": range(1, 16)}
knn = KNeighborsClassifier()
grid = GridSearchCV(knn, param_grid, cv=5, scoring="accuracy", n_jobs=-1)
grid.fit(X_train_scaled, y_train)

best_k = grid.best_params_["n_neighbors"]
print(f"\nBest k (from 5-fold CV): {best_k} (score = {grid.best_score_:.4f})")

# ==============================================================
# 9. Train final model with best k
# ==============================================================
knn_best = KNeighborsClassifier(n_neighbors=best_k)
knn_best.fit(X_train_scaled, y_train)

# ==============================================================
# 10. Evaluation
# ==============================================================
y_pred = knn_best.predict(X_test_scaled)

acc = accuracy_score(y_test, y_pred)
print(f"\nTest accuracy: {acc:.4f}")
print("\nClassification report:")
print(classification_report(y_test, y_pred, target_names=le.classes_))

# Confusion matrix plot
cm = confusion_matrix(y_test, y_pred)
plt.figure(figsize=(5,4))
sns.heatmap(cm, annot=True, fmt="d", cmap="Blues",
            xticklabels=le.classes_, yticklabels=le.classes_)
plt.title("Confusion Matrix")
plt.ylabel("True")
plt.xlabel("Predicted")
plt.show()

# ==============================================================
# 11. Export for Arduino (NEW: Generate C arrays from subsampled data)
# ==============================================================
# Subsample to fit Arduino Uno memory (~100-200 samples max; each sample ~16 bytes)
MAX_SAMPLES = 150  # Adjust if needed (test on your Arduino for memory errors)
if len(X) > MAX_SAMPLES:
    subsample_idx = np.random.choice(len(X), MAX_SAMPLES, replace=False)
    X_export = X.iloc[subsample_idx]
    y_export = y.iloc[subsample_idx]
else:
    X_export = X
    y_export = y

# Impute and scale the export data
X_export_imputed = imputer.transform(X_export)
X_export_scaled = scaler.transform(X_export_imputed)

# Print C arrays
print("\n// ===================== Dataset for Arduino =====================")
print(f"const int NUM_SAMPLES = {len(X_export)};")
print(f"const int NUM_FEATURES = {X_export_scaled.shape[1]};")
print("float dataset[NUM_SAMPLES][NUM_FEATURES] = {")
for row in X_export_scaled:
    print("  {" + ", ".join(f"{val:.4f}" for val in row) + "},")
print("};")

print("\nint labels[NUM_SAMPLES] = {")
print("  " + ", ".join(str(int(val)) for val in y_export) + "")
print("};")

# Mins and maxs for normalization (computed from full data)
mins = X.min().values
maxs = X.max().values
print("\nconst float mins[NUM_FEATURES] = {" + ", ".join(f"{val:.2f}" for val in mins) + "};")
print("const float maxs[NUM_FEATURES] = {" + ", ".join(f"{val:.2f}" for val in maxs) + "};")

# Best K
print(f"const int K = {best_k};")

# ==============================================================
# 12. Predict on a NEW sensor reading (for reference)
# ==============================================================
def predict_irrigation(soil_moist, air_temp, air_hum):
    """
    Returns 'ON' or 'OFF' for a single sensor reading.
    """
    new = np.array([[soil_moist, air_temp, air_hum]])
    new_imputed = imputer.transform(new)  # Handle potential NaNs
    new_scaled = scaler.transform(new_imputed)
    pred = knn_best.predict(new_scaled)[0]
    return le.inverse_transform([pred])[0]

# Example
print("\n--- Example prediction ---")
print(predict_irrigation(
    soil_moist=30.5,
    air_temp=32.1,
    air_hum=55.0
))